# Verplichte dockmeting bij zonekopieën

Status: serverbeta en beide Python-scripts zijn op 27 september geplaatst; de eerste live runtimecontroles antwoorden. Een aanvullende correctie voor verschillende systeemklokken wordt gevolgd onder `Novabot-55f.23.3`. De eerste herhaalmetingen op .100 faalden; het laatste paar voldoet aan de geometrische herhaalgrenzen. Een volledige registratie en kopie tussen beide maaiers zijn nog niet fysiek gevalideerd. Er is geen nauwkeurigheidsgarantie en geen vrijgave voor autonoom maaien op basis van deze proef. Uitvoering wordt gevolgd onder Beads `Novabot-55f`.

## Besluit en afbakening

Iedere nieuwe zonekopie vereist dat beide maaiers hetzelfde fysieke bronlaadstation met hun bestaande camera meten. De doelmaaier rijdt daarvoor vóór het dock van de bronmaaier, zonder te docken. Een dronefoto, satellietklik, RTK-walker of externe landmeetapparatuur is niet nodig. De gebruiker rijdt de doelmaaier zelf. Voor de bronmaaier is er een optionele automatische uitrij-, meet- en dockcyclus onder toezicht.

Dit vervangt voor de huidige implementatie het eerdere voorstel voor een eenmalige, blijvend opgeslagen registratie. De wizard gebruikt een sessie in servergeheugen met een geldigheid van **20 minuten vanaf de start**, gebonden aan bronmaaier, doelmaaier en gekozen zone. Na succesvolle opslag van de kopie wordt de sessie verbruikt. Een serverherstart, verlopen sessie of volgende kopie vraagt nieuwe metingen. Wijzigen van bron of zone wist de wizardregistratie; wijzigen van de optie voor obstakels behoudt de metingen maar vraagt een nieuwe preview.

Een satellietlaag of dronefoto blijft optionele weergave. Geen beeldlaag bepaalt de omrekening van de gekopieerde navigatiegeometrie. De eigen docks en LoRa-koppelingen blijven behouden; de meetprocedure wijzigt `pos.json` niet.

## Wizard en eerste praktijktest

1. **Bron, eerste meting:** zet .100 stil vóór zijn eigen dock, met het patroon in beeld, zonder te docken. Bevestig het fysieke bronstation en neem de meting op.
2. **Bron, herhaalmeting:** verplaats .100 ongeveer 20 cm, houd hetzelfde patroon in beeld, stop en meet opnieuw. De gemeten verplaatsing moet minstens 15 cm zijn.
3. **Doel, eerste meting:** maak ruimte met .100. Rijd .244 vóór hetzelfde dock van .100, zonder te docken. Bevestig opnieuw het fysieke bronstation, stop en meet.
4. **Doel, herhaalmeting:** verplaats .244 ongeveer 20 cm en meet hetzelfde patroon opnieuw terwijl hij stilstaat.
5. **Preview en toepassen:** zet .244 terug op zijn eigen dock met laadcontact. Ook .100 blijft stilstaan met gezonde RTK-ontvangst. Controleer zone, obstakels en voorgesteld dockkanaal; beide maaiers blijven stilstaan tot de kaartoverdracht en eindcontrole zijn bevestigd.

Handmatige opnamen zijn aparte gebruikersacties; de automatische broncyclus neemt na één expliciete start beide bronmetingen op. Het patroon identificeert het station niet uniek: daarom blijft de fysieke bronstationbevestiging bij iedere meting verplicht. Ontbrekende firmwareondersteuning, onvoldoende verse of onbetrouwbare metingen blokkeren de flow. Er is geen fotoklik, sleepbare kopieermarker of losse positiemeting als omweg. Annuleren of opnieuw beginnen blijft mogelijk; late antwoorden mogen een gesloten of gewijzigde wizard niet herstellen.

De eerste proef stopt al na stap 2 als dezelfde maaier hetzelfde stilstaande station niet voldoende reproduceerbaar meet. Pas na een geslaagde bronproef volgt de vergelijking tussen beide maaiers. Lokale tests en een succesvolle build vervangen deze apparaattest niet.

## Berekening en controles in de testcode

Gebruik de effectieve lokale navigatiepose voor de markerberekening; vervang deze niet door GNSS. Accepteer een registratie alleen wanneer die navigatiepose voldoende aansluit op het vaste native GPS-frame en die verhouding tijdens de procedure niet verandert. De camera rapporteert een relatieve pose; de gecontroleerde richting van die transformatie en de volledige 3D-oriëntatie moeten behouden blijven. De lokalisatie-odometrie gebruikt het GPS-referentiepunt. Reken dit via de gemeten `base_link`–`gps_link`-transformatie naar het voertuigreferentiepunt om voordat de markerpose wordt samengesteld.

Voor beide maaiers wordt de positie van hetzelfde markerpatroon in hun eigen kaartframe bepaald. De huidige kaartframes gebruiken dezelfde UTM-gridassen en schaal. Met `marker_A` en `marker_B` als gemiddelden van de twee geaccepteerde metingen:

```text
verschuiving = marker_B - marker_A
punt_B = punt_A + verschuiving
dock_A_in_B = opgeslagen_dock_A + verschuiving
```

Het markerpatroon wordt dus niet gelijkgesteld aan het opgeslagen dockreferentiepunt. Werkgebied en obstakels krijgen dezelfde verschuiving. De opgeslagen dockpose van .244 bepaalt zijn eigen kanaalaansluiting. Het doeldock zelf wordt niet verplaatst.

De huidige controles omvatten verse RTK Fixed/lokalisatie, stabiele stilstand, tijdkoppeling tussen camera en odometrie van maximaal 0,12 seconde, passende meetvensters, en ongewijzigde brongeometrie en frame-identiteiten. De herhaalde markerposities moeten binnen **3 cm in 3D** en **1 graad yaw** overeenkomen. De headings tussen beide maaiers worden eveneens gecontroleerd. De bestaande geometrie-, obstakel-, dock- en bevestigde overdrachtscontroles blijven gelden.

**De headinggrens van 1 graad is uitsluitend een controle op grove tegenspraak.** Zij bewijst geen centimeternauwkeurigheid verderop in de tuin: 1 graad komt op 25 meter overeen met circa 44 cm dwarsafwijking. Er wordt geen hele zone gedraaid om een enkele camerarichting passend te maken. Ook twee goede metingen bij één dock bewijzen de ligging van de volledige zone nog niet.

Een `pos.json`-/dockfingerprint en serverframerevisie herkennen bestands- en bekende framewijzigingen, maar niet iedere interne lokalisatiecompensatie. Die compensatie kan tijdens rijden veranderen zonder dat een bestandshash wijzigt. De 20-minutensessie voorkomt langdurig hergebruik, maar neemt dit risico niet weg. Onafhankelijke controles van de bestaande fysieke grens, verspreid over de tuin en na rijden, blijven nodig voor globale acceptatie.

## Live bronproef op .100: afstandsafhankelijke resultaten

De ruwe opnamen en het controlescript staan lokaal in `research/captures/2026-09-26-dock-marker/` (gitignored). De bestanden `novabot-aruco-source-first.json` en `novabot-aruco-source-second.json` zijn omgerekend met `novabot-analyze-marker.py`. Dit zijn diagnostische opnamen; de nieuwe wizard/extended-command-code is daarvoor niet gedeployed.

| Gemiddelde markerpose in .100-kaartframe | Eerste stand | Tweede stand |
|---|---:|---:|
| x (m) | 0,109850 | 0,096185 |
| y (m) | −0,056264 | −0,015350 |
| z (m) | 0,041508 | 0,094297 |
| yaw | 92,1641° | 89,5622° |
| Unieke beeldtijdstempels | 60 | 60 |
| Unieke uitgegeven `/aruco/pose`-waarden | 4 | 1 |

Hetzelfde vaste patroon verschilt tussen de twee standen **4,31 cm horizontaal, 6,82 cm in 3D en 2,60 graden in yaw**. De maaierverplaatsing was circa 23,4 cm. Dit overschrijdt de herhaalgrenzen; de koppeling is hiermee **niet bruikbaar verklaard**. De grenzen worden niet verruimd om deze proef alsnog te laten slagen.

Zestig unieke beeldtijdstempels zijn hier geen bewijs van zestig onafhankelijke poses: de detector gaf in de eerste opname slechts vier verschillende poses en in de tweede één pose uit. Kleine spreiding binnen zo'n opname bewijst daarom geen evenredig kleine meetfout; kwantisatie van gedetecteerde beeldhoeken kan herhaalde poses opleveren. Een aparte, latere alleen-lezen beeldcontrole (`novabot-camera-hash-shm-100.json`) gaf 30 beelden met 30 verschillende pixelhashes in circa 8 seconden: die stream stond toen niet stil. Omdat deze controle niet gelijktijdig met de poses plaatsvond, bewijst zij niet dat de eerdere 60 poses onafhankelijk waren. De detectoruitvoer en afstandsafhankelijkheid moeten verder worden onderzocht. Een vergelijking .100–.244 en een onafhankelijke grenscontrole zijn nog niet uitgevoerd.

Een derde diagnostische opname (`novabot-aruco-source-close.json`) vanaf circa 35 cm camera-afstand gaf marker `(0,140179; -0,055441; 0,081412)` m en yaw `91,3374°`. Ten opzichte van de eerste stand is dat circa 3,03 cm XY, 5,01 cm in 3D en 0,83° yaw. Ook die vergelijking voldoet nog niet aan de positiegrens.

De vierde opname (`novabot-aruco-source-close-repeat.json`) vanaf circa 47 cm camera-afstand gaf marker `(0,119959; -0,043026; 0,071005)` m en yaw `91,7327°`. Ten opzichte van de derde opname is dit **2,373 cm XY, 2,591 cm in 3D en 0,395° yaw**. De onderlinge afwijking is beter, maar de gemeten voertuigverplaatsing bedraagt slechts **11,245 cm**, onder het minimum van 15 cm voor twee voldoende verschillende standplaatsen. Deze vergelijking is daarom nog geen geslaagde registratie. De 61 beeldstempels leverden één verschillende camerapose, dus de interne spreiding blijft geen zelfstandig nauwkeurigheidsbewijs.

De vijfde opname (`novabot-aruco-source-close-final.json`) vanaf circa 71 cm camera-afstand geeft marker `(0,108154; -0,049137; 0,094232)` m en yaw `91,4792°`. Het laatste paar (vierde naar vijfde) verschilt **1,329 cm XY, 2,676 cm in 3D en 0,2535° yaw**, met **23,673 cm voertuigverplaatsing**. Dit paar voldoet aan de geometrische herhaalgrenzen. De eerdere grotere verschillen blijven bewijs dat afstand en gezichtspunt ertoe doen; één geaccepteerd paar bewijst geen algemene of absolute nauwkeurigheid.

De vijfde opname bevat ook tien chassisberichten met beide LoRa-vlaggen onwaar, 101 BestPos-berichten met qual 4 en correctieouderdom 1,2–2,0 seconden, en LOC_SUCCESS. De lokale productievalidator accepteert het vooraf gekozen middelste venster van 12 seconden. Een onafhankelijke herberekening accepteert ook de aangrenzende vensters (+3 tot +7 seconden start): het geometrische resultaat hangt niet van één gunstig venster af. Dit is nog geen volledige productregistratie: de vierde opname mist gelijktijdige chassisgegevens en de diagnostische opnamen zijn niet via een actieve server-wizardsessie gedaan. De detector is na de vijfde opname aantoonbaar uitgeschakeld. De volgende proef meet .244 voor hetzelfde fysieke dock van .100.

### Algemene LoRa-status en actuele meetgezondheid

De latere opnamen tonen `RobotStatus.error_status=8`, terwijl de actuele BestPos Fixed en LOC_SUCCESS zijn. De eerste implementatie blokkeerde daarom. Een daaropvolgende gelijktijdige, alleen-lezen controle (`novabot-source-lora-flags-concurrent.json`) zag tien nieuwe RobotStatus-berichten met code 8, terwijl beide actuele `/chassis_incident`-berichten `error_lora=false` en `warning_lora_rtk_data_overtime=false` meldden. De algemene code alleen beschrijft de actuele meetgezondheid dus onvoldoende. De exacte oorzaak van het blijven staan van code 8 is niet vastgesteld; er is niets gereset.

De meetopdracht accepteert status 0 of 8 (en sinds de hieronder beschreven controle ook 113) als tevens de actuele LoRa-fout- en waarschuwingvlag expliciet onwaar zijn, voor en gedurende het hele meetvenster. Er zijn vóór inschakelen twee gezonde chassisberichten nodig. Ontvangstouderdom en tussenpozen mogen maximaal 3 seconden zijn, passend bij de gemeten publicatiecadans van 2 seconden. Chassisstempels zijn nul; hiervoor wordt ontvangsttijd gebruikt. BestPos moet daarnaast verse, unieke ontvangerstempels, `qual=4` en een eindige correctieouderdom van 0 tot 3 seconden hebben. De bestaande stilstand-, LOC_SUCCESS- en geometriecontroles blijven staan. Iedere andere fout, ontbrekend veld, ongezonde vlag of ontbrekende/verouderde meetstroom blokkeert. Dit is uitsluitend een controle voor stilstaande metingen, geen algemene onderdrukking van LoRa-fouten.

De eerste vier diagnostische opnamen bevatten nog geen gelijktijdige chassisgegevens en kunnen daarom niet achteraf als volledig geslaagde productmeting gelden. De verdere opnamen nemen die gegevens mee. De handler is nog niet gedeployed.

### Verse bronronde op 27 september

Na de onderbreking worden bron en doel opnieuw in één meetronde gecontroleerd. De eerdere geometrische proef blijft onderzoeksbewijs, geen actuele serverregistratie. De ruwe gegevens van deze ronde staan lokaal in `research/captures/2026-09-27-dock-marker/` (gitignored).

.100 meldt bij handmatig positioneren nog fout 113. Stockcode (`chargingResultCallback` → `monitorRechargeWorking` → `rechargeFinishedDeal`) koppelt deze aan een voltooide, mislukte AutoCharging-actie met `result.code == 3`; het is geen actuele cameragezondheidsmelding. De setter gebruikt `updateErrorStatus(113, false)` (ELF `0x8d1e4`). De meetcontrole staat daarom uitsluitend bij stilstand ook 113 toe om detectorobservatie te proberen. Accepteren vereist vervolgens nieuwe geslaagde markerwaarnemingen, alle actuele chassis-/RTK-/lokalisatiecontroles en de ongewijzigde meetgrenzen. Andere foutcodes blijven geweigerd; er wordt niets gewist of gereset en geen nieuwe dockactie gestart. De bestaande foutafhandeling voor navigatie blijft behouden.

De eerste opname van 27 september herkent het patroon op circa 72 cm camera-afstand. Gemiddelde markerpose over de opname: `(0,114237; -0,057890; 0,092254)` m, yaw `89,6118°`; maximale XY-spreiding 0,827 cm. Het middelste 12-secondenvenster slaagt voor de werkelijke meetvalidator met 36 verse beeldstempels, tien gezonde chassisberichten in de volledige opname en RTK Fixed met correctieouderdom 1,2–2,0 s. De detector is daarna bevestigd uitgezet.

De tweede opname herkent hetzelfde patroon vanaf circa 36 cm camera-afstand. Ook dit vooraf gekozen middelste 12-secondenvenster slaagt (37 verse beeldstempels). Tussen beide gevalideerde vensters is de voertuigverplaatsing **36,065 cm**, het markerverschil **1,247 cm XY / 2,092 cm 3D** en het yawverschil **0,3702°**. Beide opnamen hebben gezonde actuele chassis-, RTK- en lokalisatiegegevens en bevestigde detectoruitschakeling. Een onafhankelijke herberekening accepteert ook de aangrenzende vensters (+4 en +5 seconden start). Daarmee voldoet dit verse bronpaar aan de individuele meet- en herhaalgrenzen. De volledige opnamen bevatten slechts twee respectievelijk één verschillende ArUco-pose; interne spreiding blijft dus geen zelfstandig nauwkeurigheidsbewijs. Dit is diagnostische replay, nog geen actieve serverregistratie, doelmeting of toegepaste kopie.

### Eerste doelmeting: patroon zichtbaar, correcties te oud

.244 is vervolgens vóór hetzelfde fysieke dock van .100 gezet. De eerste opname (`novabot-aruco-target-20260927-first.json`) geeft 60 verse beeldstempels en herkent het patroon op circa 53 cm camera-afstand. Detectorinschakeling en -uitschakeling zijn bevestigd. De meetvalidator weigert het venster echter wegens de correctieouderdom: 86 van 101 verse BestPos-berichten hebben `diff_age > 3 s`. De leeftijd loopt met tussentijdse terugsprongen van 0,6 tot 10,8 seconden op. Alle berichten blijven `qual=4`; ook LOC_SUCCESS en beide gezonde LoRa-vlaggen blijven staan. Deze velden alleen bewijzen dus onvoldoende verse correcties. De aangrenzende vensters worden eveneens afgekeurd.

Een daaropvolgende alleen-lezen controle ziet correctieouderdom van 8,6 tot 15 seconden, kort ook `qual=2`, waarna weer correcties van 0,8–1,6 seconden binnenkomen. Dit bewijst onderbroken tijdige ontvangst op die plek, maar onderscheidt verlies, vertraging of achterstand niet. Er wordt geen doelregistratie bewaard en geen vertaling toegepast op basis van de afgekeurde opname. De grenzen blijven ongewijzigd.

Een verdere controle van 24 seconden, met de detector uit, bevestigt het probleem: alle 121 berichten melden Fixed, maar 97 hebben correcties ouder dan 3 seconden (maximum 14,6 seconden). Het eigen laadstation van .244 meldt via de server online, 28 satellieten en `rtk_ok=1`; dat is geen bewijs van tijdige overdracht naar de maaier. De ontvangst moet eerst stabiel zijn voordat een nieuwe doelmeting bruikbaar is.

### Terugcontrole en camerahoekconflict

Na circa 1,48 m achteruit rijden ontvangt .244 gedurende 24 seconden 121/121 Fixed-berichten met correcties van 0,8–1,6 seconden. Na terugrijden zijn ook alle 120 berichten binnen de grens (0,8–2,6 seconden). De terugstandplaats verschilt volgens de API circa 28 cm van de eerste; dit bewijst dus geen exacte A-B-A-herhaalbaarheid of storing door de andere dockzender. De eerdere uitval mag niet uitsluitend aan afstand of interferentie worden toegeschreven.

De daaropvolgende markeropname (`novabot-aruco-target-20260927-return.json`) slaagt individueel in het vooraf gekozen middelste 12-secondenvenster: 36 verse beeldstempels, XY-spreiding 5,15 mm en maximale tijdkoppelfout 0,0351 s. Een correctie van 3,2 seconden valt vóór dit venster; de volledige opname is daarmee niet volledig foutloos. De detector is bevestigd uitgeschakeld. De camerafstand is circa 79 cm.

De berekende marker-yaw is echter **84,877°**, tegenover **89,615° en 89,985°** in de bronmetingen. De koppeling voldoet daarom niet aan de bestaande headingcontrole. Onafhankelijke reconstructies bevestigen dat de volledige SE3-omrekening, statische camera-/GPS-transforms en inverse kloppen. Ruwe camerapose, `/orig` en `/pose` zijn onderling consistent; er is geen filtereffect. Ook na verwerking van de voertuigkanteling verandert de berekende marker-pitch in het .244-kaartframe circa **23,73°** tussen de eerste en terugopname. Bij .100 verschilt deze pitch circa **9,95°** tussen de twee bronopnamen, ondanks de eerder geslaagde XY/yaw-controle.

Dit wijst op standpuntafhankelijke onzekerheid in de camerapose. PnP-ambiguïteit, hoekdetectie en kalibratie zijn mogelijke oorzaken; zonder pixelanalyse is de precieze oorzaak niet vastgesteld. De eerste .244-opname had onvoldoende verse RTK, zodat het onderlinge wereldpositieverschil niet uitsluitend aan de camera kan worden toegeschreven. De huidige markeruitvoer is hiermee nog niet gevalideerd voor de vereiste kaartnauwkeurigheid. Een latere ongewijzigde-camera-opname is lokaal bewaard voor offline beeldanalyse; geen instellingen of kaarten zijn aangepast.

De offline analyse van dat latere beeld vindt beide custom 8×8-markers (IDs 0 en 1), elk circa 40×40 pixels. De uit de stockbinary gereconstrueerde opstelling is een 2×1-bord, markerzijde 0,0624 m en tussenruimte 0,02776 m. Met dezelfde CONTOUR-hoeken geeft de IPPE-berekening twee oplossingen met roll **+17,33°** en **−11,78°**, met reprojectiefouten van respectievelijk **0,718 en 0,772 pixel**. Een kantelverschil van 29,1° past dus bijna even goed op dit beeld. Dat toont ambiguïteit in de beschikbare beeldinformatie. Het is geen exacte herhaling van een eerdere stockberekening: deze offline analyse gebruikt OpenCV 5.0.0 tegenover stock 4.6-dev en een later camerabeeld.

Alleen SUBPIX inschakelen is evenmin gevalideerd als oplossing: kleine verschillen in hoeken en venstergrootte veranderen ook de afstand met centimeters. De volgende fysieke proef gebruikt daarom een duidelijk schuin zijaanzicht. Recht voor-/achteruit rijden verandert vooral afstand en levert hier onvoldoende bewijs dat de camerapose eenduidig is. Een zijaanzicht en eventuele latere aanpassing van de wizard moeten nog worden getoetst; de bestaande grenzen zijn niet verruimd. Script en resultaten zijn lokaal bewaard bij de opnamen.

De hoekambiguïteit mag niet gelijkgesteld worden aan een even grote fout in de plaats van het bordmidden. Voor dezelfde twee CONTOUR/IPPE-oplossingen verschilt `t + R * (0,07628; 0,0312; 0)` slechts **0,636 mm in 3D**. De bestaande tag-origin correspondeert met dit bordmidden. De yaw-controle kan daarom een stabiele centrumpositie afwijzen wegens een onvoldoende bepaalde bordhoek. Dit ene beeld bewijst geen absolute nauwkeurigheid en geen herhaalbaarheid tussen standpunten. Een eventuele vervanging van de hoekcontrole vereist afzonderlijk bewijs voor de gemeenschappelijke kaartassen en herhaalbare centrumposities; de controle is nog niet gewijzigd. Andere hoekverfijning kan bovendien wel centrumverschillen geven: circa 4 mm bij de OpenCV-5-default voor SUBPIX en 25 mm bij een vast SUBPIX-venster van 10 pixels.

### Slapende camera en herhaling vanuit dezelfde standplaats

Een volgende opname ontvangt uitsluitend een circa 451 seconden oude markerpose en geen nieuwe beelden. De frontcamera staat volgens `/status` uit; het log bevestigt `Geen viewers voor 304s` gevolgd door een geslaagde `stop_camera`. De meetopdracht activeert alleen de detector, niet de camera. De validator weigert terecht wegens ontbrekende verse markerbeelden. De bestandsnaam `novabot-aruco-target-20260927-side.json` beschrijft de geplande proef, niet een bewezen zijaanzicht: de voertuigpose is vrijwel onveranderd. De zijaanzichtproef is hiermee nog niet uitgevoerd.

Een gewone frontsnapshot via de bestaande cameraserver wekt de camera. De daaropvolgende opname (`novabot-aruco-target-20260927-camera-wake.json`) levert 60 verse markerbeelden en 101/101 Fixed-berichten met correctieouderdom 0,8–2,6 seconden. Het middelste 12-secondenvenster slaagt met 36 beelden; de detector is daarna bevestigd uitgeschakeld. Het gemeten dockpunt verschilt 1,327 cm XY van het eerdere geldige terugvenster, terwijl de voertuigpose 1,353 cm verschilt. Dit is een herhaling vanuit vrijwel dezelfde standplaats en voldoet niet aan de vereiste 15 cm verplaatsing. De marker-yaw verandert circa 0,995°; de vergelijking met de bron blijft buiten de headinggrens. Er is geen registratie of kaartwijziging toegepast.

De wizard moet de frontcamera zelfstandig kunnen activeren en gedurende de opname beschikbaar houden. Alleen de detector inschakelen laat een verborgen afhankelijkheid van een geopende cameraweergave bestaan. De bestaande `front`- en `front_hd`-watchdogs delen dezelfde fysieke camera; ook een oude ArUco-viewer kan een eigen uitschakeltimer hebben.

De lokale correctie onder `Novabot-55f.23.1` vraagt de bestaande cameraserver om bevestigde camera-activatie en maximaal 30 seconden bescherming tegen zijn uitschakeltimers. Actieve of nog startende ArUco-weergaven blokkeren een meting; gewone frontvideo blijft beschikbaar. Opruimen geeft alleen de eigen bescherming vrij, na bevestigde detectoruitschakeling, en zet de frontcamera niet uit. Bij ontbrekende uitschakelbevestiging mislukt de meting en verloopt de bescherming vanzelf. Gedeelde camera-aliassen gebruiken gezamenlijke kijkers en meest recente activiteit. Een stream zonder eerste beeld beëindigt na 15 seconden zijn viewerregistratie. Dit coördineert de HTTP-cameraserver, niet alle onafhankelijke native ROS-consumenten.

De camerastart gebruikt maximaal 2 seconden service-discovery en 3 seconden ACK-wacht, zonder CLI-fallback; het HTTP-begin/einde maximaal 6/1 seconde. De serveropdracht heeft 50 seconden ruimte voor camerastart, gezondheid, meetvenster en cleanup. Verse ROS-markerbeelden en alle bestaande meetgrenzen blijven verplicht. De gerichte controles slagen: 7 camera-, 8 marker- en 22 registratiechecks. Deze wijziging vereist zowel `camera_stream.py` als `extended_commands.py`; zij is nog niet op de maaiers geplaatst of vanuit volledig uitgeschakelde camera via de echte wizard gevalideerd.

### Eerste zijwaartse standplaats

De opname `novabot-aruco-target-20260927-angle-check.json` volgt op een echte zijwaartse verplaatsing. In het vaste middelste venster is de voertuigverplaatsing ten opzichte van de camera-wake-opname **15,083 cm XY**, waarvan **15,003 cm naar rechts** en 1,544 cm achteruit in de eerdere voertuigassen. De heading verandert 8,518°. Beide patronen zijn volledig zichtbaar. Het markercentrum verschilt **1,094 cm XY / 1,727 cm 3D**, maar de berekende marker-yaw verandert **5,619°** en de marker-pitch circa **27,29°**. Dit past bij een onzekere bordoriëntatie, terwijl het centrum veel minder verandert.

Dit is geen geaccepteerde herhaalproef: de validator weigert wegens RTK-correctieouderdom. Alle 102 BestPos-berichten melden Fixed, maar 42 hebben correcties ouder dan 3 seconden, met een maximum van 7,4 seconden. Het middenvenster bevat eveneens te oude correcties. Detectoruitschakeling is bevestigd. De geometrische cijfers zijn uitsluitend diagnostisch; zij bewijzen geen nauwkeurigheid onder gezonde ontvangst. De gewijzigde kijklijn is circa 7° en biedt nog een beperkt ander perspectief. Een extra zijwaartse verplaatsing van ongeveer 20 cm is gevraagd voor de volgende opname.

### Groter zijaanzicht en ontleding van het positieverschil

De daaropvolgende `sidewide`-opname heeft ten opzichte van `camera-wake` een baseline van 43,76 cm, waarvan 40,78 cm zijwaarts, en een headingverandering van 24,463°. Dit is een duidelijk ander gezichtspunt. Een tweede `sidewide-repeat`-opname is zonder verplaatsing gemaakt. Beide vaste middelste vensters worden afgekeurd wegens correctieouderdom; de volledige opnamen bevatten respectievelijk 16/102 en 8/101 berichten met `diff_age > 3 s`, maximaal 4,6 seconden, ondanks uitsluitend Fixed. Beide opnamen eindigen met bevestigde detectoruitschakeling.

Een onafhankelijke ontleding koppelt BestPos en `odom` op exact gelijke tijdstempels en projecteert GNSS met `pyproj` naar EPSG:32632. De verschillen hieronder zijn horizontaal, steeds tegenover het gezonde `camera-wake`-venster:

| Grootheid | Sidewide | Herhaling zonder rijden |
|---|---:|---:|
| Markercentrum in native kaartframe | 5,16 cm | 7,45 cm |
| Verandering effectieve offset `odom_gps_link - GNSS_UTM` | 6,12 cm | 6,14 cm |
| Markercentrum bij uitsluitend vervangen van odompositie door GNSS | 1,70 cm | 1,38 cm |

Voor sidewide is de vectorontleding `(-3,364; +3,908) = (-2,724; +5,478) + (-0,641; -1,570)` cm. De veranderde verhouding tussen GNSS en de effectieve navigatiepositie verklaart dus het grootste deel van het centrumverschil. Dit is niet rechtstreeks een uitgelezen interne compensatievariabele: de precieze oorzaak, waaronder referentiepunten of filtering, is hiermee niet vastgesteld. Het is ook geen bewijs dat ruwe GNSS fysiek correcter is of geschikt is als vervanging van de navigatiepositie.

Bij de stilstaande herhaling verschuift GNSS 2,55 cm en het native markercentrum 2,57 cm, terwijl het ruwe cameracentrum slechts 0,029 mm verandert. De effectieve offset verandert daarbij circa 0,027 cm. Roll/pitchverandering tussen camera-wake en sidewide verklaart slechts circa 1,06 mm horizontaal. Er spelen dus zowel veranderende GNSS-/odom-verhoudingen tussen standplaatsen als GPS-variatie tijdens stilstand mee; de camera is niet de enige foutbron. De ruwe-GNSS-berekening blijft diagnostiek, met afgekeurde RTK-gezondheid en zonder onafhankelijke grondwaarheid.

De bekende GPS-leverarm zou bij deze rotatie een verandering van `(-7,763; -1,422)` cm geven, tegenover de gemeten `(-2,724; +5,478)` cm. Alleen een ontbrekende of dubbel toegepaste bekende leverarm verklaart dit dus niet. Het reproduceerbare script `novabot-sidewide-frame-analysis.py`, de resultaten en een snapshot van de pure meetvalidator zijn lokaal bij de captures bewaard, inclusief inputhashes en exacte gekoppelde tijdstempels.

De huidige herhaalcontrole blokkeert deze opnamen al. De continuïteit van het navigatieframe tijdens de volledige kopieerprocedure, inclusief terugrijden naar het eigen dock, vraagt afzonderlijke controle; uitsluitend dezelfde oorsprongbestanden bewijzen die niet. Dit wordt gevolgd onder `Novabot-55f.23.2`. Er zijn op basis van deze proeven geen geometrie, meetgrenzen of navigatiecoördinaten gewijzigd.

### Terugrit naar het eigen dock van .244

De gebruiker heeft .244 met de joystick teruggebracht naar zijn eigen dock en laadcontact bevestigd. Een uitsluitend lezende opname van 285,19 seconden eindigt na 30 seconden gemelde laadstatus. De 1.420 gekoppelde BestPos-/odom-berichten hebben exact dezelfde brontimestamp. De hashes van `pos.json`, `charging_station.yaml` en `map_info.json` zijn bij begin en einde gelijk. De oorspronkelijke opname, het probescript, de analyzer en byte-identiek reproduceerbare JSON staan lokaal onder `research/captures/2026-09-27-dock-marker/novabot-return-drive-*`.

De effectieve horizontale offset `odom_gps_link - (GNSS_UTM - native_origin)` is in de vaste eerste 12 seconden gemiddeld `(-7,109; -2,210)` cm. Tijdens de grote draai blijft deze vrijwel gelijk. Op 172,375 seconden verandert de offset tijdens vooruitrijden in één meetstap met 6,668 cm richting nul. Daarna blijft hij tot opname-einde, circa 113 seconden later, rond nul. De totale verandering tussen de vaste eerste en laatste 12 seconden is **7,444 cm**. Dit bewijst een verandering zonder bestandswijziging binnen deze rit, niet een permanente toestand na toekomstige ritten of herstarts.

Bij de grootste sprong meldt de ontvanger Fixed met correctieouderdom 1,2 seconden; lokalisatie meldt 200 en de robot foutcode 0. De bekende GPS-leverarm verandert door de gelijktijdige oriëntatieverandering slechts 5,43 mm horizontaal, tegenover de gemeten 66,68 mm offsetstap. Tijdkoppeling of alleen die leverarm verklaart de stap dus niet. De exacte interne oorzaak is hiermee nog niet vastgesteld; de effectieve offset is geen rechtstreeks uitgelezen compensatievariabele.

In de vaste laatste 12 seconden zijn alle 60 paren Fixed, met correctieouderdom 0,6–1,6 seconden, laadstatus 4, lokalisatie 200 en foutcode 0. De gemiddelde voertuigpositie is `(0,00614; 0,81403)` tegenover de opgeslagen dockpose `(0,03; 0,73)`: **8,735 cm verschil**. De horizontale spreiding rond het gemiddelde is maximaal 7,17 mm. Deze opname voldoet daarmee niet aan de huidige 5 cm-eigen-dockcontrole voor kopiëren. Goed laadcontact en stabiele metingen geven geen grond om die grens te verruimen of de opgeslagen dockpose automatisch te overschrijven.

Alle 1.420 odom-berichten geven bovendien nul voor zowel lineaire als hoeksnelheid, ook tijdens duidelijk rijden. Die velden bewijzen hier geen stilstand. De bestaande controle op positie- en headingspreiding blijft noodzakelijk. Geen kaart, oorsprong, dockpose, firmware of draaiende dienst is voor deze terugritmeting gewijzigd.

De bestaande decompilatie van `RobotCombinationLocalizationNode::updateRTKContinuousStatus` toont een tijdelijk herstelanker bij terugkeer naar doorlopende Fixed. De standaard herstelafstand is 2,5 m en kan bij grote afwijking tot 6 m oplopen. `updateOdomToMapTf` gebruikt dat anker zolang de interne afstandsvoorwaarden gelden en kan daarna terugvallen op GNSS ten opzichte van de opgeslagen oorsprong, met verdere filtering. Er bestaan ook afzonderlijke annuleringen van een herkende sprong. Dit verklaart hoe de effectieve verhouding zonder bestandswijziging kan veranderen. Het is geen bewijs welk pad de stap op 172,375 seconden veroorzaakte: de interne ankerflag, afstandstellers en beslisinvoer zijn niet opgenomen. Alleen een vaste afstand rijden of een melding Fixed afwachten is daarom geen sluitende gereedheidscontrole.

De kopieerroute vóór deze correctie controleerde de registratie vóór DB-opslag, verbruikte haar daarna en startte de echte kaartinstallatie pas buiten de gezamenlijke bron-/doeloperatie. De algemene apply/retry-route kende die registratie niet. Een nieuwe continuïteitscontrole uitsluitend bij de eerste validatie zou dit gat dus niet sluiten.

### Lokale correctie voor de volledige kopieerprocedure

`Novabot-55f.23.2` voegt een verplichte runtime-referentie aan iedere markeropname toe. Een afzonderlijk `measure_runtime_frame`-commando controleert dezelfde verhouding zonder camera, detector of beweging. Het vereist minimaal 20 verse BestPos-/odom-paren met exact gelijke bronstempels over 5 tot 15 seconden, gezonde RTK/lokalisatie en stilstand op basis van positie- en headingspreiding. Ieder geselecteerd GPS-paar moet zelf Fixed met correctieouderdom maximaal 3 seconden zijn, ook als het later dan de omliggende gezondheidsberichten aankomt. De ontvanger-/navigatieverhouding mag binnen een venster maximaal 2 cm spreiding hebben. Fout 113 blijft alleen bij de markeropname met verse detectorwaarnemingen toegestaan; de cameravrije controle accepteert 0 of 8 met gezonde actuele chassisflags.

De server vereist bovendien maximaal **2 cm horizontale effectieve offset ten opzichte van nul**. De onderzochte normale native `gps_link`-invoer is GNSS-UTM minus oorsprong, zonder aanvullende constante XY-term. Een stabiele tijdelijke correctie kan pas bij later rijden verdwijnen en is dus ook ongeschikt voor registratie. Deze extra grens wijst zo'n opname af; hij trekt geen correctie van de marker af en bewijst geen absolute nauwkeurigheid. Afwijkende firmware of referentiepunten worden hiermee eveneens geweigerd en moeten afzonderlijk worden onderzocht. Een vaste rijafstand is geen alternatief voor een gezonde meting die deze controle doorstaat.

Bij preview en vlak vóór installatie meet de server beide maaiers opnieuw. Iedere verse offset moet daarnaast binnen **2 cm van elk van de twee oorspronkelijke opnamen van die maaier** liggen. Een echte afwijking maakt de sessie ongeldig. Beide bestaande kaartoperaties blijven exclusief bezet tot na installatie, teruglezing en eindcontrole. De brongeometrie en native framebestanden worden opnieuw gecontroleerd; de doelbestanden mogen sinds het voorbereiden van de ZIP niet gewijzigd zijn. Alleen de verwachte doelrevisie door de installatie mag tijdens de eindcontrole verschillen.

`installZoneCopy` maakt de ZIP uit de bevestigde native doelbestanden, behoudt bestaande CSV-inhoud en voegt de nieuwe geometrie toe. Vóór overdracht worden de oude toestand en de voorgestelde ZIP opgeslagen onder `storage/zone-copy/<operation-id>/`. De bestaande installatieroutine wacht op devicebevestiging, gridopbouw en exacte teruglezing. Vervolgens worden runtime, actuele doelbestanden, eigen dock en rasters gecontroleerd. Pas daarna worden de DB-kopie en laatste ZIP definitief opgeslagen, de weergavereferentie hersteld en de registratie verbruikt. Een onbekend of afgewezen installatieresultaat laat de oude DB-kaart staan en het navigatieframe geblokkeerd. Die blokkade overleeft een serverherstart en verhindert een generieke apply/retry.

Inkomende native/app-kaartuploads, aliaswijzigingen en MQTT-kaartmetadata mogen tijdens de operatie of een ongevalideerd frame de database niet alsnog wijzigen. De native upload overschrijft dan evenmin de laatste ZIP; commandobevestigingen blijven wel verwerkt worden. Dit sluit de omweg rond de bevestigde overdracht.

De cameralevensduurwijziging vereist `camera_stream.py` samen met de bijgewerkte `extended_commands.py` op beide maaiers en de bijbehorende servercode. Oudere meetresultaten zonder runtime-referentie worden geweigerd. De fysieke dockafwijking, marker-headingcontrole en onafhankelijke controle van de gekopieerde grens blijven open; deze codewijziging verplaatst geen dock of kaart om die controles passend te maken.

Lokale verificatie: TypeScriptcontrole, 174 gerichte servertests, 27 Python-tests en de dashboardbuild geslaagd. De fotoreferentie blijft ook bij een mislukte CSV-installatie behouden; een echte framewijziging maakt haar nog steeds ongeldig. `release-beta.sh` werkt alleen de serverimage bij. De twee Python-bronbestanden moeten daarnaast naar `/root/novabot/scripts/` op beide maaiers, gevolgd door het gericht herladen van hun processen. Controleer eerst de bestaande launcher en eventuele respawnlus, zodat geen dubbele processen ontstaan. Een volledige firmwareflash of herstart van de maaier is hiervoor niet nodig.

### Eerste live installatie en controle

Op 27 september is de juiste serverbeta op de NAS bevestigd aan de aanwezige runtimecontrole en `installZoneCopy`. Beide maaiers kregen alleen de twee Python-scripts. De oude bestanden staan op iedere maaier in `/root/novabot/script-backups/20260927-runtime-frame/`; de bestaande cameraloop herstartte uitsluitend haar Python-kind en extended commands werd eenmaal via de bestaande ROS-launcher gestart. Bestandsinhoud, procesaantallen, camera-HTTP-status en gecorreleerde MQTT-runtimeantwoorden zijn gecontroleerd. De gecontroleerde oorsprong, dock-YAML en CSV-bestanden bleven gelijk. De volledige camerawizard is daarmee nog niet fysiek gevalideerd.

Op het eigen dock gaf .244 een stabiele runtime-offset van 58,90 cm (59 exacte paren, 2,36 mm spreiding). Een afzonderlijke 30-secondenopname bevatte 152 exacte paren; de laatste 12 seconden hadden allemaal Fixed, correctieouderdom maximaal 1,4 s, lokalisatiestatus 200, foutstatus 0 en laadstatus 4. De uit volledige antenne-TF berekende gemiddelde voertuigpositie was `(-0,40421, 1,38961)` tegenover opgeslagen dock `(0,03, 0,73)`: **78,97 cm verschil**. Dat is groter dan de eerdere 8,735 cm; het is actuele lokalisatie, geen verandering van de opgeslagen kaart. Er is geen herankeractie uitgevoerd. .100 gaf op zijn dock een runtime-offset van 0,484 mm met 0,315 mm spreiding.

De livecontrole vond tevens klokverschillen: .244 circa 4,3 s en .100 circa 31,7 s vóór de NAS/Mac, met circa 0,3 s meetonzekerheid. Absolute vergelijking van hun opnamestempels met de serverklok wijst daardoor actuele metingen af. De aanvullende servercorrectie gebruikt de bestaande unieke commandocorrelatie en monotone verstreken tijd. Alle tijd buiten het gemeten opnamevenster, inclusief start, transport en verdere teruglezing, telt conservatief mee als mogelijke ouderdom; meer dan 10 seconden wordt geweigerd. De server controleert dit opnieuw na alle awaits. Hergebruikte of teruglopende vensters worden per maaier geweigerd; de runtimeopname moet binnen het bijbehorende markervenster liggen. De Python-controle op lokaal verse samples blijft gelijk en de apparaatklokken worden niet verzet. Deze extra servercorrectie is lokaal getest en vereist nog een nieuwe beta. Ruwe antwoorden, voor-/na-hashes, klokvergelijking en de onafhankelijke dockopname staan lokaal onder `research/captures/2026-09-27-dock-marker/script-update/`.

### Terugkeer op het eigen dock en correctie van herankeren

Na circa drie meter handmatig achteruitrijden van het eigen .244-dock verdween de tijdelijke runtime-offset. Terug op hetzelfde dock bleef de offset circa 0,48 mm. De onafhankelijke opname bevatte 99 exacte GNSS/odomparen; de laatste twaalf seconden waren Fixed met correctieouderdom maximaal 1,4 s, lokalisatiestatus 200, foutstatus 0 en laden. De voertuigpose lag nog **14,4 cm** van het opgeslagen dock; `map_position` bevestigde circa 14,5 cm. `pos.json`, dock-YAML en mapmetadata bleven ongewijzigd. Dat voldoet niet aan de 5 cm-dockvoorwaarde voor kopiëren.

De herankerhandler bevatte daarnaast een concrete referentiefout: hij gebruikte de antenne-UTM als voertuig-UTM. De opmerking dat visueel docken het restant corrigeert was onjuist. Dit bewijst de codefout, niet dat de volledige huidige dockafwijking door die fout ontstond. De lokale correctie gebruikt de bestaande runtime-observer met volledige `base_link -> gps_link`-transformatie en actuele quaternion. Zij vereist een gedockte, gezonde meetperiode en een effectieve runtime-offset van maximaal 2 cm. De oorsprong wordt `oude_origin + gemeten_base − runtime_offset − vast_dockanker`.

De server bewaart native bestanden en voorstel vóór schrijven. Het nieuwe `base-link-reanchor-v1`-protocol verlangt dat de maaier zelf opnieuw meet en de voorgestelde oorsprong binnen 2 cm bevestigt voordat hij `pos.json` schrijft en live laadt. Een gewijzigd frame, verouderde meting, verdwenen laadcontact of oude lat/lng-payload weigert hij. De server controleert protocol en teruggelezen oorsprong; CSV, x3, dock-YAML en rasters moeten onveranderd zijn. Een al passend frame wordt zonder oorsprongwijziging geverifieerd. Na een write blijft uitrijden en handmatig terugkeren verplicht, gevolgd door een nieuwe volledige runtime-/voertuigmeting binnen 5 cm van het vaste dockanker. Een tijdelijke compensatie kan geen vrijgave opleveren.

Replay van de opgeslagen .244-opname stelt een oorsprongwijziging van ongeveer (−7,36, +12,41) cm voor; dat is niet uitgevoerd. Lokale verificatie: 107 servertests, 21 Python-tests en TypeScriptcontrole geslaagd. Referentiewiskunde is ook getest met kanteling en onafhankelijke PROJ-punten. De fix is nog niet gedeployed: Ramon publiceert de serverbeta via het bestaande `release-beta.sh` en werkt de NAS bij. Voor de eerste praktijkproef krijgt .244 daarna uitsluitend de actuele `extended_commands.py` en een gerichte reload van dat proces. `camera_stream.py` en de volledige firmware hoeven hiervoor niet opnieuw. Daarna zijn nieuwe metingen en fysieke acceptatie nodig; bestaande markeropnamen zijn geen geldige nieuwe registratiesessie. Beads `Novabot-55f.19` en `Novabot-55f.23.2` blijven hiervoor open.

## Vervolgprincipes, afzonderlijk van deze wizard

Een blijvend tuinreferentiestelsel met meerdere fysieke meetpunten is een mogelijke vervolgstap, geen eigenschap van de huidige sessie. Gebruik daarvoor reproduceerbare grondpunten verspreid over het maaigebied, afzonderlijke berekenings- en controlepunten, herhaalde plaatsingen en een terugkeer naar het beginpunt. De bestaande .100-kaart kan het numerieke referentieframe leveren; fysieke controlepunten voorkomen dat een latere lokalisatieafwijking stilzwijgend de hele tuin verschuift. Een blijvende registratie vereist eigen opslag, invalidatie en onafhankelijke fysieke validatie voordat zij de verplichte bezoeken zou kunnen vervangen.

Nieuwe grenzen kunnen zonder foto met de maaier worden opgenomen. Een gewone satellietfoto is een achtergrond of benadering, geen garantie voor enkele centimeters. Een optionele drone-orthofoto mag afzonderlijk aan gecontroleerde grondpunten worden gekoppeld, met onafhankelijke beeldcontrolepunten. Verplaatsen van een weergavepin of vervangen van een foto mag bestaande navigatiepolygonen niet wijzigen. Absolute WGS84-nauwkeurigheid is een afzonderlijke kalibratievraag.

Een gedeelde RTK-basis is geen voorwaarde voor een gemeten lokale koppeling. De maaiers hetzelfde LoRa-paar geven is geen geschikte oplossing: die verbinding draagt ook dockhandshake, status en besturing. Externe meetapparatuur en wijzigingen aan die radiokoppeling vallen buiten deze flow.

## Implementatiepunten

- `dashboard/src/components/map/MowerMap.tsx`: verplichte vijfstappenwizard; `dashboard/src/api/client.ts`: alignment-, preview- en copyaanroepen.
- `server/src/services/copyAlignment.ts`: tijdelijke sessie, vier metingen, herhaalcontrole en omrekening. `server/src/routes/dashboard.ts`: preview/apply accepteren alleen een passende voltooide `alignmentId`, geen clientcoördinaat als alternatief.
- `research/extended_commands.py`: lokale meetopdracht voor de bestaande ArUco-voorziening en cameravrije runtimecontrole; op beide maaiers geplaatst.
- `server/src/services/zoneCopy.ts` en `dockChannelRepair.ts`: bestaande dock-, geometrie- en kanaalcontroles blijven de gedeelde basis.
- `server/src/services/frameValidation.ts` en `dockPhotoReference.ts`: navigatieframe en optionele weergavekalibratie blijven afzonderlijk.

## Achtergrond bij optionele verdere kalibratie

- [Trimble: basiscoördinaten en samenhang tussen meerdere bases](https://help.fieldsystems.trimble.com/trimble-access/2021.10/en/GNSS-base-coordinates.htm).
- [Trimble: factoren die RTK-nauwkeurigheid beperken](https://receiverhelp.trimble.com/oem-gnss/position-modes-critical-factors-rtk.html). Dit zijn geen Novabot-productspecificaties.
- [Pix4D: relatieve en absolute beeldnauwkeurigheid](https://support.pix4d.com/hc/en-us/articles/202558889).
- [Pix4D: grondreferenties en onafhankelijke controlepunten](https://support.pix4d.com/hc/en-us/articles/115000140963).

Deze principes ondersteunen de vervolgopzet; de lokale apparaatmetingen bepalen of de concrete Novabot-methode voldoet.

## Automatische bronmeting (28 september 2026)

De kopieerwizard biedt bij de eerste bronstap **Bronmaaier automatisch meten**. De aanwezige operator bevestigt het eigen dock en een vrije rechte uitrit van 1,2 m. De bronmaaier rijdt langzaam circa 50 cm achteruit, stopt voor de bestaande camerameting, rijdt nog circa 20 cm achteruit, meet opnieuw en keert met `auto_recharge` visueel terug. De doelmaaier krijgt geen bewegingsopdracht. Maak voor zijn latere metingen zo nodig ruimte met de bronmaaier, zodat hetzelfde bronpatroon zichtbaar en bereikbaar is; dock de doelmaaier daar niet.

De nieuwe servercyclus houdt één exclusieve kaartoperatie vast vanaf de voorcontrole tot de eindcontrole. De bestaande `measureReanchorDock` voert vóór en na de rit alleen de voertuig-/GNSS-meting en bestandscontrole uit, zonder oorsprongwijziging. Zowel het dock als de tijdelijke runtime-offset moeten voldoen aan de bestaande grenzen. Beide bronopnamen gaan door dezelfde `captureCopyAlignment`-validatie als handmatige opnamen. Een mislukte of afgebroken cyclus levert geen bruikbare registratie op.

`dock_measurement_move` in `extended_commands.py` ontgrendelt het dock via het bestaande ROS-signaal en bewaakt de getekende achterwaartse verplaatsing bij 0,12 m/s. Grenzen: maximaal 0,7 m per reverse-opdracht, maximaal 6 cm zijwaarts of overshoot, maximaal 0,12 rad draaiing, geen positiesprongen groter dan 6 cm per controle, maximaal afstand/snelheid + 2 seconden rijtijd en maximaal 1,5 seconde zonder voortgang. Verse odometrie, Fixed met correctieouderdom tot 3 seconden, LOC_SUCCESS, chassisstatus en minimaal 30% batterij zijn vereist. De visuele dockrit begint alleen voor het eigen nabije dock met een vers zichtbaar patroon, blijft binnen 1,15 m van de opgeslagen dockpose en eindigt alleen op bevestigde stilstand en laadcontact. Docken heeft een begrensd tijdvenster binnen de bestaande cameralease.

Iedere beweging vereist een eenmalige `arm`, commandocorrelatie en een native heartbeat die binnen 2 seconden moet worden vernieuwd. De browser vernieuwt zijn serverlease elke seconde; na 5 seconden zonder vernieuwing annuleert de server. Een stop is blijvend voor deze cyclus: een late heartbeat of herhaald rijcommando hervat hem niet. De stopknop, sluiten van de wizard, handmatige bediening, verlopen lease of verlies van meetkwaliteit verhindert vervolgstappen. Bij afgebroken docking worden zowel de recharge-taak als de native AutoCharging-action geannuleerd. Een stopcommando doorbreekt de native kaartlock.

Implementatie: `server/src/services/sourceDockCycle.ts`, `dashboard/src/components/map/SourceDockMeasurement.tsx` en de twee dock-measurement-handlers in `research/extended_commands.py`. De Python-regressie simuleert de echte handler met ROS-berichten en serviceantwoorden; de servertests controleren volgorde, annulering en geweigerde bevestigingen. Voor gebruik is de nieuwe server/dashboard-beta plus deze versie van `extended_commands.py` op de bronmaaier nodig. Geen wijziging aan `camera_stream.py` of volledige firmwareflash is hiervoor nodig. Deze versie is nog niet op een maaier uitgevoerd; afstand, ArUco-start en annulering moeten ter plaatse worden gevalideerd.
