"""Run on a laptop: python3 research/test_marker_measurement.py (no ROS required)."""
import importlib.util
import itertools
import json
import math
import os
import tempfile
import types
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("marker_commands", os.path.join(os.path.dirname(__file__), "extended_commands.py"))
commands = importlib.util.module_from_spec(spec)
spec.loader.exec_module(commands)


def pose(transform):
    p, q = transform
    return {"position": dict(zip("xyz", p)), "orientation": dict(zip("xyzw", q))}


def header(t, frame):
    return {"stamp": {"sec": int(t), "nanosec": round((t % 1) * 1e9)}, "frame_id": frame}


def capture():
    # Deliberately nonzero roll, pitch and lever arm: yaw-only math must fail this test.
    q = commands._marker_qmul((0, 0, math.sin(.6), math.cos(.6)),
                             commands._marker_qmul((math.sin(.1), 0, 0, math.cos(.1)),
                                                   (0, math.sin(.15), 0, math.cos(.15))))
    map_base = ((3, -2, .1), q)
    base_gps = ((.186, 0, .15), (0, 0, 0, 1))
    base_tag = ((1.1, .03, .15), (0, 0, math.sin(.2), math.cos(.2)))
    map_gps = commands._marker_compose(map_base, base_gps)
    tag_base = commands._marker_inverse(base_tag)
    out = {"/tf_static": {"gps_link": {
        "header": header(0, "base_link"), "child_frame_id": "gps_link",
        "transform": {"translation": pose(base_gps)["position"], "rotation": pose(base_gps)["orientation"]}}}}
    for i in range(35):
        t = 99.8 + i * .2
        def add(topic, data):
            out.setdefault(topic, []).append({"received": t + .01, "data": data})
        add("/aruco/pose", {"header": header(t, "aruco_tag"), "pose": pose(tag_base)})
        add("/robot_combination_localization/odom", {
            "header": header(t, "map"), "child_frame_id": "gps_link",
            "pose": {"pose": pose(map_gps)},
            "twist": {"twist": {"linear": dict.fromkeys("xyz", 0), "angular": dict.fromkeys("xyz", 0)}}})
        add("/robot_decision/map_position", pose(map_base))
        add("/bestpos_parsed_data", {"header": header(t, "gps_link"), "qual": 4, "diff_age": 1.4,
                                      "latitude": 52.14101542986, "longitude": 6.23132876004})
        add("/robot_combination_localization/combination_status", {"status": 200})
        if i % 10 == 0:
            add("/chassis_incident", {"error_lora": False, "warning_lora_rtk_data_overtime": False})
        add("/robot_decision/robot_status", {"merged_work_status": 0, "error_status": 0})
    return out, commands._marker_compose(map_base, base_tag)


def origin():
    # Independent EPSG:32632/PROJ fixture. Deliberately nonzero effective offset.
    samples, _ = capture()
    gps = samples["/robot_combination_localization/odom"][0]["data"]["pose"]["pose"]["position"]
    return {"x": 310545.1690844319 - gps["x"] + .07,
            "y": 5780337.788794573 - gps["y"] - .02, "z": 0, "utm_zone": 32}


class MarkerMeasurementTest(unittest.TestCase):
    def test_full_se3_vehicle_offset_and_inverse(self):
        samples, expected = capture()
        result = commands._marker_measurement_result(samples, 100, 106, origin())
        for key, value in zip("xyz", expected[0]):
            self.assertAlmostEqual(result["marker"][key], value, places=12)
        self.assertAlmostEqual(result["marker"]["yaw"], commands._marker_yaw(expected[1]), places=12)
        self.assertAlmostEqual(result["base"]["x"], 3)
        self.assertGreaterEqual(result["unique_stamps"], 20)
        runtime = result["runtime_frame"]
        self.assertAlmostEqual(runtime["x"], .07, delta=.002)
        self.assertAlmostEqual(runtime["y"], -.02, delta=.002)
        self.assertEqual(runtime["max_pair_dt_s"], 0)

    def test_extracted_projection_matches_independent_proj_fixtures(self):
        # Generated with PROJ through pyproj, not the function under test.
        for lat, lng, zone, x, y in (
            (52.14101542986, 6.23132876004, 32, 310545.1690844319, 5780337.788794573),
            (0, 3, 31, 500000, 0),
            (-33.865143, 151.2099, 56, 334417.07452301355, 6251354.86000826),
            (83, 5.99, 31, 540663.8645492753, 9217512.385367315),
        ):
            actual_zone, actual_x, actual_y = commands._gps_to_utm(lat, lng)
            self.assertEqual(actual_zone, zone)
            self.assertAlmostEqual(actual_x, x, delta=.002)
            self.assertAlmostEqual(actual_y, y, delta=.002)

    def test_runtime_requires_exact_unique_fresh_pairs_and_valid_gps(self):
        for kind in ("healthy", "duplicate", "one_nanosecond", "stale", "wrong_frame", "wrong_zone", "nan", "missing_coordinate"):
            with self.subTest(kind=kind):
                samples, _ = capture()
                for row in samples["/bestpos_parsed_data"]:
                    if kind == "duplicate": row["data"]["header"] = header(101, "gps_link")
                    if kind == "one_nanosecond": row["data"]["header"]["stamp"]["nanosec"] += 1
                    if kind == "stale": row["received"] += 2
                    if kind == "wrong_frame": row["data"]["header"]["frame_id"] = "base_link"
                    if kind == "wrong_zone": row["data"]["longitude"] = 12.1
                    if kind == "nan": row["data"]["latitude"] = float("nan")
                    if kind == "missing_coordinate": del row["data"]["latitude"]
                if kind == "healthy":
                    result = commands._runtime_frame_result(samples, 100, 106, origin())
                    self.assertGreaterEqual(result["sample_count"], 20)
                    self.assertEqual(result["sample_count"], result["unique_stamps"])
                    self.assertAlmostEqual(result["x"], .07, delta=.002)
                else:
                    with self.assertRaises((ValueError, KeyError)):
                        commands._runtime_frame_result(samples, 100, 106, origin())

    def test_runtime_rejects_offset_jump_and_motion_even_with_zero_twist(self):
        for kind in ("offset_jump", "translation", "heading"):
            with self.subTest(kind=kind):
                samples, _ = capture()
                for i in range(17, 35):
                    gps = samples["/bestpos_parsed_data"][i]["data"]
                    odom = samples["/robot_combination_localization/odom"][i]["data"]
                    if kind == "offset_jump":
                        gps["latitude"] += .000001
                    elif kind == "translation":
                        # GPS and navigation move together, keeping the effective offset unchanged.
                        old = commands._gps_to_utm(gps["latitude"], gps["longitude"])
                        gps["latitude"] += .000002
                        new = commands._gps_to_utm(gps["latitude"], gps["longitude"])
                        odom["pose"]["pose"]["position"]["x"] += new[1] - old[1]
                        odom["pose"]["pose"]["position"]["y"] += new[2] - old[2]
                    else:
                        p, q = commands._marker_pose(odom["pose"]["pose"])
                        odom["pose"]["pose"] = pose((p, commands._marker_qmul((0, 0, math.sin(.1), math.cos(.1)), q)))
                    base = commands._marker_compose(commands._marker_pose(odom["pose"]["pose"]),
                                                    commands._marker_inverse(commands._measurement_base_gps(samples)))
                    samples["/robot_decision/map_position"][i]["data"] = pose(base)
                with self.assertRaisesRegex(ValueError, "unstable"):
                    commands._runtime_frame_result(samples, 100, 106, origin())

    def test_runtime_checks_late_paired_receiver_health(self):
        for bad in ({"qual": 0}, {"diff_age": 10}):
            with self.subTest(bad=bad):
                samples, _ = capture()
                row = samples["/bestpos_parsed_data"][-1]
                # Exact source pair, received within 1.5s but after last+.5.
                row["received"] = commands._marker_stamp(row["data"]) + .6
                row["data"].update(bad)
                with self.assertRaisesRegex(ValueError, "every paired receiver observation"):
                    commands._runtime_frame_result(samples, 100, 107, origin())

    def test_runtime_error113_is_not_allowed_without_marker(self):
        samples, _ = capture()
        for row in samples["/robot_decision/robot_status"]:
            row["data"]["error_status"] = 113
        with self.assertRaisesRegex(ValueError, "idle robot"):
            commands._runtime_frame_result(samples, 100, 106, origin())
        self.assertIn("runtime_frame", commands._marker_measurement_result(samples, 100, 106, origin()))

    def test_runtime_command_frame_change_protocol_and_map_lock(self):
        for change in ("none", "origin", "fingerprint"):
            with self.subTest(change=change):
                before, after = origin(), origin()
                if change == "origin": after["x"] += .01
                replies = []
                with patch.object(commands, "_coverage_is_active", return_value=False), \
                     patch.object(commands, "_runtime_origin", side_effect=[before, after]), \
                     patch.object(commands, "_marker_frame_fingerprint", side_effect=["a", "b" if change == "fingerprint" else "a"]), \
                     patch.object(commands, "_capture_runtime_frame", return_value={"x": .07, "y": -.02}):
                    commands.run_extended_command("measure_runtime_frame", commands.handle_measure_runtime_frame,
                                                  {"operation_id": "runtime-op"}, lambda key, value: replies.append(value))
                self.assertEqual(replies[0]["operation_id"], "runtime-op")
                self.assertEqual(replies[0]["result"], 0 if change == "none" else 1)
                if change == "none":
                    self.assertEqual(replies[0]["protocol"], "runtime-map-frame-v1")
                    self.assertIn("runtime_frame", replies[0])
        self.assertIn("measure_runtime_frame", commands._MAP_OPERATION_COMMANDS)

    def test_stale_repeated_and_unpaired_images_fail(self):
        for kind in ("old", "repeated", "unpaired"):
            with self.subTest(kind=kind):
                samples, _ = capture()
                for row in samples["/aruco/pose"]:
                    if kind == "old":
                        row["received"] += 2
                    elif kind == "repeated":
                        row["data"]["header"] = header(100, "aruco_tag")
                if kind == "unpaired":
                    samples["/robot_combination_localization/odom"] = samples["/robot_combination_localization/odom"][:1]
                with self.assertRaises(ValueError):
                    commands._marker_measurement_result(samples, 100, 106, origin())

    def test_one_bad_health_sample_motion_or_wrong_map_fails(self):
        for kind in ("float", "jump", "active", "moving", "wrong_map", "stale_rtk"):
            with self.subTest(kind=kind):
                samples, _ = capture()
                if kind == "float":
                    samples["/bestpos_parsed_data"][15]["data"]["qual"] = 5
                elif kind == "jump":
                    samples["/robot_combination_localization/combination_status"][15]["data"]["status"] = 6
                elif kind == "active":
                    samples["/robot_decision/robot_status"][15]["data"]["merged_work_status"] = 2
                elif kind == "moving":
                    samples["/robot_combination_localization/odom"][15]["data"]["twist"]["twist"]["linear"]["x"] = .1
                elif kind == "wrong_map":
                    samples["/robot_decision/map_position"][15]["data"]["position"]["x"] += .2
                else:
                    for row in samples["/bestpos_parsed_data"]:
                        row["data"]["header"] = header(99.8, "gps_link")
                with self.assertRaises(ValueError):
                    commands._marker_measurement_result(samples, 100, 106, origin())

    def test_lora_status8_requires_current_healthy_flags_and_fresh_corrections(self):
        for kind in ("healthy", "error", "warning", "missing", "stale", "unknown_status", "old_corrections", "nan_age"):
            with self.subTest(kind=kind):
                samples, _ = capture()
                for row in samples["/robot_decision/robot_status"]:
                    row["data"]["error_status"] = 9 if kind == "unknown_status" else 8
                flags = samples["/chassis_incident"]
                if kind == "error": flags[1]["data"]["error_lora"] = True
                if kind == "warning": flags[1]["data"]["warning_lora_rtk_data_overtime"] = True
                if kind == "missing": del flags[1]["data"]["error_lora"]
                if kind == "stale": samples["/chassis_incident"] = flags[:1]
                if kind in ("old_corrections", "nan_age"):
                    samples["/bestpos_parsed_data"][15]["data"]["diff_age"] = 3.1 if kind == "old_corrections" else float("nan")
                if kind == "healthy":
                    self.assertEqual(commands._marker_measurement_result(samples, 100, 106, origin())["result"], 0)
                else:
                    with self.assertRaises(ValueError):
                        commands._marker_measurement_result(samples, 100, 106, origin())

    def test_completed_dock_failure_still_requires_new_marker_success_and_idle(self):
        for kind in ("healthy", "no_marker", "cached_marker", "docking"):
            with self.subTest(kind=kind):
                samples, _ = capture()
                for row in samples["/robot_decision/robot_status"]:
                    row["data"]["error_status"] = 113
                    if kind == "docking": row["data"]["merged_work_status"] = 2
                if kind == "no_marker": samples["/aruco/pose"] = []
                if kind == "cached_marker":
                    for row in samples["/aruco/pose"]:
                        row["data"]["header"] = header(99, "aruco_tag")
                if kind == "healthy":
                    self.assertEqual(commands._marker_measurement_result(samples, 100, 106, origin())["result"], 0)
                else:
                    with self.assertRaises(ValueError):
                        commands._marker_measurement_result(samples, 100, 106, origin())

    def test_fingerprint_checks_native_dock_and_dispatcher_checks_frame(self):
        with tempfile.TemporaryDirectory() as directory:
            home = os.path.join(directory, "maps", "home0")
            os.makedirs(os.path.join(home, "csv_file"))
            origin = os.path.join(directory, "pos.json")
            dock_yaml = os.path.join(directory, "charging.yaml")
            with open(origin, "w") as fh:
                json.dump({"utm_origin": {"x": 310000, "y": 5780000, "z": 0, "utm_zone": 32}}, fh)
            with open(os.path.join(home, "csv_file", "map_info.json"), "w") as fh:
                json.dump({"charging_pose": {"x": .03, "y": .73, "orientation": -1.5}}, fh)
            with open(dock_yaml, "w") as fh:
                fh.write("charging_pose: [0.03, 0.73, -1.5]\n")
            with patch.object(commands, "MAPS_HOME", home), patch.object(commands, "MAP_POS_FILE", origin), patch.object(commands, "MAP_CHARGING_STATION_FILE", dock_yaml):
                before = commands._marker_frame_fingerprint()
                self.assertEqual(len(before), 64)
                with open(dock_yaml, "w") as fh:
                    fh.write("charging_pose: [1.03, 0.73, -1.5]\n")
                with self.assertRaises(ValueError):
                    commands._marker_frame_fingerprint()
        replies = []
        with patch.object(commands, "_coverage_is_active", return_value=False), patch.object(commands, "_marker_frame_fingerprint", side_effect=["a", "b"]), patch.object(commands, "_capture_dock_marker", return_value={"result": 0}), patch.object(commands, "_runtime_origin", side_effect=origin):
            commands.run_extended_command("measure_dock_marker", commands.handle_measure_dock_marker,
                                          {"operation_id": "test-op"}, lambda key, value: replies.append(value))
        self.assertEqual(replies[0]["result"], 1)
        self.assertEqual(replies[0]["operation_id"], "test-op")
        self.assertIn("measure_dock_marker", commands._MAP_OPERATION_COMMANDS)

    def test_detector_is_disabled_after_capture_failure_without_motion_publishers(self):
        callbacks, toggles, cleanup, clients = {}, [], [], []
        camera_events = []
        reject_disable = False
        hold_until = 100
        drain_bad_status = False
        spins = 0

        def camera_use(action, expires_at=None):
            camera_events.append((action, list(toggles)))
            return hold_until

        class Context:
            def shutdown(self):
                cleanup.append("context")

        class Client:
            def wait_for_service(self, **kwargs):
                return True

            def call_async(self, request):
                toggles.append(request.data)
                success = request.data or not reject_disable
                return types.SimpleNamespace(done=lambda: True, result=lambda: types.SimpleNamespace(success=success))

        class Node:
            def create_subscription(self, kind, topic, callback, qos):
                callbacks[topic] = callback

            def create_client(self, kind, topic):
                clients.append(topic)
                self_topic = "/enable_aruco_localization"
                if topic != self_topic:
                    raise AssertionError("unexpected service: " + topic)
                return Client()

            def destroy_node(self):
                cleanup.append("node")

        class Executor:
            def __init__(self, **kwargs):
                nonlocal spins
                spins = 0

            def add_node(self, node):
                pass

            def remove_node(self, node):
                pass

            def spin_until_future_complete(self, future, **kwargs):
                pass

            def spin_once(self, **kwargs):
                nonlocal spins
                spins += 1
                callbacks["/bestpos_parsed_data"]({"qual": 4, "diff_age": 1.4})
                callbacks["/robot_combination_localization/combination_status"]({"status": 6 if drain_bad_status and spins == 4 else 200})
                callbacks["/chassis_incident"]({"error_lora": False, "warning_lora_rtk_data_overtime": False})
                callbacks["/robot_decision/robot_status"]({"merged_work_status": 0, "error_status": 0})

            def shutdown(self):
                cleanup.append("executor")

        ns = types.SimpleNamespace
        modules = {"rclpy": ns(init=lambda **kw: None, create_node=lambda *a, **kw: Node()),
                   "rclpy.context": ns(Context=Context),
                   "rclpy.executors": ns(SingleThreadedExecutor=Executor),
                   "rclpy.qos": ns(QoSProfile=lambda **kw: None, ReliabilityPolicy=ns(BEST_EFFORT=1, RELIABLE=2), DurabilityPolicy=ns(TRANSIENT_LOCAL=1)),
                   "rosidl_runtime_py.utilities": ns(get_message=lambda kind: object),
                   "rosidl_runtime_py.convert": ns(message_to_ordereddict=lambda value: value),
                   "std_srvs.srv": ns(SetBool=ns(Request=lambda: ns(data=False)))}
        # Runtime continuity uses only passive subscriptions and its own ROS context.
        with patch.dict("sys.modules", modules), patch.object(commands.time, "time", side_effect=itertools.count(100, .01)), patch.object(commands.time, "monotonic", side_effect=iter(range(100))), patch.object(commands, "_marker_camera_use", side_effect=AssertionError("runtime must not use camera")):
            with self.assertRaisesRegex(ValueError, "static transform missing"):
                commands._capture_runtime_frame(origin())
        self.assertEqual(clients, [])
        self.assertEqual(toggles, [])
        self.assertNotIn("/aruco/pose", callbacks)
        self.assertEqual(cleanup, ["node", "executor", "context"])
        cleanup.clear()
        # One bad LOC update arrives only during the final drain. A completed
        # source window cannot override current health at the return boundary.
        drain_bad_status = True
        with patch.dict("sys.modules", modules), patch.object(commands.time, "time", side_effect=itertools.count(100, .01)), patch.object(commands.time, "monotonic", side_effect=[0, 1, 2, 3, 4, 15, 16, 16.1, 16.4]), patch.object(commands, "_runtime_frame_result", return_value={"x": 0, "y": 0}):
            with self.assertRaisesRegex(ValueError, "measurement drain"):
                commands._capture_runtime_frame(origin())
        self.assertEqual(clients, [])
        self.assertEqual(cleanup, ["node", "executor", "context"])
        drain_bad_status = False
        cleanup.clear()
        with patch.dict("sys.modules", modules), patch.object(commands.time, "time", side_effect=itertools.count(100, .01)), patch.object(commands.time, "monotonic", side_effect=iter(range(100))), patch.object(commands, "_marker_camera_use", side_effect=camera_use):
            with self.assertRaisesRegex(ValueError, "static transform missing"):
                commands._capture_dock_marker(origin())
        self.assertEqual(toggles, [True, False])
        self.assertEqual(camera_events, [("begin", []), ("end", [True, False])])
        self.assertEqual(cleanup, ["node", "executor", "context"])

        # A failed disable must keep the server-side protection until its deadline.
        toggles.clear()
        cleanup.clear()
        camera_events.clear()
        reject_disable = True
        with patch.dict("sys.modules", modules), patch.object(commands.time, "time", side_effect=itertools.count(100, .01)), patch.object(commands.time, "monotonic", side_effect=iter(range(100))), patch.object(commands, "_marker_camera_use", side_effect=camera_use):
            with self.assertRaisesRegex(ValueError, "disable.*not acknowledged"):
                commands._capture_dock_marker(origin())
        self.assertEqual(toggles, [True, False])
        self.assertEqual(camera_events, [("begin", [])])
        self.assertEqual(cleanup, ["node", "executor", "context"])

        # Even healthy telemetry cannot yield a result after camera protection expires.
        reject_disable = False
        hold_until = 12
        toggles.clear()
        camera_events.clear()
        with patch.dict("sys.modules", modules), patch.object(commands.time, "time", side_effect=itertools.count(100, .01)), patch.object(commands.time, "monotonic", side_effect=iter(range(100))), patch.object(commands, "_marker_camera_use", side_effect=camera_use):
            with self.assertRaisesRegex(ValueError, "protection expired"):
                commands._capture_dock_marker(origin())
        self.assertEqual(toggles, [True, False])
        self.assertEqual(camera_events, [("begin", []), ("end", [True, False])])

        # If the camera server is unavailable, do not even enable the detector.
        toggles.clear()
        with patch.dict("sys.modules", modules), patch.object(commands.time, "time", side_effect=itertools.count(100, .01)), patch.object(commands.time, "monotonic", side_effect=iter(range(100))), patch.object(commands, "_marker_camera_use", side_effect=ValueError("camera unavailable")):
            with self.assertRaisesRegex(ValueError, "camera unavailable"):
                commands._capture_dock_marker(origin())
        self.assertEqual(toggles, [])

    def test_camera_use_requires_ack_and_current_bounded_protection(self):
        from urllib.error import URLError
        from unittest.mock import MagicMock
        for payload in ({"success": False}, {"success": True, "expires_at": 100},
                        {"success": True, "expires_at": 131}, {"success": True, "expires_at": float("nan")}):
            response = MagicMock()
            response.__enter__.return_value.read.return_value = json.dumps(payload).encode()
            with patch("urllib.request.urlopen", return_value=response), patch.object(commands.time, "monotonic", return_value=100):
                with self.assertRaises(ValueError):
                    commands._marker_camera_use("begin")
        response.__enter__.return_value.read.return_value = b'{"success": true, "expires_at": 130}'
        with patch("urllib.request.urlopen", return_value=response) as request, patch.object(commands.time, "monotonic", return_value=100):
            self.assertEqual(commands._marker_camera_use("begin"), 130)
            self.assertEqual(request.call_args.kwargs["timeout"], 6)
            self.assertEqual(request.call_args.args[0].full_url, "http://127.0.0.1:8000/marker-camera-use")
        with patch("urllib.request.urlopen", side_effect=URLError("timeout")):
            with self.assertRaisesRegex(ValueError, "unavailable"):
                commands._marker_camera_use("begin")


if __name__ == "__main__":
    unittest.main()
