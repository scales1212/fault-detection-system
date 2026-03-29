from __future__ import annotations
import json
import socket


class FaultInjector:
    """Sends a one-shot fault injection command to a device simulator via UDP.

    The inject command sets a latch inside the simulator for a single field.
    The latch fires once — the field is true in the next monitor poll response
    only — and then clears automatically.  The simulator returns no UDP reply
    for inject commands, so the monitor is unaffected until its next cycle.

    Protocol::

        {
          "uid":     "<device-uid>",
          "command": "inject",
          "inject":  { "<field_name>": true }
        }
    """

    def __init__(self, host: str = "127.0.0.1") -> None:
        """
        Args:
            host: Target hostname or IP address (default localhost).
        """
        self._host = host

    def inject(self, uid: str, port: int, field_name: str) -> None:
        """Send an inject command that sets field_name=true for one response cycle.

        Args:
            uid:        Device instance UID (e.g. ``"tcu-01"``).
            port:       UDP port the target simulator is bound to.
            field_name: Sensor field to latch (e.g. ``"overheating"``).
        """
        payload = json.dumps({
            "uid":     uid,
            "command": "inject",
            "inject":  {field_name: True},
        })
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.sendto(payload.encode(), (self._host, port))
