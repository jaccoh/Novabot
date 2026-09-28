"""Run: python3 research/test_dock_measurement_motion.py. No ROS or hardware."""
import importlib.util
import math
import os
import sys
import tempfile
import json
import types
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('dock_commands', os.path.join(os.path.dirname(__file__), 'extended_commands.py'))
c = importlib.util.module_from_spec(spec); spec.loader.exec_module(c)
ID = '12345678-1234-1234-1234-123456789abc'


class MotionTest(unittest.TestCase):
    def setUp(self):
        c._DOCK_MOTIONS.clear()

    def test_single_use_arm_cancel_and_expired_heartbeat(self):
        with patch.object(c.time, 'monotonic', return_value=100):
            c.handle_dock_measurement_control({'motion_id': ID, 'action': 'arm'}, lambda *a: None)
            with self.assertRaises(ValueError):
                c.handle_dock_measurement_control({'motion_id': ID, 'action': 'arm'}, lambda *a: None)
        with patch.object(c.time, 'monotonic', return_value=103):
            with self.assertRaises(ValueError):
                c.handle_dock_measurement_control({'motion_id': ID, 'action': 'keepalive'}, lambda *a: None)
        self.assertTrue(c._DOCK_MOTIONS[ID]['cancelled'])
        c._DOCK_MOTIONS.clear()
        c.handle_dock_measurement_control({'motion_id': ID, 'action': 'stop'}, lambda *a: None)
        with self.assertRaises(ValueError):
            c.handle_dock_measurement_control({'motion_id': ID, 'action': 'arm'}, lambda *a: None)

    def test_signed_distance_heading_sideways_and_jump_guards(self):
        start = (0, 0, 0)
        self.assertAlmostEqual(c._dock_motion_progress(start, (-.5, 0, 0), (-.48, 0, 0), .5), .5)
        for pose, previous in [((.04, 0, 0), start), ((-.02, .07, 0), start), ((-.02, 0, .2), start), ((-.5, 0, 0), start), ((-.57, 0, 0), (-.55, 0, 0)), ((math.nan, 0, 0), start)]:
            with self.subTest(pose=pose), self.assertRaises(ValueError):
                c._dock_motion_progress(start, pose, previous, .5)

    def test_chassis_faults_and_missing_stop_sensor_are_rejected(self):
        d = {k: False for k in ('warning_push_button_stop', 'error_push_button_stop', 'warning_collision_stop', 'error_collision_stop', 'warning_upraise_stop', 'error_upraise_stop', 'error_turn_over', 'error_lora', 'warning_lora_rtk_data_overtime')}
        self.assertTrue(c._dock_motion_chassis_ok(d))
        for key in d:
            self.assertFalse(c._dock_motion_chassis_ok(dict(d, **{key: True})))
        del d['error_push_button_stop']
        self.assertFalse(c._dock_motion_chassis_ok(d))

    def run_motion(self, action='reverse', failure=None):
        sim = types.SimpleNamespace(now=100., x=0. if action == 'reverse' else -.7, velocity=0., homing=False, detector=False, calls=[], published=[], destroyed=False)
        subscriptions = {}
        state = {'created': 100., 'heartbeat': 100., 'cancelled': False, 'used': False}
        c._DOCK_MOTIONS[ID] = state
        class Twist:
            def __init__(self):
                self.linear = types.SimpleNamespace(x=0., y=0., z=0.)
                self.angular = types.SimpleNamespace(x=0., y=0., z=0.)
        class Service:
            class Request: pass
        class Client:
            def __init__(self, name): self.name = name
            def wait_for_service(self, **kw): return True
            def call_async(self, req):
                sim.calls.append(self.name)
                if self.name.endswith('auto_recharge'): sim.homing = True
                if self.name.endswith('cancel_recharge') or self.name.endswith('cancel_goal'): sim.homing = False
                if self.name == '/enable_aruco_localization': sim.detector = req.data
                return types.SimpleNamespace(done=lambda: True, result=lambda: types.SimpleNamespace(success=True, return_code=0))
        class Publisher:
            def __init__(self, topic): self.topic = topic
            def get_subscription_count(self): return 1
            def publish(self, msg):
                if self.topic == '/cmd_vel':
                    sim.velocity = msg.linear.x
                    sim.published.append(sim.velocity)
        class Node:
            def create_publisher(self, kind, topic, qos): return Publisher(topic)
            def create_client(self, kind, name): return Client(name)
            def create_subscription(self, kind, topic, cb, qos): subscriptions[topic] = cb
            def destroy_node(self): sim.destroyed = True
        class Context:
            def shutdown(self): pass
        class Executor:
            def __init__(self, **kw): pass
            def add_node(self, node): pass
            def remove_node(self, node): pass
            def shutdown(self): pass
            def spin_once(self, timeout_sec):
                sim.now += .05
                if failure != 'heartbeat': state['heartbeat'] = sim.now
                if failure == 'stop' and sim.now > 101: state['cancelled'] = True
                speed = .15 if sim.homing else sim.velocity
                if failure != 'stalled': sim.x = min(0., sim.x + speed * .05)
                if sim.x == 0 and sim.homing: sim.homing = False; speed = 0
                stamp_ns = round((1000 + sim.now - .001) * 1e9)
                h = {'stamp': {'sec': stamp_ns // 10**9, 'nanosec': stamp_ns % 10**9}}
                p = {'position': {'x': sim.x, 'y': 0., 'z': 0.}, 'orientation': {'x': 0., 'y': 0., 'z': 0., 'w': 1.}}
                chassis = {k: False for k in ('warning_push_button_stop', 'error_push_button_stop', 'warning_collision_stop', 'error_collision_stop', 'warning_upraise_stop', 'error_upraise_stop', 'error_turn_over', 'error_lora', 'warning_lora_rtk_data_overtime')}
                if failure == 'bumper' and sim.now > 101: chassis['error_collision_stop'] = True
                values = {
                    '/robot_decision/map_position': p,
                    '/robot_combination_localization/odom': {'header': h, 'twist': {'twist': {'linear': {'x': speed, 'y': 0., 'z': 0.}, 'angular': {'x': 0., 'y': 0., 'z': 0.}}}},
                    '/robot_decision/robot_status': {'merged_work_status': 4 if sim.x > -.01 and failure != 'no-contact' else 2 if sim.homing else 0, 'error_status': 0, 'battery_power': 80},
                    '/bestpos_parsed_data': {'qual': 5 if failure == 'rtk' and sim.now > 101 else 4, 'diff_age': 1.},
                    '/chassis_incident': chassis,
                    '/robot_combination_localization/combination_status': {'status': 200},
                }
                if sim.detector and failure != 'missing-pattern': values['/aruco/pose'] = {'header': h}
                if failure == 'manual' and sim.now > 101: values['/cloud_move_cmd'] = {}
                if failure == 'stale' and sim.now > 101: values.pop('/robot_combination_localization/odom')
                for topic, cb in list(subscriptions.items()):
                    if topic in values: cb(values[topic])
        modules = {}
        def module(name, **members):
            m = types.ModuleType(name); m.__dict__.update(members); modules[name] = m; return m
        module('rclpy', init=lambda **kw: None, create_node=lambda *a, **kw: Node())
        module('rclpy.context', Context=Context); module('rclpy.executors', SingleThreadedExecutor=Executor)
        module('rclpy.qos', QoSProfile=lambda **kw: None, ReliabilityPolicy=types.SimpleNamespace(BEST_EFFORT=0))
        module('rosidl_runtime_py.utilities', get_message=lambda kind: object)
        module('rosidl_runtime_py.convert', message_to_ordereddict=lambda msg: msg)
        module('geometry_msgs.msg', Twist=Twist); module('std_msgs.msg', UInt8=lambda **kw: kw)
        module('std_srvs.srv', Trigger=Service, SetBool=Service)
        module('action_msgs.srv', CancelGoal=Service)
        results = []
        with tempfile.TemporaryDirectory() as directory:
            os.mkdir(directory + '/csv_file')
            with open(directory + '/csv_file/map_info.json', 'w') as f: json.dump({'charging_pose': {'x': 0, 'y': 0, 'orientation': 0}}, f)
            with patch.dict(sys.modules, modules), patch.object(c, '_marker_frame_fingerprint', return_value='frame'), patch.object(c, '_map_home', return_value=directory), patch.object(c, '_marker_camera_use', side_effect=lambda action, *args: sim.now + 30), patch.object(c.time, 'monotonic', side_effect=lambda: sim.now), patch.object(c.time, 'time', side_effect=lambda: sim.now + 1000):
                try:
                    c.handle_dock_measurement_move({'motion_id': ID, 'action': action, 'distance_m': .5 if action == 'reverse' else 0, 'from_dock': action == 'reverse', 'frame_fingerprint': 'frame'}, lambda name, data: results.append(data))
                except ValueError as error: sim.error = str(error)
        self.assertTrue(sim.destroyed)
        self.assertEqual(sim.published[-1], 0.)
        self.assertTrue(state['cancelled'])
        return sim, results

    def test_real_handler_reverses_to_measured_distance_then_confirms_stop(self):
        sim, results = self.run_motion()
        self.assertEqual(results[0]['result'], 0)
        self.assertAlmostEqual(sim.x, -.5, delta=.02)
        self.assertNotIn('/robot_decision/auto_recharge', sim.calls)

    def test_real_handler_visual_dock_requires_contact(self):
        sim, results = self.run_motion('dock')
        self.assertEqual(results[0]['docked'], True)
        self.assertAlmostEqual(sim.x, 0.)
        sim, results = self.run_motion('dock', 'no-contact')
        self.assertFalse(results)
        self.assertIn('/robot_decision/cancel_recharge', sim.calls)

    def test_real_handler_aborts_and_stops_for_fault_loss_or_operator_intervention(self):
        for failure in ('stop', 'heartbeat', 'stalled', 'rtk', 'bumper', 'manual', 'stale'):
            with self.subTest(failure=failure):
                sim, results = self.run_motion(failure=failure)
                self.assertFalse(results)
                self.assertTrue(sim.error)
                self.assertNotIn('/robot_decision/auto_recharge', sim.calls)
        for failure in ('stop', 'heartbeat', 'missing-pattern'):
            with self.subTest(docking=failure):
                sim, results = self.run_motion('dock', failure)
                self.assertFalse(results)
                self.assertFalse(sim.homing)


if __name__ == '__main__': unittest.main()
