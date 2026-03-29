import json
import socket
import sys
import threading
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from gui.app.FaultInjector import FaultInjector


def _receive_one(port: int, received: list, ready: threading.Event) -> None:
    """Listen for a single UDP packet on localhost:port and store the payload."""
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(("127.0.0.1", port))
        s.settimeout(2.0)
        ready.set()
        try:
            data, _ = s.recvfrom(4096)
            received.append(data.decode())
        except socket.timeout:
            pass


def test_inject_sends_correct_json():
    port     = 19876  # use a high port to avoid conflicts
    received = []
    ready    = threading.Event()

    t = threading.Thread(target=_receive_one, args=(port, received, ready), daemon=True)
    t.start()
    ready.wait(timeout=1.0)

    injector = FaultInjector()
    injector.inject(uid="tcu-01", port=port, field_name="overheating")

    t.join(timeout=2.0)

    assert len(received) == 1
    payload = json.loads(received[0])
    assert payload["uid"]     == "tcu-01"
    assert payload["command"] == "poll"
    assert payload["inject"]["overheating"] is True


def test_inject_different_field():
    port     = 19877
    received = []
    ready    = threading.Event()

    t = threading.Thread(target=_receive_one, args=(port, received, ready), daemon=True)
    t.start()
    ready.wait(timeout=1.0)

    injector = FaultInjector()
    injector.inject(uid="gd-01", port=port, field_name="garage_open")

    t.join(timeout=2.0)
    assert len(received) == 1
    payload = json.loads(received[0])
    assert payload["inject"]["garage_open"] is True
    assert payload["uid"] == "gd-01"
