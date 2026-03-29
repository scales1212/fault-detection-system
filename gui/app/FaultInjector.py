from __future__ import annotations
import json
import socket


class FaultInjector:
    """Sends a one-shot fault injection command to a device simulator via UDP."""

    def __init__(self, host: str = "127.0.0.1") -> None:
        self._host = host

    def inject(self, uid: str, port: int, field_name: str) -> None:
        """Send an inject command that sets field_name=true for one response cycle."""
        payload = json.dumps({
            "uid":     uid,
            "command": "poll",
            "value":   0,
            "inject":  {field_name: True},
        })
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.sendto(payload.encode(), (self._host, port))
