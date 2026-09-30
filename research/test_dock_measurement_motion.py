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
        with patch.object(c.time, 'monotonic', return_value=102.86):
            c.handle_dock_measurement_control({'motion_id': ID, 'action': 'keepalive'}, lambda *a: None)
        with patch.object(c.time, 'monotonic', return_value=107):
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
        charging = dict(d, warning_charge_stop=True, error_charge_stop=True)
        self.assertFalse(c._dock_motion_chassis_ok(charging))
        self.assertTrue(c._dock_motion_chassis_ok(charging, allow_charge_stop=True))
        for key in d:
            self.assertFalse(c._dock_motion_chassis_ok(dict(d, **{key: True})))
            self.assertFalse(c._dock_motion_chassis_ok(dict(charging, **{key: True}), allow_charge_stop=True))
        del d['error_push_button_stop']
        self.assertFalse(c._dock_motion_chassis_ok(d))

    def run_motion(self, action='reverse', failure=None, from_dock=None, recovery=False):
        if from_dock is None: from_dock = action == 'reverse'
        sim = types.SimpleNamespace(now=100., next_map=100., x=0. if from_dock else -.5 if action == 'reverse' else -.7, velocity=0., homing=False, detector=False, calls=[], published=[], destroyed=False)
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
                if failure == 'stop' and sim.now > 103: state['cancelled'] = True
                speed = .15 if sim.homing else sim.velocity
                if failure == 'moving-at-start' or (failure == 'creeping' and speed == 0 and any(v < 0 for v in sim.published)):
                    speed = -.03
                if failure != 'stalled': sim.x = min(0., sim.x + speed * .05)
                if sim.x == 0 and sim.homing: sim.homing = False; speed = 0
                stamp_ns = round((1000 + sim.now - .001) * 1e9)
                h = {'stamp': {'sec': stamp_ns // 10**9, 'nanosec': stamp_ns % 10**9}}
                p = {'position': {'x': sim.x, 'y': 0., 'z': 0.}, 'orientation': {'x': 0., 'y': 0., 'z': 0., 'w': 1.}}
                if failure == 'shifted-map':
                    p['position']['x'] += 12
                    p['position']['y'] += 7
                if failure == 'initial-heading-correction' and sim.x > -.1:
                    p['position']['x'] += .15
                    p['orientation'].update(z=math.sin(.4 / 2), w=math.cos(.4 / 2))
                chassis = {k: False for k in ('warning_push_button_stop', 'error_push_button_stop', 'warning_collision_stop', 'error_collision_stop', 'warning_upraise_stop', 'error_upraise_stop', 'error_turn_over', 'error_lora', 'warning_lora_rtk_data_overtime')}
                if from_dock or failure == 'charge-fault':
                    chassis.update(warning_charge_stop=True, error_charge_stop=True)
                if failure == 'bumper' and sim.now > 103: chassis['error_collision_stop'] = True
                values = {
                    '/robot_decision/map_position': p,
                    '/robot_combination_localization/odom': {'header': h, 'twist': {'twist': {'linear': {'x': 0. if failure in ('moving-at-start', 'creeping') else speed, 'y': 0., 'z': 0.}, 'angular': {'x': 0., 'y': 0., 'z': 0.}}}},
                    '/robot_decision/robot_status': {'merged_work_status': 4 if sim.x > -.01 and failure != 'no-contact' else 2 if sim.homing else 0, 'error_status': 0, 'battery_power': 80},
                    '/bestpos_parsed_data': {'qual': 5 if failure == 'float' else 1 if failure == 'rtk' and sim.now > 103 else 4,
                                             'diff_age': 4. if failure == 'rtk-age' and sim.now > 103 else 1.},
                    '/chassis_incident': chassis,
                    '/robot_combination_localization/combination_status': {'status': 200},
                }
                if sim.detector and failure != 'missing-pattern':
                    values['/aruco/pose'] = {'header': dict(h, frame_id='aruco_tag'), 'pose': {'position': {'x': -(2 if failure == 'distant-pattern' else .1 - sim.x), 'y': 0., 'z': -.15}, 'orientation': {'x': 0., 'y': 0., 'z': 0., 'w': 1.}}}
                if failure == 'manual' and sim.now > 103: values['/cloud_move_cmd'] = {}
                if failure == 'stale' and sim.now > 103: values.pop('/robot_combination_localization/odom')
                if sim.now < sim.next_map:
                    values.pop('/robot_decision/map_position')
                else:
                    sim.next_map = sim.now + .5
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
                    c.handle_dock_measurement_move({'motion_id': ID, 'action': action, 'distance_m': (.5 if from_dock else .2) if action == 'reverse' else 0, 'from_dock': from_dock, 'recovery': recovery, 'frame_fingerprint': 'frame'}, lambda name, data: results.append(data))
                except ValueError as error: sim.error = str(error)
        self.assertTrue(sim.destroyed)
        self.assertEqual(sim.published[-1], 0.)
        self.assertTrue(state['cancelled'])
        return sim, results

    def test_real_handler_bounds_departure_then_uses_distance_for_second_step(self):
        sim, results = self.run_motion()
        self.assertEqual(results[0]['result'], 0)
        self.assertAlmostEqual(sim.x, -.5, delta=.02)
        self.assertNotIn('/robot_decision/auto_recharge', sim.calls)
        sim, results = self.run_motion(from_dock=False)
        self.assertEqual(results[0]['result'], 0)
        self.assertAlmostEqual(sim.x, -.7, delta=.06)

    def test_first_departure_accepts_heading_initialization(self):
        sim, results = self.run_motion(failure='initial-heading-correction')
        self.assertEqual(results[0]['result'], 0)
        self.assertAlmostEqual(sim.x, -.5, delta=.02)
        self.assertTrue(all(v in (0., -.08) for v in sim.published))

    def test_first_departure_accepts_float_but_later_motion_does_not(self):
        sim, results = self.run_motion(failure='float')
        self.assertEqual(results[0]['result'], 0)
        self.assertAlmostEqual(sim.x, -.5, delta=.02)
        sim, results = self.run_motion(failure='float', from_dock=False)
        self.assertFalse(results)
        self.assertFalse(any(v < 0 for v in sim.published))
        sim, results = self.run_motion(failure='charge-fault', from_dock=False)
        self.assertFalse(results)
        self.assertFalse(any(v < 0 for v in sim.published))

    def test_real_handler_visual_dock_requires_contact(self):
        sim, results = self.run_motion('dock')
        self.assertEqual(results[0]['docked'], True)
        self.assertAlmostEqual(sim.x, 0.)
        sim, results = self.run_motion('dock', 'no-contact')
        self.assertFalse(results)
        self.assertIn('/robot_decision/cancel_recharge', sim.calls)

    def test_recovery_docks_on_camera_and_contact_despite_translated_map(self):
        sim, results = self.run_motion('dock', 'shifted-map', recovery=True)
        self.assertEqual(results[0]['protocol'], 'dock-measurement-motion-v3')
        self.assertTrue(results[0]['docked'])
        self.assertAlmostEqual(sim.x, 0.)
        # The ordinary copy cycle retains its absolute saved-dock guard.
        sim, results = self.run_motion('dock', 'shifted-map')
        self.assertFalse(results)
        self.assertNotIn('/robot_decision/auto_recharge', sim.calls)
        for failure in ('missing-pattern', 'distant-pattern', 'no-contact', 'stop', 'heartbeat'):
            sim, results = self.run_motion('dock', failure, recovery=True)
            self.assertFalse(results)
            self.assertFalse(sim.homing)

    def test_retired_calibration_never_reads_or_writes_files(self):
        responses = []
        with patch('builtins.open', side_effect=AssertionError('legacy command touched files')):
            c.handle_recalibrate_charging_pose({'x': 9, 'y': 4, 'theta': 1}, lambda _, data: responses.append(data))
        self.assertEqual(responses[0]['result'], 1)

    def test_zero_twist_does_not_prove_standstill(self):
        sim, results = self.run_motion(failure='moving-at-start')
        self.assertFalse(results)
        self.assertFalse(any(v < 0 for v in sim.published), 'must not start while positions are moving')
        sim, results = self.run_motion(failure='creeping')
        self.assertFalse(results, 'must not confirm a stop while positions are moving')
        self.assertNotIn('/robot_decision/auto_recharge', sim.calls)

    def test_real_handler_aborts_and_stops_for_fault_loss_or_operator_intervention(self):
        for failure in ('stop', 'heartbeat', 'stalled', 'rtk', 'rtk-age', 'bumper', 'manual', 'stale'):
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
