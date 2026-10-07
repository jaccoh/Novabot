"""Local lifecycle regressions: python3 research/test_camera_stream.py (no ROS)."""
import importlib.util
import io
import json
import os
import types
import unittest
from unittest.mock import Mock, patch


spec = importlib.util.spec_from_file_location("camera_stream_tested", os.path.join(os.path.dirname(__file__), "camera_stream.py"))
camera = importlib.util.module_from_spec(spec)
with patch("signal.signal"):
    spec.loader.exec_module(camera)


class CameraLifecycleTests(unittest.TestCase):
    def setUp(self):
        self.registry = camera.CameraRegistry()
        self.registry.node = object()
        self.front = self.registry.get('front')
        self.hd = self.registry.get('front_hd')
        self.aruco = self.registry.get('aruco')
        for manager in self.registry.managers.values():
            manager._subscribed = True
            manager._watchdog_thread = Mock(is_alive=lambda: True)
            manager._call_stop_camera = Mock()
        self.front._call_start_camera = Mock(return_value=True)

    def test_capture_wakes_camera_and_protects_shared_hardware_until_cleanup(self):
        self.hd._active = True
        self.hd.last_viewer_time = 1
        with patch.object(camera.time, 'monotonic', return_value=1000):
            until = self.registry.begin_marker_use()
            self.assertEqual(until, 1030)
            self.front._call_start_camera.assert_called_once_with(self.registry.node, allow_fallback=False)
            self.assertTrue(self.front.is_active)
            # Both a stale alias watchdog and the detector watchdog must stay quiet.
            self.aruco._active = True
            self.aruco.last_viewer_time = 1
            for manager in (self.front, self.hd, self.aruco):
                self.registry.stop_if_idle(manager)
                manager._call_stop_camera.assert_not_called()
            with self.assertRaisesRegex(RuntimeError, 'measurement'):
                self.aruco.viewer_start(self.registry.node)
            self.assertEqual(self.aruco.active_viewers, 0)
            # Normal video stays available; releasing the hold never stops it.
            self.hd.viewer_start(self.registry.node)
            self.registry.end_marker_use(until)
            for manager in (self.front, self.hd, self.aruco):
                manager._call_stop_camera.assert_not_called()
        with patch.object(camera.time, 'monotonic', return_value=1400):
            self.registry.stop_if_idle(self.front)
            self.front._call_stop_camera.assert_not_called()
            self.hd.viewer_stop()
        with patch.object(camera.time, 'monotonic', return_value=1600):
            self.registry.stop_if_idle(self.front)
            self.front._call_stop_camera.assert_not_called()
        with patch.object(camera.time, 'monotonic', return_value=1701):
            self.registry.stop_if_idle(self.front)
            self.front._call_stop_camera.assert_called_once()
            self.assertFalse(self.hd.is_active)

    def test_alias_uses_latest_activity_not_its_own_old_idle_deadline(self):
        self.front._active = self.hd._active = True
        self.hd.last_viewer_time = 1
        self.front.last_viewer_time = 990
        self.hd.latest_frame = b'old'
        with patch.object(camera.time, 'monotonic', return_value=1000):
            self.registry.stop_if_idle(self.hd)
            self.hd._call_stop_camera.assert_not_called()
        with patch.object(camera.time, 'monotonic', return_value=1291):
            self.registry.stop_if_idle(self.hd)
            self.hd._call_stop_camera.assert_called_once()
            self.assertFalse(self.front.is_active)
            self.assertIsNone(self.hd.get_frame())

    def test_busy_detector_or_pending_enable_cannot_race_capture_cleanup(self):
        with patch.object(camera.time, 'monotonic', return_value=1000):
            for field in ('_active', '_activating', 'active_viewers'):
                setattr(self.aruco, field, True)
                with self.assertRaisesRegex(RuntimeError, 'busy'):
                    self.registry.begin_marker_use()
                setattr(self.aruco, field, False)
            self.front._call_start_camera.assert_not_called()
            until = self.registry.begin_marker_use()
            with self.assertRaisesRegex(RuntimeError, 'busy'):
                self.registry.begin_marker_use()
            self.registry.end_marker_use(until)
        with patch.object(camera.time, 'monotonic', return_value=1001):
            next_until = self.registry.begin_marker_use()
            self.registry.end_marker_use(until)
            self.assertEqual(self.registry.marker_use_until, next_until)

    def test_abandoned_hold_expires_and_does_not_enable_detector(self):
        self.aruco._call_start_camera = Mock()
        with patch.object(camera.time, 'monotonic', return_value=1000):
            self.registry.begin_marker_use()
        with patch.object(camera.time, 'monotonic', return_value=1400):
            self.registry.stop_if_idle(self.front)
        self.front._call_stop_camera.assert_called_once()
        self.aruco._call_start_camera.assert_not_called()

    def test_negative_camera_ack_is_failure_without_cli_retry_or_detector_enable(self):
        response = types.SimpleNamespace(success=False)
        client = Mock()
        client.wait_for_service.return_value = True
        client.call_async.return_value = types.SimpleNamespace(done=lambda: True, result=lambda: response)
        node = Mock(create_client=lambda *_: client)
        service = types.SimpleNamespace(SetBool=types.SimpleNamespace(Request=lambda: types.SimpleNamespace(data=False)))
        del self.front._call_start_camera  # exercise the real service acknowledgement path
        with patch.dict('sys.modules', {'std_srvs.srv': service}), patch('subprocess.run') as cli:
            self.registry.node = node
            with self.assertRaisesRegex(RuntimeError, 'not acknowledged'):
                self.registry.begin_marker_use()
        cli.assert_not_called()
        client.wait_for_service.assert_called_once_with(timeout_sec=2.0)
        self.assertFalse(self.front.is_active)
        self.assertFalse(self.front._activating)
        self.assertEqual(self.registry.marker_use_until, 0)

    def test_stream_without_first_frame_releases_viewer(self):
        self.front._active = True
        handler = object.__new__(camera.StreamHandler)
        handler.client_address = ('127.0.0.1', 1234)
        handler.wfile = io.BytesIO()
        handler.send_response = Mock()
        handler.send_header = Mock()
        handler.end_headers = Mock()
        with patch.object(camera, 'registry', self.registry), patch.object(camera.time, 'time', side_effect=[0, 16]), patch.object(camera.time, 'sleep'):
            handler._handle_stream('front')
        self.assertEqual(self.front.active_viewers, 0)
        self.assertEqual(handler.wfile.getvalue(), b'')

    def test_http_hold_is_local_only_and_reports_camera_failure(self):
        handler = object.__new__(camera.StreamHandler)
        handler.path = '/marker-camera-use'
        handler.client_address = ('192.168.0.1', 1234)
        handler.send_error = Mock()
        with patch.object(camera, 'registry', self.registry):
            handler.do_POST()
        handler.send_error.assert_called_once_with(403)
        self.front._call_start_camera.assert_not_called()

        handler.client_address = ('127.0.0.1', 1234)
        handler.headers = {'Content-Length': '2'}
        handler.rfile = io.BytesIO(b'[]')
        handler.send_error.reset_mock()
        with patch.object(camera, 'registry', self.registry):
            handler.do_POST()
        self.assertEqual(handler.send_error.call_args.args[0], 400)

        body = json.dumps({'action': 'begin'}).encode()
        handler.headers = {'Content-Length': str(len(body))}
        handler.rfile = io.BytesIO(body)
        handler.send_error.reset_mock()
        self.front._call_start_camera.return_value = False
        with patch.object(camera, 'registry', self.registry):
            handler.do_POST()
        self.assertEqual(handler.send_error.call_args.args[0], 503)
        self.assertEqual(self.registry.marker_use_until, 0)

    def test_idle_stop_waits_for_the_firmware(self):
        self.front._active = True
        self.front.last_viewer_time = 1
        with patch.object(camera.time, 'monotonic', return_value=1000):
            with patch.object(camera.firmware_state, 'firmware_busy', return_value=True):
                self.registry.stop_if_idle(self.front)
                self.front._call_stop_camera.assert_not_called()
            with patch.object(camera.firmware_state, 'firmware_busy', return_value=False):
                self.registry.stop_if_idle(self.front)
                self.front._call_stop_camera.assert_called_once()


if __name__ == '__main__':
    unittest.main()
