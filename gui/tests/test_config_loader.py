import json
import pytest
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from gui.app.ConfigLoader import ConfigLoader, DeviceConfig


def test_parses_valid_config(tmp_path):
    cfg = {"devices": [
        {"uid": "tcu-01", "type": "TemperatureControlUnit", "port": 9001},
        {"uid": "gd-01",  "type": "GarageDoor",             "port": 9003},
    ]}
    p = tmp_path / "devices.json"
    p.write_text(json.dumps(cfg))

    loader = ConfigLoader(str(p))
    loader.load()
    devs = loader.get_devices()
    assert len(devs) == 2
    assert devs[0].uid  == "tcu-01"
    assert devs[0].type == "TemperatureControlUnit"
    assert devs[0].port == 9001
    assert devs[1].uid  == "gd-01"


def test_raises_on_missing_file():
    with pytest.raises(FileNotFoundError):
        ConfigLoader("nonexistent.json").load()


def test_raises_on_missing_devices_key(tmp_path):
    p = tmp_path / "bad.json"
    p.write_text(json.dumps({"foo": []}))
    with pytest.raises(ValueError):
        ConfigLoader(str(p)).load()
