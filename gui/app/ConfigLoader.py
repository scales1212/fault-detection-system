from __future__ import annotations
import json
from dataclasses import dataclass
from pathlib import Path
from typing import List


@dataclass
class DeviceConfig:
    uid:  str
    type: str
    port: int


class ConfigLoader:
    def __init__(self, config_path: str) -> None:
        self._path    = Path(config_path)
        self.devices: List[DeviceConfig] = []

    def load(self) -> None:
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
        return self.devices
