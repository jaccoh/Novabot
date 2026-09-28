# Herankeren na de wijzigingen aan dockmeting en kaartkopie

Gecontroleerd op `master` bij `5265f7b0`, 28 september 2026. Beads-audit: `Novabot-55f.19.1`; vervolgwerk onder `Novabot-55f.19`.

## Conclusie

Het rekenkundige principe klopt voor een **fysiek onverplaatst eigen dock**. De huidige procedure is nog niet volledig betrouwbaar: zij eist een correcte heading voordat zij gelegenheid geeft die te initialiseren, kan na een timeout een verplichte uitrijcontrole overslaan en kan de blokkade van een mislukte kaartinstallatie opheffen. Daarnaast sluiten wizardstatus en herstelknoppen niet overal aan op de server.

De recente correctie voor automatisch meten bij het brondock lost de headingvolgorde voor **kaartkopie** op. De aparte herankerroute gebruikt die correctie nog niet. Deze audit heeft uitsluitend broncode en lokale tests onderzocht. Er zijn geen maaiers aangestuurd, oorsprongen geschreven of releases uitgevoerd.

## Wat klopt en behouden moet blijven

Een GNSS-meting hoort bij de antenne. Voor herankeren moet eerst het voertuigreferentiepunt worden berekend, met de volledige geroteerde antenneafstand:

```text
voertuig_UTM = antenne_UTM - R(voertuighouding) × antenneafstand
nieuwe_oorsprong = voertuig_UTM - vast_lokaal_dockanker
```

De code schrijft dit equivalent als `oude_oorsprong + voertuigpose - runtime_offset - dockanker`. De sampler definieert `runtime_offset = odom_GPS - (GPS_UTM - oude_oorsprong)`; substitutie levert bovenstaande formule op. De rotatie gebruikt ook roll en pitch. Dit staat in `research/extended_commands.py:2912,2976-2978,5380-5392` en `server/src/services/reanchorGps.ts:49-55`.

De procedure heeft al belangrijke waarborgen:

- Minstens twintig verschillende GNSS/odom-paren met exact gelijke bronstempels in een aaneengesloten venster van 5 tot 15 seconden, met actuele RTK Fixed, gezonde lokalisatie en stabiele voertuigpose.
- De echte `base_link -> gps_link`-transformatie, expliciet laadcontact en vergelijking met de lokale voertuigpose.
- Een tijdelijke lokalisatiecompensatie groter dan 2 cm wordt geweigerd. Die mag niet stilzwijgend in de permanente oorsprong terechtkomen.
- Eén exclusieve kaartoperatie, een bestandsvingerafdruk, een expliciet oorsprongvoorstel en een tweede meting op de maaier voordat deze schrijft. Het voorstel moet binnen 2 cm overeenkomen.
- Backup voor schrijven, atomische vervanging van `pos.json`, bevestiging van `/load_utm_origin_info` en teruglezen. CSV, x3-CSV, rasters en dock-YAML moeten ongewijzigd blijven.
- Na een normale afgeronde schrijfcyclus moet de eindmeting binnen 5 cm van het vaste dockanker liggen. Een los dockbericht of los `verify`-verzoek geeft het frame niet vrij.

Deze controles bewijzen softwareconsistentie en meetkwaliteit. Zij bewijzen op zichzelf geen centimeternauwkeurigheid over de hele tuin.

## Bevindingen

### P1: de strenge headingcontrole staat nog vóór de voorbereidende beweging

**Issue:** `Novabot-55f.19.2`.

`runAutoReanchor` roept op `server/src/routes/dashboard.ts:3299` meteen `measureReanchorDock` aan. Die verlangt op `server/src/services/reanchorGps.ts:34-38` een heading binnen 0,05 radiaal, ongeveer 2,9 graden, van de opgeslagen dockrichting. De fase `needs_drive` komt pas na schrijven, op regel 3328. De native schrijffunctie controleert dezelfde heading.

Een lokale reproductie met een bootafwijking van ongeveer zes graden eindigt daarom in `error`, zonder oorsprongwrite en zonder voorbereidende rijstap. De weigering voorkomt een onbetrouwbare write, maar maakt het herstel onbruikbaar voor precies dit opstartprobleem.

**Correctie:** laat eerst onder toezicht een begrensde beweging relatief aan de maaier en een gecontroleerde terugkeer plaatsvinden. Pas daarna zijn een precieze dockrichting en antennecorrectie zinvolle eisen. De strenge controle bij de uiteindelijke meting en write moet blijven bestaan.

Hergebruik de bestaande bewegingsbegrenzing en operatorwatchdog. Neem niet de hele `sourceDockCycle` over: die weigert terecht een ongevalideerd frame (`sourceDockCycle.ts:34`), terwijl herankeren juist zo'n frame moet herstellen. Ook `dock_measurement_move` staat in `FRAME_BLOCKED_KEYS` (`frameValidation.ts:135`). Hergebruik vereist een specifiek begrensde hersteloperatie, geen algemene omzeiling van die blokkade.

### P1: een goede dockmeting kan een mislukte kaartinstallatie vrijgeven

**Issue:** `Novabot-55f.19.3`.

`installVerifiedMapZip` zet dezelfde `frame_unvalidated`-vlag bij kaartinstallatie en laat die staan bij een mislukte sync, rastergeneratie of plannercontrole (`server/src/services/mowerMapApply.ts:97-113`). Herankeren wist die vlag op `dashboard.ts:3303` of `3351`, zonder de mislukte installatie te herstellen. De MQTT-commandoguard controleert frame en actieve operatie, niet de afzonderlijke foutstatus van kaarttoepassing (`server/src/mqtt/mapCommandGuard.ts:13-16`).

Lokaal gereproduceerd: na `regenerate_failed` levert een passende dockmeting `done` op. `map_apply_phase=failed` en `map_apply_error=regenerate_failed` blijven staan, maar `isFrameNavBlocked(..., {start_navigation:{}})` wordt `false`. Dit scenario schrijft geen nieuwe oorsprong.

**Correctie:** bewaar de redenen voor blokkering afzonderlijk en duurzaam. Herankeren mag uitsluitend de framecontrole afhandelen. Een onvolledige kaartinstallatie moet eerst aantoonbaar worden hersteld of gereconcilieerd. Een correcte positie van het dock bewijst niet dat polygonen, rasters en planner dezelfde kaart gebruiken.

### P1: timeout plus opnieuw starten kan de uitrijcontrole overslaan

**Issue:** `Novabot-55f.19.4`.

De server bewaart de vereiste vervolgcontrole alleen in `reanchorCycles`. `finally` verwijdert deze toestand na een fout of timeout (`dashboard.ts:3362-3363`). Een nieuwe poging mag vervolgens de snelle route nemen zodra de gedockte pose bij het anker past (`3302-3305`).

Lokaal gereproduceerd: oorsprong geschreven, fase `needs_drive`, vijf minuten wachten zonder ooit het dock te verlaten, timeout, opnieuw starten. De tweede poging meldt `done` en wist de blokkade. Geen enkel testbericht heeft een uitgedockte positie gemeld.

**Correctie:** sla vóór de write duurzaam op dat verificatie na een mogelijke oorsprongwijziging vereist is. Die verplichting moet ook bij een verloren antwoord, timeout of serverherstart blijven bestaan. Hervatten moet de ontbrekende uitrij- en terugkeercontrole uitvoeren, zonder onnodig nogmaals de oorsprong te schrijven. Een reeds passende stilstaande dockpose is geen bewijs van die cyclus.

### P2: wizardstatus en herstelknoppen horen niet altijd bij de actuele cyclus

**Issue:** `Novabot-55f.19.5`.

- Na een fout verwijderen `dashboard.ts:3362-3363` de cyclus en `relocked`. De aangeboden handmatige knop **Verifieer** kan daardoor niet meer werken: de knop blijft uitgeschakeld of de route antwoordt 409 (`3380-3383`). Dit is lokaal gereproduceerd na een mislukte eindmeting. Beide wizards bieden deze herstelknop aan (`dashboard/src/components/dashboard/ReanchorWizard.tsx:176-187`, `app/src/components/ReanchorWizard.tsx:244-256`).
- Een nieuwe invalidatie wist de eerdere `done/ok`-status niet (`dashboard.ts:3389`). De statusroute blijft oud succes teruggeven, terwijl het frame opnieuw geblokkeerd is. Lokaal gereproduceerd. De dashboardwizard sluit bij dit antwoord meteen (`ReanchorWizard.tsx:56-64`).
- De app zet bij heropenen `running=false` en wist haar status. Zij pollt uitsluitend wanneer `running=true` (`app/src/components/ReanchorWizard.tsx:72-106`). Een nog actieve servercyclus kan daardoor niet normaal worden hervat na sluiten van de wizard. Dit punt is een codebevinding; er is geen fysieke app-test uitgevoerd.

**Correctie:** koppel voortgang en succes aan de actuele frameversie en herstelcyclus. Haal bij openen altijd de serverstatus op. Toon alleen herstelacties die bij de bewaarde cyclus passen.

## Aanbevolen procedure

1. Stel vast dat dit het eigen, fysiek onverplaatste dock is. Controleer het opgeslagen anker en behandel eventuele onvoltooide kaartinstallatie apart.
2. Initialiseer zo nodig de lokalisatie door onder toezicht begrensd achteruit te rijden en gecontroleerd terug te keren. Vereis bij vertrek nog geen precieze vergelijking met de opgeslagen heading. Gebruik voor terugkeer geen routeplanning door het nog ongevalideerde kaartframe.
3. Meet na terugkeer de verse stabiele voertuigpose met volledige antennecorrectie. Beoordeel nu pas of de oorsprong moet wijzigen. Een passende pose kan een write overbodig maken, maar mag geen eerdere openstaande verificatieverplichting overslaan.
4. Als schrijven nodig is: leg de openstaande verificatie duurzaam vast en gebruik de bestaande backup, voorstelcontrole, native her-meting, bevestigde reload en teruglezing.
5. Bewijs na een write met verse metingen dat de maaier uitgedockt opnieuw lokaliseert en bij terugkeer binnen 5 cm van het vaste dockanker komt. Bewaar de resterende fase bij onderbreking.
6. Hef uitsluitend de opgeloste frameblokkade op. Andere openstaande kaartproblemen blijven navigatie blokkeren.

Deze volgorde gebruikt de bestaande meet- en bewegingscode. Er is geen nieuw kalibratiemodel of dronefoto nodig. De voorbereidende beweging en de controle na een write hebben verschillende doelen; zij mogen alleen worden samengevoegd wanneer de relevante metingen werkelijk na de oorsprongwijziging vallen.

## Grenzen ten opzichte van kaartkopie en foto

Herankeren herstelt de koppeling tussen GPS en het bestaande lokale kaartframe van **dezelfde maaier**. De lokale zonecoördinaten en het lokale dockanker blijven daarbij vast. De bedoeld blijvende fysieke plek van de zones volgt alleen wanneer het dock niet verplaatst is en het probleem inderdaad een oorsprongverschuiving is. De procedure corrigeert geen rotatie, schaalfout of plaatselijk vervormde lokalisatie.

Bij kaartkopie blijft daarnaast een gemeten overeenkomst tussen beide maaierframes nodig. Met de doelmaaier het brondock bezoeken dient om die overeenkomst te meten. De doelmaaier daar herankeren alsof dat zijn eigen dock is, zou fout zijn. ArUco-docken is een manier om fysiek terug te keren; het is geen zelfstandige GPS-kalibratie of oorsprongwrite.

Een fysiek verplaatst dock vraagt om een nieuwe gemeten koppeling tussen dock en bestaande grondkaart. Gebruik van het oude lokale dockanker bij het nieuwe fysieke dock verplaatst anders de hele kaart in de echte wereld. Een goed herankerde maaier bewijst evenmin dat een satelliet- of dronebeeld op enkele centimeters ligt.

Een daadwerkelijk ontbrekend of onleesbaar `pos.json` wordt momenteel niet door deze procedure hersteld: meting en bestandsvingerafdruk vereisen een geldige bestaande oorsprong. Verkeerd gekoppelde oorsprong en ontbrekend bestand zijn verschillende herstelgevallen.

## Uitgevoerde controles

- 117 bestaande Vitest-tests geslaagd in `dashboardReanchorAuto`, `reanchorGps`, `frameValidation`, `sourceDockCycle`, `copyAlignment` en `mowerMapOperation`.
- 15 Python-tests uit `research/test_marker_measurement.py` en 8 uit `research/test_dock_measurement_motion.py` geslaagd.
- Vijf aanvullende geïsoleerde routeproeven hebben de hierboven beschreven headingweigering, kaartblokkade-vrijgave, timeoutomweg, onbruikbare Verify en oude successtatus gereproduceerd. Deze proeven gebruiken mocks en bevestigen huidig foutgedrag; ze zijn geen goedkeuring van dat gedrag. De tijdelijke test is buiten de reguliere testsuite bewaard onder `research/captures/2026-09-28-reanchor-audit/`.
- Geen live beweging, herankering, kaartinstallatie of fysieke acceptatie uitgevoerd. Productiecode is voor deze audit ongewijzigd gebleven.

De bestaande groene tests missen deze foutcombinaties. Bij de correcties moeten de reproducties veranderen in tests voor het gewenste gedrag. Daarna blijven de volledige cyclus op een eigen dock, hervatten na onderbreking en gecontroleerde fysieke kaartnavigatie afzonderlijke acceptatiestappen.
