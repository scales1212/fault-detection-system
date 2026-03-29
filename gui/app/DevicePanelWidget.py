from __future__ import annotations
from typing import List

from PyQt6.QtWidgets import (
    QLabel, QScrollArea, QVBoxLayout, QWidget
)

from .ConfigLoader import DeviceConfig
from .DeviceCardWidget import DeviceCardWidget
from .FaultInjector import FaultInjector


class DevicePanelWidget(QWidget):
    def __init__(self, devices: List[DeviceConfig], injector: FaultInjector,
                 parent=None) -> None:
        super().__init__(parent)

        outer = QVBoxLayout(self)
        outer.setContentsMargins(4, 4, 4, 4)
        outer.addWidget(QLabel("<b>Devices — click a fault ID to inject</b>"))

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)

        container = QWidget()
        inner = QVBoxLayout(container)
        inner.setSpacing(8)

        for dev in devices:
            card = DeviceCardWidget(dev, injector)
            inner.addWidget(card)

        inner.addStretch()
        scroll.setWidget(container)
        outer.addWidget(scroll)
