#!/usr/bin/env python3
"""
Launcher for the Fault Detection System.

Reads config/devices.json, then starts:
  1. monitor.exe
  2. One device_simulator.exe per device entry
  3. The Python GUI

Press Ctrl+C to shut everything down cleanly.

Usage:
  python launch.py [--config config/devices.json] [--log fault_monitor.log]
                   [--build-dir build]
"""
import argparse
import json
import os
import signal
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent


def find_exe(build_dir: Path, name: str) -> Path:
    # MSVC puts binaries in Release/ or Debug/ subdirs
    for sub in ("Release", "Debug", ""):
        candidate = build_dir / name / sub / f"{name}.exe"
        if candidate.exists():
            return candidate
        # Also try flat layout (non-MSVC generators)
        candidate = build_dir / sub / f"{name}.exe"
        if candidate.exists():
            return candidate
    raise FileNotFoundError(
        f"Cannot find {name}.exe under {build_dir}. "
        "Run: cmake --build build --config Release"
    )


def load_devices(config_path: Path) -> list:
    with open(config_path) as f:
        return json.load(f)["devices"]


def main() -> None:
    parser = argparse.ArgumentParser(description="Fault Detection System launcher")
    parser.add_argument("--config",    default="config/devices.json")
    parser.add_argument("--log",       default="fault_monitor.log")
    parser.add_argument("--build-dir", default="build")
    args = parser.parse_args()

    config_path = REPO_ROOT / args.config
    build_dir   = REPO_ROOT / args.build_dir

    # ------------------------------------------------------------------ #
    # Locate executables
    # ------------------------------------------------------------------ #
    monitor_exe = find_exe(build_dir, "monitor")
    sim_exe     = find_exe(build_dir, "device_simulator")

    devices = load_devices(config_path)

    procs: list[subprocess.Popen] = []

    def shutdown(sig=None, frame=None) -> None:
        print("\nShutting down all processes...")
        for p in procs:
            try:
                p.terminate()
            except Exception:
                pass
        for p in procs:
            try:
                p.wait(timeout=3)
            except subprocess.TimeoutExpired:
                p.kill()
        print("All processes stopped.")
        sys.exit(0)

    signal.signal(signal.SIGINT,  shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    # ------------------------------------------------------------------ #
    # Start device simulators
    # ------------------------------------------------------------------ #
    for dev in devices:
        cmd = [
            str(sim_exe),
            "--type", dev["type"],
            "--uid",  dev["uid"],
            "--port", str(dev["port"]),
        ]
        print(f"Starting simulator: {dev['uid']} ({dev['type']}) on port {dev['port']}")
        p = subprocess.Popen(cmd, cwd=str(REPO_ROOT))
        procs.append(p)

    # Brief pause to let simulators bind their ports before the monitor starts polling
    time.sleep(0.5)

    # ------------------------------------------------------------------ #
    # Start monitor
    # ------------------------------------------------------------------ #
    monitor_cmd = [
        str(monitor_exe),
        "--config", str(config_path),
        "--log",    str(REPO_ROOT / args.log),
    ]
    print(f"Starting monitor (log → {args.log})")
    procs.append(subprocess.Popen(monitor_cmd, cwd=str(REPO_ROOT)))

    # Brief pause so the monitor creates the log file before the GUI tries to tail it
    time.sleep(0.3)

    # ------------------------------------------------------------------ #
    # Start GUI (runs in the foreground; blocks until window is closed)
    # ------------------------------------------------------------------ #
    print("Starting GUI...")
    gui_cmd = [
        sys.executable,
        str(REPO_ROOT / "gui" / "main.py"),
        "--config", str(config_path),
        "--log",    str(REPO_ROOT / args.log),
    ]
    try:
        gui_proc = subprocess.Popen(gui_cmd, cwd=str(REPO_ROOT))
        procs.append(gui_proc)
        gui_proc.wait()  # block until GUI window is closed
    except KeyboardInterrupt:
        pass

    shutdown()


if __name__ == "__main__":
    main()
