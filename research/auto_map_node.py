#!/usr/bin/env python3
"""auto_map_node — orkestratie van de autonome boundary-rit (route B).

Standalone daemon naast extended_commands.py (importeert die als bibliotheek
voor MiniMQTT/read_config/ros2_run — import is bijwerkingsvrij). Luistert op
novabot/extended/<SN> en handelt ALLEEN de auto-map-commando's af:

  start_auto_map_test  {radiusM?, timeoutS?}  — kale volg-test (fase 0) én
                                                de volgmotor tijdens een echte
                                                opname (de server start dan
                                                eerst start_scan_map)
  stop_auto_map        {}                     — cancel de rit
  get_auto_map_status  {}                     — laatste status opvragen

Statusstroom: publiceert auto_map_status-events op
novabot/extended_response/<SN>:
  {"auto_map_status": {"phase": ..., ...}}
met phase ∈ preparing | searching_boundary | following | result | error |
aborted. Bij phase "result" zit er {"code": <int>, "name": <str>} bij.

Zie docs/superpowers/specs/2026-07-22-autonomous-mapping-design.md.
"""
import json
import math
import os
import re
import subprocess
import sys
import threading
import time

DEFAULT_RADIUS_M = 30.0     # geofence-straal vanaf startpositie (spec §4)
DEFAULT_TIMEOUT_S = 1200    # 20 min (spec: result-tabel)
GPS_STALE_S = 15.0          # vangnet: zonder verse GPS geen geofence, dus stoppen
ACTION_LOG = "/tmp/auto_map_action.log"

RESULT_NAMES = {
    0: "LOOP_CLOSED",
    1: "NO_VALID_BOUNDARY",
    2: "CANCELLED",
    3: "FOLLOW_FAILED",
    4: "SEARCHING_START_FAILED",
}


def boundary_goal_yaml():
    """Goal voor `ros2 action send_goal /boundary_follow
    coverage_planner/action/BoundaryFollow` (follow_mode=0).

    start_follow_wait + more_close_to_boundary AAN: zonder deze vlaggen
    aborteert de action direct met 'No valid boundary need robot!!!'
    (status 1) — met de vlaggen vindt hij de boundary en start het volgen
    (live bewezen 2026-10-09: zelfde positie, zelfde costmap, alleen deze
    vlaggen anders → status 3 'boundary complex' i.p.v. status 1)."""
    # inflation_radius 0.4: met de default 0.0 brak het volgen na ~25 s af
    # (FOLLOW_FAILED, meerdere posities); met 0.4 reed hij direct en bleef
    # rijden tot de goal extern geannuleerd werd (live 2026-10-09).
    return ("{follow_mode: 0, start_follow_wait: true, "
            "more_close_to_boundary: true, inflation_radius: 0.4}")


def haversine_m(lat1, lng1, lat2, lng2):
    """Afstand in meters tussen twee WGS84-punten (geofence-check)."""
    if lat1 == lat2 and lng1 == lng2:
        return 0.0
    r = 6371000.0
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dp = math.radians(lat2 - lat1)
    dl = math.radians(lng2 - lng1)
    a = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * r * math.asin(math.sqrt(a))


def should_retry(code, attempt):
    """Spec result-tabel: alleen SEARCHING_START_FAILED (4) krijgt één
    automatische retry (vanaf ~2 m verderop), daarna abort."""
    return code == 4 and attempt < 2


def parse_action_result(text):
    """Parse `ros2 action send_goal`-uitvoer → (status, result_code).

    Zoekt de LAATSTE `result: <n>` OF `status: <n>`-regel (de BoundaryFollow-
    result heet in de echte firmware `status`, live gezien 2026-07-23:
    "Result:\n    status: 1\nmsg: No valid boundary need robot!!!") en de
    `Goal finished with status: <STATUS>`-regel. Beide None zolang de action
    nog loopt of de log onvolledig is. De "Goal finished"-regel zelf matcht
    de code-regex niet (waarde is een woord, geen cijfer, en de regel begint
    niet met result:/status:).
    """
    status = None
    m = re.search(r"Goal finished with status:\s*(\w+)", text)
    if m:
        status = m.group(1)
    codes = re.findall(r"^\s*(?:result|status):\s*(\d+)\s*$", text, flags=re.MULTILINE)
    code = int(codes[-1]) if codes and status is not None else None
    return status, code


# ── Daemon ───────────────────────────────────────────────────────────────────
# extended_commands.py als bibliotheek: MiniMQTT, read_config, ros2_run, log.
# Import is bijwerkingsvrij (alles achter __main__-guard), bewezen door
# research/__tests__/test_extended_helpers.py.
_EC = None


def _ec():
    global _EC
    if _EC is None:
        import importlib.util
        p = os.path.join(os.path.dirname(os.path.abspath(__file__)), "extended_commands.py")
        spec = importlib.util.spec_from_file_location("ec_lib", p)
        _EC = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(_EC)
    return _EC


class AutoMapSession:
    """Eén rit. State + watchdog. De lock beschermt alleen schrijven/lezen van
    last_status (concurrent geraadpleegd door get_auto_map_status); de rest
    van de sessie draait op de eigen achtergrondthread zonder verdere locking."""

    def __init__(self, publish_status, radius_m, timeout_s,
                 depart_from_dock=False):
        self.publish_status = publish_status   # dict -> None (MQTT publish)
        self.radius_m = radius_m
        self.timeout_s = timeout_s
        self.depart_from_dock = bool(depart_from_dock)
        self.lock = threading.Lock()
        # Synchroon al op "preparing" zetten (niet pas in de thread) zodat de
        # already_running-gate in main() geen race heeft met een tweede
        # start_auto_map_test vlak na elkaar.
        self.last_status = {"phase": "preparing"}
        self.stop_requested = False
        self.start_gps = None                  # (lat, lng) bij start
        self.last_gps = None
        self.last_fix_mono = None               # monotone tijd van laatste NavSatFix
        self.started = time.monotonic()

    def status(self, phase, **extra):
        st = {"phase": phase, "elapsed_s": int(time.monotonic() - self.started)}
        st.update(extra)
        with self.lock:
            self.last_status = st
        try:
            self.publish_status(st)
        except Exception as ex:
            # Publiceren mag falen (MQTT-socketbreuk); last_status ligt al
            # vast onder de lock hierboven, dus de terminale fase blijft
            # zichtbaar via get_auto_map_status ook als MQTT weg is.
            try:
                _ec().log(f"[auto_map] publish_status faalde: {ex}")
            except Exception:
                pass


def _preflight_node(name_prefix):
    """Kortlevende rclpy-node voor preflight-checks, zelfde idioom als
    _wait_for_perception_data. De ros2-CLI kost op deze A55 ~4 s per
    aanroep (live gemeten, rustige tuin) en overschrijdt onder mapping-load
    z'n eigen timeout — de session_crash van 2026-10-04 (set_infer_model)
    en 2026-10-09 (topic info) waren allebei precies dit. In-process is
    milliseconden zodra de node eenmaal bestaat."""
    import rclpy
    from rclpy.node import Node
    # EIGEN context, nooit de default: extended_commands draait een shared
    # executor op de default context en twee spinners daarop racen (les uit
    # de RtkRelay-code). Mijn spin_once op de default context wurgde de
    # callback-levering voor latere default-context subscriptions — de
    # perceptie-wacht hieronder kreeg daardoor NOOIT een frame (py-spy-bewezen
    # 2026-10-09). Eigen context = volledig geisoleerd.
    ctx = rclpy.Context()
    rclpy.init(context=ctx)
    return Node(f"{name_prefix}_{os.getpid()}_"
                f"{int(time.monotonic() * 1000) % 1000000}", context=ctx), ctx


def _relay_alive(ec):
    """Is lawn_edge_relay actief? In-process publisher-count op het
    relay-topic (ros2-CLI time-outte live onder load, 2026-10-09)."""
    import rclpy
    try:
        node, ctx = _preflight_node("auto_map_relaycheck")
        try:
            deadline = time.monotonic() + 10.0
            n = 0
            while time.monotonic() < deadline:
                n = node.count_publishers("/perception/points_relabeled")
                if n:
                    break
                # discovery heeft een paar spin-ticks nodig
                rclpy.spin_once(node, timeout_sec=0.5)
            return n > 0
        finally:
            node.destroy_node()
            try:
                ctx.shutdown()
            except Exception:
                pass
    except Exception as ex:
        ec.log(f"[auto_map] relay-check faalde: {ex}")
        return False


def _wait_for_perception_data(ec, deadline_s=90.0):
    """Wacht tot er echt labeled-punten op het relay-topic stromen.

    Gedraaid als SUBPROCESS met eigen python-proces. Reden: in het
    session-proces leverde de rclpy-subscription (ook op een eigen
    Context) geen enkele frame — 0 in 90 s — terwijl een vers
    python-proces op hetzelfde moment 41-54 frames per 8-10 s op
    hetzelfde topic, dezelfde QoS-varianten ontving (live 2026-10-09,
    meerdere keren herhaald). De oorzaak zit diep in het process-lokale
    rmw/CycloneDDS/iceoryx-gedrag van dit proces (shared executor van
    extended_commands draait op de default context); tot die ondergrond
    begrepen is, is een vers proces de enige bewezen betrouwbare
    ontvanger. CLI-echo is hiervoor blind (QoS/daemon, bekend), maar
    een eigen python-subprocess werkt — het onderscheid is het
    process, niet de API.
    """
    script = (
        "import sys, rclpy, time\n"
        "from rclpy.node import Node\n"
        "from sensor_msgs.msg import PointCloud2\n"
        "deadline=float(sys.argv[1])\n"
        "rclpy.init()\n"
        "node=Node('auto_map_datacheck_subproc')\n"
        "n=[0]\n"
        "node.create_subscription(PointCloud2,\n"
        "    '/perception/points_relabeled',\n"
        "    lambda m: n.__setitem__(0, n[0]+1), 5)\n"
        "end=time.monotonic()+deadline\n"
        "while time.monotonic()<end and n[0]<3:\n"
        "    rclpy.spin_once(node, timeout_sec=0.5)\n"
        "print(n[0])\n"
    )
    try:
        out = subprocess.run(
            ["python3", "-c", script, str(deadline_s)],
            capture_output=True, text=True, timeout=deadline_s + 20.0,
            env={**os.environ,
                 "RMW_IMPLEMENTATION": "rmw_cyclonedds_cpp",
                 "ROS_LOCALHOST_ONLY": "1", "ROS_DOMAIN_ID": "0",
                 "LD_LIBRARY_PATH": os.environ.get("LD_LIBRARY_PATH", ""),
                 "PYTHONPATH": os.environ.get("PYTHONPATH", ""),
                 "AMENT_PREFIX_PATH": os.environ.get("AMENT_PREFIX_PATH", "")})
        count = int((out.stdout or "0").strip() or 0)
        ec.log(f"[auto_map] perceptie-datacheck (subprocess): {count} frames")
        return count > 0
    except Exception as ex:
        ec.log(f"[auto_map] perceptie-datacheck subprocess faalde: {ex}")
        return False


def _set_costmap_topic(ec):
    """Runtime costmap-param (NOOIT YAML, maart-les). Verifieer met param get.

    Get-first: de param staat meestal al goed (vorige sessie/eerdere run),
    en een ros2-CLI-aanroep kost 4-8 s idle en ruim meer onder load
    (gemeten 7.7 s idle, live 2026-10-09) — elke overbodige call is een
    timeout-risico voor de sessie. Timeouts ruim genomen (45 s): het werk
    is milliseconds, de CLI-startup niet."""
    g = ec.ros2_run(["ros2", "param", "get", "/local_costmap/local_costmap",
                     "obstacle_layer.pointcloud.topic"], timeout=45)
    if "points_relabeled" in (g.stdout or ""):
        ec.log("[auto_map] costmap-param stond al goed — set overgeslagen")
        return True
    ec.ros2_run(["ros2", "param", "set", "/local_costmap/local_costmap",
                 "obstacle_layer.pointcloud.topic", "/perception/points_relabeled"],
                timeout=45)
    r = ec.ros2_run(["ros2", "param", "get", "/local_costmap/local_costmap",
                     "obstacle_layer.pointcloud.topic"], timeout=45)
    return "points_relabeled" in (r.stdout or "")


def _cancel_follow(ec):
    """Zelfde stop-pad als stop_boundary_follow in extended_commands, maar
    exception-tolerant en in de juiste volgorde: EERST de kill van de
    CLI-client (mag nooit falen of blokkeren), DAARNA best-effort de
    cover_task_stop-servicecall (kan 3-6 s+ duren of timeouten — dat mag de
    pkill nooit tegenhouden)."""
    try:
        # Vaste string, geen user input -> geen command-injection risico.
        os.system("pkill -f 'ros2 action send_goal /boundary_follow' 2>/dev/null")
    except Exception:
        pass
    try:
        ec.ros2_run(["ros2", "service", "call", "/coverage_planner_server/cover_task_stop",
                     "std_srvs/srv/SetBool", "'{data: true}'"], timeout=15)
    except Exception:
        pass


def _drive_forward_retry(sess, ec):
    """~2 m vooruit rijden vóór een retry-poging na SEARCHING_START_FAILED
    (code 4): 8 s lang Twist(linear.x=0.25) op /cmd_vel, dan een nul-Twist
    om netjes te stoppen. Zelfde patroon als drive_backward in de
    calibration-drive van extended_commands.py, maar vooruit i.p.v.
    achteruit. Een stop_auto_map tijdens de rit breekt de rit direct af
    (stop_requested-check per publish-tik). Best-effort en volledig
    exception-tolerant: een rijfout hier mag de sessie nooit stil laten
    sterven — de caller herstart gewoon de goal ook als deze functie
    faalt."""
    try:
        import rclpy
        from rclpy.node import Node
        from geometry_msgs.msg import Twist
        try:
            rclpy.init()
        except RuntimeError:
            pass
        node = Node(f"auto_map_retry_drive_{os.getpid()}_"
                    f"{int(time.monotonic() * 1000) % 1000000}")
        pub = node.create_publisher(Twist, "/cmd_vel", 10)
        msg = Twist()
        msg.linear.x = 0.25
        end_at = time.monotonic() + 8.0
        try:
            while time.monotonic() < end_at and not sess.stop_requested:
                pub.publish(msg)
                time.sleep(0.05)
        finally:
            # Nul-Twist ALTIJD sturen, ook als de rijlus halverwege raist:
            # anders hangt het stoppen af van de firmware-deadman op /cmd_vel.
            try:
                stop = Twist()
                for _ in range(5):
                    pub.publish(stop)
                    time.sleep(0.05)
            except Exception:
                pass
            try:
                node.destroy_node()
            except Exception:
                pass
    except Exception as ex:
        try:
            ec.log(f"[auto_map] retry-rit vooruit mislukte: {ex}")
        except Exception:
            pass


def _wait_proc(proc):
    """Best-effort wachten op de CLI-client van de action-goal; bij timeout
    (proc reageert niet binnen 15 s) hard killen i.p.v. de sessie te laten
    hangen."""
    try:
        proc.wait(timeout=15)
    except subprocess.TimeoutExpired:
        try:
            proc.kill()
        except Exception:
            pass
    except Exception:
        pass


def _run_session(sess, ec):
    """Prepare → goal → watchdog. Draait in eigen thread.

    Wrapper om _run_session_body: die functie mag intern raisen (ros2_run
    timeout, proc.wait, of sess.status() bij een MQTT-socketbreuk) zonder dat
    de geofence-bewaking stilletjes doodgaat — hier vangen we alles af, doen
    best-effort _cancel_follow en zetten best-effort een terminale status."""
    try:
        _run_session_body(sess, ec)
    except Exception as ex:
        try:
            _ec().log(f"[auto_map] sessie crashte: {ex}")
        except Exception:
            pass
        try:
            _cancel_follow(ec)
        except Exception:
            pass
        try:
            sess.status("error", error=f"session_crash: {ex}")
        except Exception:
            pass


def _run_session_body(sess, ec):
    """Feitelijke prepare → goal → watchdog-logica (zie _run_session voor het
    crash-vangnet eromheen)."""
    sess.status("preparing")

    if not _relay_alive(ec):
        sess.status("error", error="relay_missing")
        return
    if not _set_costmap_topic(ec):
        sess.status("error", error="costmap_param_failed")
        return

    # Maart-flow stap 1+2: camera's aan + perceptie aan. Zonder rijdende
    # maaibeurt staan de camera's UIT en blijft de costmap leeg — dan komt
    # BoundaryFollow direct terug met "No valid boundary need robot!!!"
    # (live gezien op .244, 2026-07-23). Alle drie SetBool true; best-effort
    # (staan ze al aan dan zijn dit no-ops).
    # In-process SetBool-calls (zelfde idioom als set_infer_model hieronder):
    # de CLI-vorm time-outte live alle drie tegelijk onder load (2026-10-09,
    # 20.2/20.1/20.2 s) terwijl de in-process call in dezelfde sessie
    # binnen een seconde slaagde — zonder camera's is de sessie blind.
    try:
        import rclpy
        from std_srvs.srv import SetBool
        node, ctx = _preflight_node("auto_map_camstart")
        try:
            for srv in ("/camera/preposition/start_camera",
                        "/camera/tof/start_camera",
                        "/perception/do_perception"):
                try:
                    cli = node.create_client(SetBool, srv)
                    if not cli.wait_for_service(timeout_sec=10.0):
                        ec.log(f"[auto_map] {srv}: service niet gevonden (ga door)")
                        continue
                    fut = cli.call_async(SetBool.Request(data=True))
                    end_at = time.monotonic() + 10.0
                    while not fut.done() and time.monotonic() < end_at:
                        rclpy.spin_once(node, timeout_sec=0.5)
                    ec.log(f"[auto_map] {srv}: "
                           + ("ok" if fut.done() and fut.result().success
                              else ("time-out (ga door)" if not fut.done()
                                    else f"weigerde: {fut.result().message}")))
                except Exception as ex:
                    ec.log(f"[auto_map] {srv} aanzetten faalde (ga door): {ex}")
        finally:
            node.destroy_node()
            try:
                ctx.shutdown()
            except Exception:
                pass
    except Exception as ex:
        ec.log(f"[auto_map] camera-start blok faalde (ga door): {ex}")

    # Enige perceptie-instelling die wij zetten: SEG_HIGH (mode 3, maart-flow).
    # coverage_planner_server regelt semantic/detection-mode ZELF bij de goal.
    # In-process service call: de CLI-vorm time-outte live (session_crash
    # 2026-10-04, 15 s) terwijl de call zelf milliseconden werk is.
    try:
        import rclpy
        from general_msgs.srv import SetUint8
        node, ctx = _preflight_node("auto_map_infermodel")
        try:
            cli = node.create_client(SetUint8, "/perception/set_infer_model")
            if not cli.wait_for_service(timeout_sec=10.0):
                ec.log("[auto_map] set_infer_model: service niet gevonden (ga door)")
            else:
                fut = cli.call_async(SetUint8.Request(value=3))
                end_at = time.monotonic() + 10.0
                while not fut.done() and time.monotonic() < end_at:
                    rclpy.spin_once(node, timeout_sec=0.5)
                if not fut.done():
                    ec.log("[auto_map] set_infer_model: call time-out (ga door)")
                else:
                    ec.log(f"[auto_map] set_infer_model ok: {fut.result()}")
        finally:
            node.destroy_node()
            try:
                ctx.shutdown()
            except Exception:
                pass
    except Exception as ex:
        ec.log(f"[auto_map] set_infer_model faalde (ga door): {ex}")

    # Maart-flow stap 3: wachten tot er echt labeled-data stroomt (camera's
    # hebben spin-up nodig). Drie pogingen van ~15 s elk; geen data → abort.
    if not _wait_for_perception_data(ec):
        sess.status("error", error="no_perception_data")
        return

    # GPS-volger voor de geofence: één achtergrond-subscription op NavSatFix.
    _start_gps_watch(sess)
    deadline = time.monotonic() + 30
    while sess.start_gps is None and time.monotonic() < deadline:
        time.sleep(0.5)
    if sess.start_gps is None:
        sess.status("error", error="no_gps_fix")
        return

    # Optioneel: eerst deterministisch achteruit van de dock (zelfde
    # _depart_pile als edge-cut: magnet-lock vrijgeven, ~1 m reverse).
    # Alleen op verzoek (departFromDock) — midden op het gazon achteruit
    # rijden kan een obstakel raken dat de costmap niet kent.
    if sess.depart_from_dock:
        ec.log("[auto_map] departFromDock: achteruit van de dock (~1 m)")
        try:
            ec._depart_pile()
        except Exception as ex:
            ec.log(f"[auto_map] depart_pile faalde: {ex} — goal toch proberen")

    # BoundaryFollow-goal via CLI, output naar ACTION_LOG voor result-parse.
    # Eén automatische retry bij SEARCHING_START_FAILED (code 4, zie
    # should_retry()): de maaier rijdt ~2 m vooruit en probeert de goal
    # nogmaals. Alle abort-paden (stop/timeout/gps_stale/geofence) blijven
    # binnen ELKE poging actief.
    for attempt in (1, 2):
        try:
            os.unlink(ACTION_LOG)
        except OSError:
            pass
        with open(ACTION_LOG, "w") as logf:
            proc = subprocess.Popen(
                ["ros2", "action", "send_goal", "/boundary_follow",
                 "coverage_planner/action/BoundaryFollow", boundary_goal_yaml()],
                stdout=logf, stderr=subprocess.STDOUT)
        if attempt == 1:
            sess.status("searching_boundary")
        # attempt 2: de "searching_boundary"-status is al gepubliceerd door
        # de retry-tak hieronder (met retry=2), dus hier niet nogmaals.

        following_reported = False
        result_code = None
        while True:
            time.sleep(2.0)
            elapsed = time.monotonic() - sess.started
            if sess.stop_requested:
                _cancel_follow(ec)
                _wait_proc(proc)
                sess.status("aborted", error="user_stop")
                return
            if elapsed > sess.timeout_s:
                _cancel_follow(ec)
                _wait_proc(proc)
                sess.status("aborted", error="timeout")
                return
            if sess.start_gps is not None and (
                    sess.last_fix_mono is None or
                    time.monotonic() - sess.last_fix_mono > GPS_STALE_S):
                # Geen verse GPS meer (topic weg / GPS-thread dood) -> geofence
                # werkt niet meer, dus stoppen i.p.v. blind doorrijden.
                _cancel_follow(ec)
                _wait_proc(proc)
                sess.status("aborted", error="gps_stale")
                return
            if sess.last_gps and sess.start_gps:
                d = haversine_m(sess.start_gps[0], sess.start_gps[1],
                                sess.last_gps[0], sess.last_gps[1])
                if d > sess.radius_m:
                    _cancel_follow(ec)
                    _wait_proc(proc)
                    sess.status("aborted", error="geofence", dist_m=round(d, 1))
                    return
                if not following_reported and elapsed > 10:
                    following_reported = True
                    sess.status("following", dist_m=round(d, 1))
            if proc.poll() is not None:
                try:
                    with open(ACTION_LOG) as f:
                        text = f.read()
                except OSError:
                    text = ""
                status, code = parse_action_result(text)
                if code is None:
                    sess.status("error", error=f"action_exit_{proc.returncode}_no_result")
                    return
                result_code = code
                break

        if should_retry(result_code, attempt):
            # Stop-verzoek wint altijd van een retry: niet meer rijden en
            # geen tweede goal starten.
            if sess.stop_requested:
                sess.status("aborted", error="user_stop")
                return
            _drive_forward_retry(sess, ec)
            if sess.stop_requested:
                sess.status("aborted", error="user_stop")
                return
            sess.status("searching_boundary", retry=attempt + 1)
            continue

        sess.status("result", code=result_code,
                    name=RESULT_NAMES.get(result_code, f"code_{result_code}"))
        return


def _start_gps_watch(sess):
    """NavSatFix-subscriber in eigen thread (patroon: calibration-drive in
    extended_commands). Vult sess.start_gps (eerste fix) en sess.last_gps."""
    def _spin():
        try:
            import rclpy
            from rclpy.node import Node
            from sensor_msgs.msg import NavSatFix
            try:
                rclpy.init()
            except RuntimeError:
                pass
            # Unieke naam per sessie: voorkomt botsing met een nog uitdovende
            # node van een vorige (net gestopte) sessie-thread.
            node = Node(f"auto_map_gps_watch_{os.getpid()}_"
                        f"{int(time.monotonic() * 1000) % 1000000}")

            def on_fix(msg):
                if msg.latitude == 0.0 and msg.longitude == 0.0:
                    return
                if sess.start_gps is None:
                    sess.start_gps = (msg.latitude, msg.longitude)
                sess.last_gps = (msg.latitude, msg.longitude)
                sess.last_fix_mono = time.monotonic()

            node.create_subscription(NavSatFix, "/gps_raw", on_fix, 5)
            while not sess.stop_requested and sess.last_status.get("phase") not in (
                    "result", "error", "aborted"):
                rclpy.spin_once(node, timeout_sec=1.0)
            node.destroy_node()
        except Exception as ex:
            _ec().log(f"[auto_map] gps watch dood: {ex}")
    threading.Thread(target=_spin, daemon=True).start()


def main():
    ec = _ec()
    sn, addr, port = ec.read_config()
    sub_topic = f"novabot/extended/{sn}"
    resp_topic = f"novabot/extended_response/{sn}"
    ec.log(f"[auto_map] SN={sn} MQTT={addr}:{port} sub={sub_topic}")

    state = {"session": None, "client": None}

    def publish_status(st):
        c = state["client"]
        if c:
            c.publish(resp_topic, json.dumps({"auto_map_status": st}))

    def respond(key, payload):
        c = state["client"]
        if c:
            c.publish(resp_topic, json.dumps({key: payload}))

    def on_message(topic, payload):
        try:
            cmd = json.loads(payload)
        except (ValueError, TypeError):
            return
        if "start_auto_map_test" in cmd:
            params = cmd.get("start_auto_map_test") or {}
            sess = state["session"]
            if sess and sess.last_status.get("phase") in (
                    "preparing", "searching_boundary", "following"):
                respond("start_auto_map_test_respond",
                        {"result": 1, "error": "already_running"})
                return
            try:
                radius = float(params.get("radiusM", DEFAULT_RADIUS_M))
                timeout = int(params.get("timeoutS", DEFAULT_TIMEOUT_S))
                depart = bool(params.get("departFromDock", False))
            except (TypeError, ValueError) as ex:
                respond("start_auto_map_test_respond",
                        {"result": 1, "error": f"param type error: {ex}"})
                return
            radius = max(5.0, min(200.0, radius))
            timeout = max(60, min(3600, timeout))
            sess = AutoMapSession(publish_status, radius, timeout,
                                  depart_from_dock=depart)
            state["session"] = sess
            threading.Thread(target=_run_session, args=(sess, ec), daemon=True).start()
            respond("start_auto_map_test_respond", {"result": 0})
        elif "stop_auto_map" in cmd:
            sess = state["session"]
            if sess:
                sess.stop_requested = True
            respond("stop_auto_map_respond", {"result": 0})
        elif "get_auto_map_status" in cmd:
            sess = state["session"]
            respond("get_auto_map_status_respond",
                    sess.last_status if sess else {"phase": "idle"})
        # Alle andere commando's zijn voor extended_commands.py — negeren.

    while True:
        try:
            client = ec.MiniMQTT(addr, port, f"auto_map_{sn}", on_message)
            client.connect()
            client.subscribe(sub_topic)
            state["client"] = client
            ec.log("[auto_map] verbonden, wacht op commando's")
            client.loop_forever()
        except Exception as ex:
            ec.log(f"[auto_map] MQTT-verbinding weg ({ex}), retry in 10 s")
            time.sleep(10)


if __name__ == "__main__":
    sys.exit(main())
