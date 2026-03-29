from __future__ import annotations
import sys
from pathlib import Path
from typing import List

from PyQt6.QtWidgets import QGroupBox, QVBoxLayout, QPushButton, QLabel

# Ensure the generated Python types are importable regardless of working directory.
_REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(_REPO_ROOT / "generated" / "python"))

from fault_ids import FAULT_ID_REGISTRY, FaultId  # noqa: E402
from .ConfigLoader import DeviceConfig
from .FaultInjector import FaultInjector


class DeviceCardWidget(QGroupBox):
    """A control card for a single device instance.

    Displays one inject button per fault ID that applies to the device's type.
    Button labels are fault ID strings (e.g. ``TCU_OVERHEAT``); tooltips show the
    field name, numeric ID, and group memberships.

    The fault ID list is derived entirely from the generated ``FAULT_ID_REGISTRY``
    — no device names or field names are hard-coded here.

    Clicking a button calls FaultInjector.inject(), which sends a UDP inject
    command to the simulator.  The simulator latches the fault for one poll cycle
    and the monitor logs the FAIL → PASS transition.
    """

    def __init__(self, config: DeviceConfig, injector: FaultInjector, parent=None) -> None:
        """
        Args:
            config:   Device instance metadata (uid, type, port).
            injector: Shared FaultInjector used to send UDP inject commands.
            parent:   Optional Qt parent widget.
        """
        super().__init__(f"{config.uid}  [{config.type}]", parent)
        self._config   = config
        self._injector = injector

        layout = QVBoxLayout(self)
        layout.setSpacing(4)

        # Filter the global registry to only the fault IDs for this device type.
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
            # Capture fid.field in the closure with a default argument to avoid
            # the common late-binding pitfall in Python loop lambdas.
            btn.clicked.connect(
                lambda checked, field=fid.field: self._on_inject(field)
            )
            layout.addWidget(btn)

    def _on_inject(self, field_name: str) -> None:
        """Send an inject command for the given field to this device's simulator."""
        self._injector.inject(
            uid=self._config.uid,
            port=self._config.port,
            field_name=field_name,
        )
