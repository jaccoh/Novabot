# auto_recharge_server (libautomatic_recharge.so) — decompile en gevolgen voor het docken

Datum: 2026-09-30. Firmware: mower v6.0.2 (`research/firmware/mower_firmware_v6.0.2/install/automatic_recharge/`).
Binary: `lib/libautomatic_recharge.so` (3,2 MB, aarch64, niet gestript). De executable `auto_recharge_server` is alleen een 22 kB launcher.

Aanleiding: de begeleide cameraterugkeer (dock-return, `POST /maps/:sn/dock-return`) en de re-anchor roundtrip
roepen `/robot_decision/auto_recharge` (Trigger) aan. Op 2026-09-30 20:11 op LFIN2230700238 (.244) eindigde dat in
`No charge pose set, just step back to start for searching visual signal` → `MOVE_ROBOT_FAIL` ×3 → Error 126.
Vraag: wat doet `auto_recharge_server` precies in `INIT_CHECK`, en is er een route zonder nav2 en zonder map-write
als de maaier al vlak vóór het eigen dock staat?

## Reproduceren

```bash
/Applications/Ghidra.app/Contents/Resources/ghidra/support/analyzeHeadless <proj_dir> autorecharge \
  -import research/firmware/mower_firmware_v6.0.2/install/automatic_recharge/lib/libautomatic_recharge.so \
  -scriptPath research/ghidra-scripts -postScript DecompAutoRecharge.java \
  research/ghidra_output/auto_recharge_server_decompiled.c
```

Output: `research/ghidra_output/auto_recharge_server_decompiled.c` (82 functies: `AutoRechargeServer::*`,
`PidRechargeController::*`, `StringWorkStatus`). Let op: Ghidra laadt de .so op basis `0x100000`; een `DAT_002e0200`
in de decompile staat in het bestand op VA `0x1e0200` (`nm` geeft `initCheck` op `0x112f98`, Ghidra op `0x212f98`).

## Interface

Action `/auto_charging` (`automatic_recharge_msgs/action/AutoCharging`), goal-velden en hun offset in het goal-object
(`this+0xcb0` wijst naar het goal):

| Veld | Offset | Betekenis (uit de code) |
|---|---|---|
| `charge_pose` (PoseStamped) | `+0x00` (pose xy op `+0x28/+0x30`, quaternion `+0x40..`) | alleen gebruikt als `overwrite` |
| `overwrite` | `+0x60` | zet `this+0x4a0 = 1` (charge pose set) en kopieert `charge_pose` naar `this+0x4a8`; guide pose = charge pose + `guide_pose_offset` langs de station-yaw. Log: `Setting charge pose … setting only valid for this request!!!` |
| `non_charging_pose_mode` | `+0x61` | zonder charge pose: `true` → STEP_BACK + visueel zoeken; `false` → result code 5 `NO_CHARGE_POSE_SET` |
| `enable_no_visual_recharge` | `+0x62` | geen marker na twee zoekrondes → toch aligneren ("Using no visual mode") |
| `max_retry` | `+0x63` | 0 wordt 1 |
| `disable_charge_check` | `+0x64` | |
| `keep_alive` | `+0x65` | na CHARGING wachten op `isRobotChargingFinished` |
| `rotate_searching` | `+0x66` | in het visuele zoeken draaien als er geen marker is |

Result: `code` (SUCCESS=100, NO_VISUAL_SIGNAL=3, NO_CHARGE_POSE_SET=5, NAV_TO_GUIDE_POSE_EXCEPTION=6, CANCELED=7,
ARUCO_SERVICE_EXCEPTION=8, MOVE_ROBOT_EXCEPTION=9, TF_RELATED_EXCEPTION=10, NAV_TO_POSE_ACTION_FAIL=11,
RECHARGE_FAIL=12, OVERTIME_WORK=13), `charge_status`, `message`. Feedback: `charging_phase` (statusnaam), `in_align_mode`.

Topics/services van de node: publisher `cmd_vel` (rechtstreeks; `CChassisControl` is subscriber, `decision_assistant`
luistert alleen mee), `/led_set`, subscribers `/aruco/pose`, `/chassis_incident`, `/motor_current`,
`/chassis_node/just_for_recharge_status`, `/camera/preposition/total_gain`; clients `enable_aruco_localization`
(SetBool, `this+0x3b8`), `/perception/do_perception` (SetBool, `this+0x3a8`), `/local_costmap/set_detection_mode`
(SetBool, `this+0x3c8`), `/perception/set_infer_model`, costmap-clear services, action client `navigate_to_pose`;
service `/auto_recharge/set_charge_pose` (Trigger, `HandleSettingChargePose`).

**De node leest geen `charging_station.yaml`.** Er staat geen yaml- of bestandspad-string in de binary. De laadpose
komt per verzoek uit het goal (`overwrite`) of uit `/auto_recharge/set_charge_pose` (neemt de actuele `map`-pose van
`base_link` als laadpose, blijft staan tot herstart). De aanname in `extended_commands.py` dat
`auto_recharge_server` de yaml "bij boot cachet" klopt dus niet; het is `robot_decision` dat de pose in het goal
meegeeft (guide pose mode, log 2026-09-26 14:56:15 `Setting charge pose: x: 0.03 :y 0.73 guide pose: x: -0.04 :y 1.98`).

## Parameters (launch) → leden

`guide_pose_offset` −1,25 → `0x9a8`; `forward_dist_to_search` 0,55 → `0x9e0`; `coarse_align_offset` −0,75 → `0x9b0`;
`final_align_offset` 0,30 → `0x9d0`; `second_align_offset` −0,46 → `0x9c0`; **`base_link_to_front` 0,47 → `0x9d8`**;
`angle_range_for_align` 0,21 → `0xa00`; `final_step_back_dis` 0,45 → `0xa08`; **`init_step_back_dis` 0,10 → `0xa18`**;
`second_step_back_dis` 0,30 → `0xa10`; `rotate_speed_for_searching` → `0xa50`; `max_time_out` 60 → `0xa58`.

Vaste constanten (rodata): `DAT_002e0200` = **0,51 rad** (hoekdrempel), `DAT_002e0208` = **0,4**, `DAT_002e0210` = 0,2,
`DAT_002e0218` = 0,015 m (afstand per iteratie), `DAT_002e0220` = 6,79 rad (zoekrotatie), `DAT_002e01e8` = 0,1 s
(tf-timeout), step-back twist `linear.x = −0,15 m/s`, zoek-twist `+0,15 m/s`, iteratieslaap 100 ms, wachtslaap 200 ms.

## Statusmachine (`this+0xbe8`, vorige status `this+0xbec`)

0 WAITING, 1 INIT_CHECK, 2 CHARGING, 3 NAV_TO_GUIDE_POSE, 4 NAV_TO_GUIDE_POSE_FAIL, 5 NAV_TO_POSE_ACTION_FAIL,
6 WAIT_CHARGE_SIGNAL_FAIL, 7 SEARCHING_VISUAL_SIGNAL, 8 SEARCHING_VISUAL_SIGNAL_FAIL, 9 QUIT_CHARGE_STATION,
10 STEP_BACK, 11 MOVE_ROBOT_FAIL, 12 COURSE_ALIGN, 13 SECOND_ALIGN, 14 FINAL_ALIGN, 15 CHECK_STATION_CLEAN
(gaat direct door naar COURSE_ALIGN), 16 CHARGE_STATION_CHECK_FAIL, 17 TASK_CANCELLED, 18 TF_GETTING_FAILED,
19 RECHARGE_FAIL, 20 NO_CHARGE_POSE_FAIL, 21 WAIT_FOR_ARUCO_SERVICE_FAIL, 22 MOTOR_OVERCURRENT, 23 OVERTIME_WORK.

`autoRechargeCallback` loopt elke 200 ms door de statusmachine en eindigt op de faalstatussen 4, 5, 8, 11, 17, 18,
19, 20, 21, 23 (met zero-twist en, na RECHARGE_FAIL vanuit FINAL_ALIGN, 1,7 s achteruit op −0,2 m/s). Aan het einde
wordt `enable_aruco_localization false` gestuurd ("Stop Aruco Localization" in de aruco-log).

## `initCheck` (de routeringsbeslissing)

```
als isRobotCharging():                     → CHARGING ("Already in charging status")
als geen charge pose (this+0x4a0 == 0):
    als !non_charging_pose_mode:           → NO_CHARGE_POSE_FAIL (code 5)
    anders: log "No charge pose set, just step back to start for searching visual signal"
            wacht 200 ms op enable_aruco_localization → STEP_BACK (10), anders WAIT_FOR_ARUCO_SERVICE_FAIL
anders (charge pose gezet):
    robot = actuele pose in map (tf map→base_link, 0,1 s)   ; mislukt → blijft INIT_CHECK (en wist de pose-vlag bij overwrite)
    station_yaw = yaw(charge_pose)          robot_yaw = yaw(robot)
    forward_yaw = atan2(charge − robot)     dis = |charge − robot|
    log "forward_yaw: %.2f station_yaw: %.2f dis: %.2f diff: %.2f"   (diff = wrap(station_yaw − forward_yaw))
    aligned = |wrap(station_yaw − forward_yaw)| < 0,51 rad  EN  |wrap(station_yaw − robot_yaw)| < 0,51 rad
    als aligned en dis < base_link_to_front + 0,4  (= 0,87 m)  → "Robot is too close … step back" → STEP_BACK
    als aligned en dis < base_link_to_front + 1,0  (= 1,47 m)  → SEARCHING_VISUAL_SIGNAL (geen nav2)
    anders                                                     → NAV_TO_GUIDE_POSE (nav2 in het kaartframe)
```

`station_yaw` is de heading van de gedockte maaier (de opgeslagen `charging_pose`-orientation, hier −1,518 rad), niet
de richting waarin het dock "kijkt". "Aligned" betekent dus: de maaier staat op de rijas vóór het dock en kijkt naar het
dock. Validatie tegen de live logs:

- 2026-09-26 14:56:16 (go_to_charge, guide pose mode): `forward_yaw: -1.62 station_yaw: -1.52 dis: 1.52 diff: 0.10`
  → 1,52 m ≥ 1,47 m → `NAV_TO_GUIDE_POSE` → daarna visueel aligneren → CHARGING. Klopt met de drempel.
- 2026-09-30 20:11:28 (onze Trigger, geen pose): `No charge pose set…` → STEP_BACK → MOVE_ROBOT_FAIL → code 6 bij
  robot_decision ("Get result from recharge action 6") → Error 126. Klopt met de code.

## `stepBack`

Doel per vorige status: vanuit INIT_CHECK of COURSE_ALIGN `init_step_back_dis` (0,10 m), vanuit FINAL_ALIGN
`final_step_back_dis` (0,45 m), vanuit SECOND_ALIGN `second_step_back_dis` (0,30 m), anders 0,4 m (of 0,2 m bij
MOTOR_OVERCURRENT). Uitvoering: `int(doel/0,015) + 5` iteraties van `cmd_vel linear.x = −0,15` met 100 ms slaap
(0,10 m → 11 iteraties → 1,1 s; dat is exact de retry-cadans in de log van 20:11:30/31/32). Daarna zero-twist en
odom-check (tf `odom`→`base_link`): **afgelegd < 0,4 × doel (= 4 cm voor 0,10 m) → "MOVE_ROBOT_FAIL, Retry step back
again"**, maximaal 3 pogingen (`this+0xda0`), dan status MOVE_ROBOT_FAIL. Bij succes vanuit INIT_CHECK →
SEARCHING_VISUAL_SIGNAL.

Waarom de maaier op 2026-09-30 in die 1,1 s geen 4 cm odom-verplaatsing haalde is **niet** vastgesteld. Uitgesloten:
de `hall_uplift`/`hall_collision`-WARN's in de chassis-log rond 20:11:28–32 zijn statuswijzigingen van de
bladhoogte-hallsensor (`hall_cutmotor cnt 0→1`) door `Setting blade height to: 90` bij de recharge-start, geen geweigerde
beweging; om 16:42:56 verschenen ze ook tijdens onze eigen (geslaagde) 0,5 m-uitrit van het dock. Op idle publiceert
niemand op `/cmd_vel` (`ros2 topic hz` 6 s: niets), dus er is geen zero-twist-override. Kandidaten: snelheidsramp van het
chassis binnen zo'n korte burst plus odom-vertraging (eerdere waarneming 2026-06-07: "MOVE_ROBOT_FAIL, schuin weg van de
dock", toen bewoog hij wel). Voor onze flows is de praktische conclusie: **STEP_BACK vermijden**, niet repareren.

## `searchingForVisualSignal`

1. `enable_aruco_localization true`, `/perception/do_perception false`, `/local_costmap/set_detection_mode true`.
2. Maximaal 30 × 200 ms (6 s) wachten op een markerdetectie (`this+0x4a1`, gezet door `arucoSubCallback`).
3. Marker gezien → "Successfully searching visual signal" → CHECK_STATION_CLEAN → COURSE_ALIGN → SECOND_ALIGN →
   FINAL_ALIGN → CHARGING. De align-fases refereren **niet** aan de charge/guide pose (grep op de veldoffsets: alleen
   `initCheck`, `navToGuidePoseDeal`, `HandleSettingChargePose` en `autoRechargeCallback` gebruiken ze); ze werken op
   `/aruco/pose` en odom via `updateRechargeLocalization` en `PidRechargeController`.
4. Geen marker → als `rotate_searching` of tweede ronde: LED aan als het donker is, **0,55 m vooruit op 0,15 m/s**
   (36 × 100 ms), dan `6,79 / rotate_speed_for_searching` s draaien; eerste keer terug naar SEARCHING_VISUAL_SIGNAL
   ("run forward, then Rotate to try again"), tweede keer SEARCHING_VISUAL_SIGNAL_FAIL (code 3), tenzij
   `enable_no_visual_recharge` → "Using no visual mode" → toch aligneren.

## Gevolgen voor OpenNova

1. **`/robot_decision/auto_recharge` (Trigger) is voor ons doodlopend.** robot_decision stuurt dan een goal zonder
   pose ("no guide pose mode", `rechargeDeal(true)`), de server doet STEP_BACK, en dat faalt op .244 met MOVE_ROBOT_FAIL.
   Bovendien meldde de Trigger op 20:11:22 binnen 0,4 s geen `success` terwijl robot_decision de taak wél startte;
   onze `cancel_recharge` werd geweigerd ("Cannot cancel recharge when recharge task is executing") en de native
   recharge liep door tot Error 126. Dit bevestigt de notitie van 2026-06-07 (CLAUDE.md "Re-anchor dock").

2. **Route zonder nav2 en zonder map-write: het `/auto_charging`-goal zelf sturen met `overwrite: true`.**
   Voorwaarden waaronder `initCheck` direct naar het visuele aligneren gaat:
   - `charge_pose` in frame `map` met stamp nu; positie/yaw in **hetzelfde, actuele** frame als `map_position`.
     Voor de re-anchor roundtrip is dat de **gemeten gedockte pose vlak vóór de uitrit** (server:
     `stablePosition(sn, {docked:true})`), niet de opgeslagen `charging_pose` uit `map_info.json`: vóór de
     origin-correctie kan die meters naast de live pose liggen en dan kiest `initCheck` nav2 naar het verkeerde punt.
   - afstand tot die pose **tussen 0,87 m en 1,47 m** (dus ≈ 1,0–1,3 m achteruit, niet 0,5 m), heading en peiling
     binnen 0,51 rad van de dock-heading.
   - marker zichtbaar (onze handler controleert dat al; `_recovery_marker_forward` staat tot 1,2 m toe).
   - `non_charging_pose_mode: false`, `enable_no_visual_recharge: false` (nooit blind docken), `rotate_searching:
     false`, `keep_alive: false`, `max_retry` klein.
   - `disable_charge_check`: de open robot_decision zet dit op `true` bij accu ≥ 85 % omdat de laadstroomcheck van
     de server dan RECHARGE_FAIL geeft ondanks contact (`robot-decision-gap-analysis.md`, regel 265). .244 staat
     op 89 %. Onze handler bevestigt contact zelf via `merged_work_status == 4`, dus `true` is hier verdedigbaar;
     dit moet live bevestigd worden.
   - Annuleren via `/auto_charging/_action/cancel_goal` (client bestaat al in `handle_dock_measurement_move`);
     de server stopt dan zelf met zero-twist (TASK_CANCELLED). robot_decision staat buiten deze route: geen
     tasktype-switch naar MAPPING, geen Error 126 uit robot_decision. Of robot_decision de laadstatus daarna netjes
     oppakt zoals bij handmatig op het dock zetten: aannemelijk, niet getest.

3. **Aangepast in `handle_dock_measurement_move` / de servercycli** (protocol `dock-measurement-motion-v4`, 2026-09-30 avond;
   reverse-antwoord geeft `start_pose`, dock vereist `charge_pose` binnen 0,95–1,40 m / 0,40 rad, goal met `overwrite`,
   slagen op laadcontact, `rememberDockedPose`/`recallDockedPose` in `dockMotion.ts`; nog niet fysiek getest):
   - `reverse`-afstand van 0,5 m naar ≈ 1,0 m (huidige limiet `.15 <= distance <= .7`) en de begrensde
     naderingsstraal (`> 1.15` m abort) mee omhoog;
   - `dock`-tak: geen `Trigger auto_recharge` meer, maar een `AutoCharging`-goal met `overwrite` + `charge_pose`
     uit de commandparameters; slagen = action SUCCESS én contact; falen = action-result, niet "service did not
     confirm";
   - dock-return (maaier handmatig geplaatst): een charge pose in het actuele frame afleiden uit de markerpose
     (`aruco_tag`→`base_link` plus dockgeometrie) of de operator op 1,0–1,3 m laten plaatsen; niet verzinnen.
   - Wacht tot 6 s op de eerste detectie in de server: als de marker in die tijd wegvalt, rijdt de server 0,55 m
     vooruit en draait; onze guard moet dan het goal annuleren (markerverlies is al een guard-conditie).

4. Losse correctie: `extended_commands.py` en `sync_map` gaan ervan uit dat `auto_recharge_server` na een
   `charging_station.yaml`-wijziging herstart moet worden. De node leest dat bestand niet; de herstart is onschadelijk
   maar zinloos voor dit doel.

## robot_decision (decompile 2026-09-30 21:38, `research/ghidra_output/robot_decision_recharge_decompiled.c`)

Binary `install/compound_decision/lib/compound_decision/robot_decision`, script `research/ghidra-scripts/DecompRobotDecisionRecharge.java`.

- **`handleAutoRechargeTask` geeft `success=true` bij acceptatie** (zet vlag `this+0x736`, log "Receiving recharge task no
  guide pose mode command for mapping task"); alleen als er al een recharge loopt `success=false` ("Cannot recharge when
  recharge task is executing"). De eerdere conclusie dat de Trigger geen `success` gaf was fout. Wat op 20:11 gebeurde:
  onze guard (`merged_work_status` moet in 0/2/4/5 zitten) brak af zodra robot_decision zijn eigen statemachine startte
  (chassis-log 20:11:23.04 `RechargeStatus = 50`), en de daarna in `finally` uitgevoerde `cancel_recharge` werd geweigerd
  ("Cannot cancel recharge when recharge task is executing"), waardoor `service()` in de `finally` met "dock service did not
  confirm" de echte fout overschreef. v4 bewaart sinds 21:41 de oorspronkelijke fout in de melding.
- **`rechargeDeal(no_guide_pose)`** in volgorde: action server bereikbaar (anders fout 111) → `recharge_status > 89` → "Already
  in recharging status, No need to recharge" (`updateErrorStatus(2)`, wist NIETS) → `error_status > 149` → "please unlock"
  → … → **`updateErrorStatus(0)`** (hier wordt een taakfout zoals 126 gewist) → 400 ms → chassis/lokalisatie-check → bij
  no-guide-pose `updateTaskType(2)` (MAPPING) → goal `non_charging_pose_mode=1, max_retry=5` (guide-pose-modus:
  `getChargingPose` → `overwrite=1, max_retry=10`, en `updateTaskType(1)` als het tasktype MAPPING was).
- **`updateErrorStatus(new)`**: `new=0` overschrijft alles ≤150; een `new<100` kan een bestaande fout ≥100 niet
  overschrijven ("Error cannot overwrite"); ≥151 alleen geforceerd. Bevestigt `server/src/mqtt/errorKind.ts`.
- **Error 126 wissen** kan dus alleen via een taakstart die `updateErrorStatus(0)` bereikt: mow-start (`coverStartDeal`),
  resume (`coverContinueDeal`) of een recharge-start terwijl `recharge_status ≤ 89`. Gedockt en ladend zou een
  `auto_recharge`/`go_to_charge` de fout wissen en daarna in `auto_recharge_server` direct op "Already in charging status"
  (`isRobotCharging`) slagen zonder beweging; wordt het laden niet als zodanig herkend, dan volgt de STEP_BACK-poging.
  Niet live geverifieerd.

## Open punten

- Oorzaak van de te kleine odom-verplaatsing in STEP_BACK (0,15 m/s, 1,1 s, < 4 cm) op .244.
- `navToGuidePoseDeal` en de align-fases zijn niet regel voor regel doorgelopen; alleen hun gebruik van de pose-velden
  en hun log-strings zijn gecontroleerd.
- Gedrag van robot_decision (work_status/recharge_status/foutcodes) wanneer `/auto_charging` buiten robot_decision om
  slaagt of faalt.
- Live bevestiging van de drempels met een eigen goal vanaf ≈ 1,0 m (droge test met `disable_charge_check` en
  supervisie).

## Bronnen

- `research/ghidra_output/auto_recharge_server_decompiled.c` (`initCheck` r. 1894–2264, `stepBack` r. 2268–2775,
  `searchingForVisualSignal` r. 2779–3397, `autoRechargeCallback` r. 10158–10971, `HandleSettingChargePose`
  r. 1535–1719, `StringWorkStatus` r. 600–690).
- `research/firmware/mower_firmware_v6.0.2/install/automatic_recharge/share/automatic_recharge/launch/automatic_recharge_launch.py`
- `…/automatic_recharge_msgs/share/automatic_recharge_msgs/actions/AutoCharging.action`
- `…/debug_sh/test_recharge.sh`, `test_recharge_rotate.sh`, `setting_auto_charge_station.sh` (Novabots eigen aanroepen)
- Live logs .244: `/root/novabot/data/ros2_log/auto_recharge_server_20260918_224743_3439.log` (r. 45–90),
  `robot_decision_20260918_224743_3454.log` (r. 461–489), `chassis_control_node_20260918_224743_3413.log`
  (r. 46694–46726, 50361–50392), `aruco_localization_20260918_224740_3220.log` (r. 1872–1883).
- Server .247 (`rvbcrs/opennova:beta` `6b5a549c`): dock-return cycli `4aa48f48` (17:21Z) en `fff72fb0` (18:10Z).
