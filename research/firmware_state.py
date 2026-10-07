#!/usr/bin/env python3
"""Is the firmware busy with a task, mapping or docking?

Read from the tail of robot_decision's own log, the way extended_commands.py
reads it for the cadence: the last `Mode:... Work:<state> ... Recharge: <state>`
line is the firmware's current state. No ROS, no subprocess: a daemon that
calls this every few seconds must not become an iceoryx participant.

Busy means: any Work state other than idle (WAIT, FINISHED, CANCELLED, FAILED)
or a recharge that is driving. A parked task (USER_STOP, ERROR_STOP) counts as
busy too: the mower is standing in the field waiting for Resume.

robot_decision logs at least every 30 s while it runs (cpu usage, cov_ratio),
so a log untouched for DEAD_AFTER_S means it is gone and nothing is busy.
"""
import os
import re
import time

LOG_DIR = "/root/novabot/data/ros2_log"
DEAD_AFTER_S = 180.0
IDLE_WORK = ("WAIT", "FINISHED", "CANCELLED", "FAILED")
IDLE_RECHARGE = ("IDLE", "FINISHED", "CANCELLED", "FAILED", "WAIT", "NONE")
TAIL_BYTES = 200_000


def newest_log(log_dir=LOG_DIR):
    """Path of the most recently modified robot_decision_*.log, or None."""
    try:
        fs = [os.path.join(log_dir, f) for f in os.listdir(log_dir)
              if f.startswith("robot_decision_") and f.endswith(".log")]
    except OSError:
        return None
    return max(fs, key=os.path.getmtime) if fs else None


def last_state_line(path):
    try:
        size = os.path.getsize(path)
        with open(path, "rb") as fh:
            if size > TAIL_BYTES:
                fh.seek(-TAIL_BYTES, os.SEEK_END)
            text = fh.read().decode("utf-8", "replace")
    except OSError:
        return None
    line = None
    for ln in text.splitlines():
        if "Work:" in ln:
            line = ln
    return line


def firmware_busy(log_dir=LOG_DIR, now=None):
    path = newest_log(log_dir)
    if not path:
        return False
    try:
        mtime = os.path.getmtime(path)
    except OSError:
        return False
    if (now if now is not None else time.time()) - mtime > DEAD_AFTER_S:
        return False
    line = last_state_line(path)
    if not line:
        return False
    rm = re.search(r"Recharge:\s*(\w+)", line)
    if rm and rm.group(1) not in IDLE_RECHARGE:
        return True
    wm = re.search(r"Work:(\w+)", line)
    return bool(wm) and wm.group(1) not in IDLE_WORK
