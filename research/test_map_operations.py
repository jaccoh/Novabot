"""Local-only regression checks for MQTT map mutations, using temporary maps."""
import base64
import json
from pathlib import Path
import sys
import tempfile
import threading
import unittest
from types import SimpleNamespace
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import extended_commands as ext


class MapOperationTest(unittest.TestCase):
    def test_receiver_samples_carry_original_stamp_and_valid_coordinates(self):
        msg = SimpleNamespace(header=SimpleNamespace(stamp=SimpleNamespace(sec=123, nanosec=42)),
                              latitude=52.1, longitude=6.2, qual=4, svs=30)
        payload = ext.rtk_sample_payload(msg)
        self.assertEqual(payload['rtk_sample_id'], '123:42')
        self.assertEqual(payload['rtk_latitude'], 52.1)
        msg.latitude = float('nan')
        self.assertIsNone(ext.rtk_sample_payload(msg))
        msg.latitude = 52.1
        msg.header.stamp = SimpleNamespace(sec=0, nanosec=0)
        self.assertIsNone(ext.rtk_sample_payload(msg))

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.home = self.root / "home0"
        for sub in ("csv_file", "x3_csv_file"):
            (self.home / sub).mkdir(parents=True)
            (self.home / sub / "map0_work.csv").write_text("0,0\n1,0\n1,1\n")
        self.pos = self.root / "pos.json"
        self.pos.write_text('{"time_stamp":12,"utm_origin":{"x":1,"y":2}}')
        self.charger = self.root / "charging_station.yaml"
        self.charger.write_text("x: 1\ny: 2\n")
        for key, value in (("MAPS_HOME", str(self.home)), ("MAP_POS_FILE", str(self.pos)),
                           ("MAP_CHARGING_STATION_FILE", str(self.charger))):
            p = patch.object(ext, key, value)
            p.start()
            self.addCleanup(p.stop)
        self.responses = []

    def respond(self, name, body):
        self.responses.append((name, body))

    def payload(self):
        return {
            "csv_files": {"map1_work.csv": "2,2\n3,2\n3,3\n"},
            "x3_csv_files": {"map1_work.csv": "2,2\n3,2\n3,3\n", "map1tocharge_unicom.csv": "1,2\n2,2\n"},
            "map_files_text": {"map.yaml": "image: map.pgm\n", "map1.yaml": "image: map1.pgm\n"},
            "map_files_b64": {"map.pgm": base64.b64encode(b"P5\n2 2\n255\n" + bytes([254] * 4)).decode()},
            "charging_station_yaml": "x: 1\ny: 2\n", "restart_mapping": False, "prune_connectors": True,
        }

    def test_bad_payload_and_failed_backup_do_not_delete_existing_files(self):
        old = ext._snapshot_manifest(ext._map_snapshot_files("home0"))
        payload = self.payload()
        payload["map_files_b64"]["map.pgm"] = "this is not base64!"
        ext.handle_write_map_files(payload, self.respond)
        self.assertEqual(self.responses[-1][1]["result"], 1)
        self.assertEqual(ext._snapshot_manifest(ext._map_snapshot_files("home0")), old)
        with patch("shutil.copytree", side_effect=OSError("disk full")):
            ext.handle_write_map_files(self.payload(), self.respond)
        self.assertEqual(self.responses[-1][1]["result"], 1)
        self.assertEqual(ext._snapshot_manifest(ext._map_snapshot_files("home0")), old)

    def test_write_preserves_separate_x3_and_snapshot_hashes_all_returned_bytes(self):
        (self.home / "map9.yaml").write_text("stale")
        before_pos = self.pos.read_bytes()
        payload = self.payload()
        ext.handle_write_map_files(payload, self.respond)
        self.assertEqual(self.responses[-1][1]["result"], 0)
        self.assertFalse((self.home / "map9.yaml").exists())
        self.assertEqual(self.pos.read_bytes(), before_pos)
        ext.handle_read_map_files({}, self.respond)
        snapshot = self.responses[-1][1]
        self.assertTrue(snapshot["snapshot_consistent"])
        self.assertEqual(snapshot["csv_files"], payload["csv_files"])
        self.assertEqual(snapshot["x3_csv_files"], payload["x3_csv_files"])
        self.assertEqual(snapshot["snapshot_manifest"], ext._snapshot_manifest(ext._map_snapshot_files("home0")))
        original = ext._map_snapshot_files("home0")
        changed = dict(original, **{"csv_file/map1_work.csv": b"5,5\n6,6\n"})
        with patch.object(ext, "_map_snapshot_files", side_effect=[original, changed]):
            ext.handle_read_map_files({}, self.respond)
        self.assertFalse(self.responses[-1][1]["snapshot_consistent"])

    def test_dispatch_serializes_read_and_write_and_echoes_correct_request_id(self):
        first_started = threading.Event()
        second_started = threading.Event()
        release = threading.Event()

        def first(params, reply):
            first_started.set()
            release.wait(2)
            reply("write_map_files_respond", {"result": 0})

        def second(params, reply):
            second_started.set()
            reply("read_map_files_respond", {"result": 0})

        with patch.object(ext, "_coverage_is_active", return_value=False):
            threads = [threading.Thread(target=ext.run_extended_command, args=("write_map_files", first, {"operation_id": "first"}, self.respond)),
                       threading.Thread(target=ext.run_extended_command, args=("read_map_files", second, {"operation_id": "second"}, self.respond))]
            threads[0].start()
            self.assertTrue(first_started.wait(1))
            threads[1].start()
            self.assertFalse(second_started.wait(0.03))
            release.set()
            for thread in threads:
                thread.join(2)
                self.assertFalse(thread.is_alive())
        self.assertEqual([body["operation_id"] for _, body in self.responses], ["first", "second"])

    def test_reanchor_remeasures_before_writing_and_rejects_old_payloads(self):
        origin = {"x": 310532.558502842, "y": 5780319.396453575, "z": 0, "utm_zone": 32}
        self.pos.write_text(json.dumps({"time_stamp": 12, "utm_origin": origin}))
        anchor = {"x": .03, "y": .73, "orientation": -1.518}
        (self.home / "csv_file" / "map_info.json").write_text(json.dumps({"charging_pose": anchor}))
        self.charger.write_text("charging_pose: [0.03, 0.73, -1.518]")
        runtime = {"x": 0, "y": 0, "base": {"x": -.0437, "y": .8539, "yaw": -1.50},
                   "docked": True, "northern": True, "capture_finished": 100}
        expected = ext._reanchor_origin_from_observation(origin, runtime, anchor)
        params = {"protocol": "base-link-reanchor-v1", "frame_fingerprint": ext._marker_frame_fingerprint(),
                  "anchor_x": .03, "anchor_y": .73, "utm_origin": expected}
        original = self.pos.read_bytes()
        for kind in ("old", "anchor", "frame", "offset", "moved", "expired", "undocked", "nan", "frame_during_capture"):
            with self.subTest(kind=kind):
                p, measurement = dict(params), dict(runtime)
                if kind == "old": p = {"lat": 52, "lng": 6, "anchor_x": .03, "anchor_y": .73}
                if kind == "anchor": p["anchor_x"] = 1
                if kind == "frame": p["frame_fingerprint"] = "wrong"
                if kind == "offset": measurement["x"] = .59
                if kind == "moved": measurement["base"] = dict(runtime["base"], x=.2)
                if kind == "expired": measurement["capture_finished"] = 90
                if kind == "undocked": measurement["docked"] = False
                if kind == "nan": p["utm_origin"] = dict(expected, x=float("nan"))
                with patch.object(ext, "_capture_runtime_frame", return_value=measurement), \
                     patch.object(ext.time, "time", return_value=101), patch.object(ext, "ros2_run") as reload:
                    if kind == "frame_during_capture":
                        with patch.object(ext, "_marker_frame_fingerprint", side_effect=[params["frame_fingerprint"], "changed"]):
                            ext.handle_reanchor_pos(p, self.respond)
                    else:
                        ext.handle_reanchor_pos(p, self.respond)
                    self.assertEqual(self.responses[-1][1]["result"], 1)
                    self.assertEqual(self.pos.read_bytes(), original)
                    reload.assert_not_called()
        with patch.object(ext, "_capture_runtime_frame", return_value=runtime), \
             patch.object(ext.time, "time", return_value=101), \
             patch.object(ext, "ros2_run", return_value=SimpleNamespace(returncode=0, stdout="result=True")):
            ext.handle_reanchor_pos(params, self.respond)
        result = self.responses[-1][1]
        self.assertEqual(result["result"], 0)
        self.assertEqual(result["protocol"], "base-link-reanchor-v1")
        saved = json.loads(self.pos.read_text())
        self.assertEqual(saved["utm_origin"], expected)
        self.assertEqual(saved["time_stamp"], 12)
        gps = saved["wgs84_origin"]
        zone, x, y = ext._gps_to_utm(gps["latitude"], gps["longitude"])
        self.assertEqual(zone, 32)
        self.assertAlmostEqual(x, expected["x"], delta=.002)
        self.assertAlmostEqual(y, expected["y"], delta=.002)

    def test_correlated_sync_requires_pinned_payload_and_checks_hash_before_writing(self):
        params = {"sn": "A", "server": "localhost:3000", "operation_id": "sync-A"}
        with patch.object(ext, "_coverage_is_active", return_value=False), patch.object(ext, "_local_zip_md5", return_value=None):
            ext.handle_sync_map(params, self.respond)
            self.assertIn("requires", self.responses[-1][1]["error"])
            params.update(zip_url="http://localhost:3000/pinned.zip", expected_md5="0" * 32)
            class Response:
                def __enter__(self): return self
                def __exit__(self, *args): pass
                def read(self): return b"wrong ZIP version"
            with patch("urllib.request.urlopen", return_value=Response()):
                ext.handle_sync_map(params, self.respond)
            self.assertIn("hash mismatch", self.responses[-1][1]["error"])


if __name__ == "__main__":
    unittest.main()
