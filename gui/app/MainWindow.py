from __future__ import annotations

from PyQt6.QtWidgets import QMainWindow, QSplitter
from PyQt6.QtCore import Qt

from .ConfigLoader import ConfigLoader
from .DevicePanelWidget import DevicePanelWidget
from .FaultInjector import FaultInjector
from .LogViewerWidget import LogViewerWidget


class MainWindow(QMainWindow):
    """Top-level application window.

    Layout:
        A horizontal QSplitter divides the window into two panels:
          - Left (60 %): LogViewerWidget — live fault log tail.
          - Right (40 %): DevicePanelWidget — one inject card per device.

    Lifecycle:
        The log file watcher is started in __init__ and stopped cleanly in
        closeEvent so the background QThread exits before the process does.
    """

    def __init__(self, config_path: str, log_path: str) -> None:
        """
        Args:
            config_path: Path to config/devices.json.
            log_path:    Path to fault_monitor.log (tailed by LogViewerWidget).
        """
        super().__init__()
        self.setWindowTitle("Fault Detection System")
        self.resize(1200, 700)

        config = ConfigLoader(config_path)
        config.load()

        injector = FaultInjector()

        self._log_viewer   = LogViewerWidget(log_path)
        self._device_panel = DevicePanelWidget(config.get_devices(), injector)

        splitter = QSplitter(Qt.Orientation.Horizontal)
        splitter.addWidget(self._log_viewer)
        splitter.addWidget(self._device_panel)
        splitter.setSizes([720, 480])

        self.setCentralWidget(splitter)
        self._log_viewer.start_watching()

    def closeEvent(self, event) -> None:
        """Stop the log watcher background thread before closing."""
        self._log_viewer.stop_watching()
        super().closeEvent(event)
