from __future__ import annotations
import sys
from pathlib import Path
from typing import List

from PyQt6.QtWidgets import QGroupBox, QVBoxLayout, QPushButton, QLabel

# Add generated/python to path so we can import the registry
_REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(_REPO_ROOT / "generated" / "python"))

from fault_ids import FAULT_ID_REGISTRY, FaultId  # noqa: E402
from .ConfigLoader import DeviceConfig
from .FaultInjector import FaultInjector


class DeviceCardWidget(QGroupBox):
    def __init__(self, config: DeviceConfig, injector: FaultInjector, parent=None) -> None:
        super().__init__(f"{config.uid}  [{config.type}]", parent)
        self._config   = config
        self._injector = injector

        layout = QVBoxLayout(self)
        layout.setSpacing(4)

        # Find all fault IDs that apply to this device type
        fault_ids: List[FaultId] = [
            fid for (dtype, _), fid in FAULT_ID_REGISTRY.items()
            if dtype == config.type
        ]

        if not fault_ids:
            layout.addWidget(QLabel("(no fault fields defined)"))
            return

        for fid in fault_ids:
            btn = QPushButton(fid.id)
            groups_str = ", ".join(fid.groups)
            btn.setToolTip(
                f"Field: {fid.field}\n"
                f"Numeric ID: {fid.numeric_id}\n"
                f"Groups: {groups_str}"
            )
            btn.setStyleSheet(
                "QPushButton { background: #37474f; color: #eceff1; border-radius: 4px; padding: 4px 8px; }"
                "QPushButton:hover { background: #e53935; }"
            )
            # Capture fid.field in the closure
            btn.clicked.connect(
                lambda checked, field=fid.field: self._on_inject(field)
            )
            layout.addWidget(btn)

    def _on_inject(self, field_name: str) -> None:
        self._injector.inject(
            uid=self._config.uid,
            port=self._config.port,
            field_name=field_name,
        )
