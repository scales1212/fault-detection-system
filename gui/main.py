#!/usr/bin/env python3
"""Entry point for the Fault Detection System GUI."""
import argparse
import sys
from pathlib import Path

from PyQt6.QtWidgets import QApplication

# Ensure repo root is on the path so generated/ modules are importable
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from app.MainWindow import MainWindow  # noqa: E402


def main() -> None:
    parser = argparse.ArgumentParser(description="Fault Detection System GUI")
    parser.add_argument("--config", default="config/devices.json",
                        help="Path to devices.json")
    parser.add_argument("--log",    default="fault_monitor.log",
                        help="Path to the monitor log file")
    args = parser.parse_args()

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    # Dark palette
    from PyQt6.QtGui import QPalette, QColor
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window,          QColor("#263238"))
    palette.setColor(QPalette.ColorRole.WindowText,      QColor("#eceff1"))
    palette.setColor(QPalette.ColorRole.Base,            QColor("#1c2b33"))
    palette.setColor(QPalette.ColorRole.AlternateBase,   QColor("#263238"))
    palette.setColor(QPalette.ColorRole.Text,            QColor("#eceff1"))
    palette.setColor(QPalette.ColorRole.Button,          QColor("#37474f"))
    palette.setColor(QPalette.ColorRole.ButtonText,      QColor("#eceff1"))
    palette.setColor(QPalette.ColorRole.Highlight,       QColor("#0288d1"))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor("#ffffff"))
    app.setPalette(palette)

    window = MainWindow(args.config, args.log)
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
