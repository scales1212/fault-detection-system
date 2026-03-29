from __future__ import annotations
import json
from dataclasses import dataclass
from pathlib import Path
from typing import List


@dataclass
class DeviceConfig:
    """Parsed representation of one entry from config/devices.json."""
    uid:  str   # Unique instance identifier, e.g. "tcu-01".
    type: str   # Device type name, e.g. "TemperatureControlUnit".
    port: int   # UDP port the device simulator listens on.


class ConfigLoader:
    """Reads config/devices.json and exposes the device list.

    Expected JSON format::

        {
          "devices": [
            { "uid": "tcu-01", "type": "TemperatureControlUnit", "port": 9001 },
            { "uid": "gd-01",  "type": "GarageDoor",             "port": 9003 }
          ]
        }

    Raises:
        FileNotFoundError: if the config file does not exist.
        ValueError:        if the file is missing the ``devices`` array.
    """

    def __init__(self, config_path: str) -> None:
        self._path    = Path(config_path)
        self.devices: List[DeviceConfig] = []

    def load(self) -> None:
        """Parse the configuration file. Must be called before get_devices()."""
        if not self._path.exists():
            raise FileNotFoundError(f"Config not found: {self._path}")
        with open(self._path) as f:
            data = json.load(f)
        if "devices" not in data or not isinstance(data["devices"], list):
            raise ValueError("Config missing 'devices' array")
        self.devices = [
            DeviceConfig(uid=d["uid"], type=d["type"], port=int(d["port"]))
            for d in data["devices"]
        ]

    def get_devices(self) -> List[DeviceConfig]:
        """Return the device list populated by the most recent call to load()."""
        return self.devices
