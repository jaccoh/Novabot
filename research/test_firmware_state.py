"""python3 research/test_firmware_state.py — no ROS, temp logs only."""
import os
import sys
import tempfile
import time
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import firmware_state as fs  # noqa: E402

LINE = "[2026-10-07-09:00:00][INFO] [1791356400.0] [robot_decision]: Mode:COVERAGE Work:%s Prev work:WAIT Recharge: %s"


class FirmwareBusy(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()

    def write(self, work, recharge="WAIT", age_s=0.0, name="robot_decision_1.log"):
        p = os.path.join(self.dir, name)
        with open(p, "w") as fh:
            fh.write("[..] cpu usage 10%\n" + (LINE % (work, recharge)) + "\n[..] cov_ratio: 1%\n")
        t = time.time() - age_s
        os.utime(p, (t, t))
        return p

    def test_idle_states_are_not_busy(self):
        for w in ("WAIT", "FINISHED", "CANCELLED"):
            self.write(w)
            self.assertFalse(fs.firmware_busy(self.dir), w)

    def test_task_mapping_parked_and_docking_are_busy(self):
        for w, r in (("COVERING", "WAIT"), ("MANUAL_MAPPING_WORKING_ZONE", "WAIT"),
                     ("USER_STOP", "WAIT"), ("WAIT", "RETURN_TO_PILE"), ("WAIT", "ALIGN_PILE")):
            self.write(w, r)
            self.assertTrue(fs.firmware_busy(self.dir), (w, r))

    def test_dead_robot_decision_is_not_busy(self):
        self.write("COVERING", age_s=600)
        self.assertFalse(fs.firmware_busy(self.dir))

    def test_newest_log_wins_and_missing_dir_is_idle(self):
        self.write("COVERING", age_s=30, name="robot_decision_old.log")
        self.write("WAIT", name="robot_decision_new.log")
        self.assertFalse(fs.firmware_busy(self.dir))
        self.assertFalse(fs.firmware_busy(os.path.join(self.dir, "nope")))


if __name__ == "__main__":
    unittest.main()
