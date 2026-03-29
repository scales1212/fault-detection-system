from __future__ import annotations
import os
from pathlib import Path
from typing import List

from PyQt6.QtCore import Qt, QThread, pyqtSignal
from PyQt6.QtGui import QColor, QTextCharFormat, QTextCursor
from PyQt6.QtWidgets import QVBoxLayout, QLabel, QTextEdit, QWidget

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer


class _LogWatcher(FileSystemEventHandler, QThread):
    """Runs watchdog in a QThread; emits new_lines when the log file grows."""
    new_lines = pyqtSignal(list)

    def __init__(self, log_path: str) -> None:
        FileSystemEventHandler.__init__(self)
        QThread.__init__(self)
        self._log_path  = Path(log_path).resolve()
        self._offset    = 0
        self._observer  = Observer()

    def run(self) -> None:
        # Catch up on any content already in the file
        self._read_new_lines()
        watch_dir = str(self._log_path.parent)
        self._observer.schedule(self, watch_dir, recursive=False)
        self._observer.start()
        self._observer.join()

    def stop_watching(self) -> None:
        self._observer.stop()
        self._observer.join()

    def on_modified(self, event) -> None:
        if Path(event.src_path).resolve() == self._log_path:
            self._read_new_lines()

    def _read_new_lines(self) -> None:
        if not self._log_path.exists():
            return
        with open(self._log_path, "rb") as f:
            f.seek(self._offset)
            raw = f.read()
            self._offset = f.tell()
        if raw:
            lines = raw.decode("utf-8", errors="replace").splitlines()
            if lines:
                self.new_lines.emit(lines)


class LogViewerWidget(QWidget):
    # Colour rules keyed by log tag substring
    _COLOURS = {
        "[FAULT:FAIL]": QColor("#e53935"),   # red
        "[GROUP:FAIL]": QColor("#e53935"),
        "[FAULT:PASS]": QColor("#43a047"),   # green
        "[GROUP:PASS]": QColor("#43a047"),
        "[TIMEOUT]":    QColor("#fb8c00"),   # orange
    }

    def __init__(self, log_path: str, parent=None) -> None:
        super().__init__(parent)
        self._log_path = log_path

        layout = QVBoxLayout(self)
        layout.setContentsMargins(4, 4, 4, 4)
        layout.addWidget(QLabel("<b>Fault Log</b>"))

        self._text = QTextEdit()
        self._text.setReadOnly(True)
        self._text.setLineWrapMode(QTextEdit.LineWrapMode.NoWrap)
        self._text.setStyleSheet("font-family: monospace; font-size: 11px;")
        layout.addWidget(self._text)

        self._watcher = _LogWatcher(log_path)
        self._watcher.new_lines.connect(self._append_lines)

    def start_watching(self) -> None:
        self._watcher.start()

    def stop_watching(self) -> None:
        self._watcher.stop_watching()
        self._watcher.wait()

    def _append_lines(self, lines: List[str]) -> None:
        cursor = self._text.textCursor()
        cursor.movePosition(QTextCursor.MoveOperation.End)

        default_fmt = QTextCharFormat()
        default_fmt.setForeground(QColor("#d4d4d4"))

        for line in lines:
            fmt = QTextCharFormat(default_fmt)
            for tag, colour in self._COLOURS.items():
                if tag in line:
                    fmt.setForeground(colour)
                    break

            cursor.insertText(line + "\n", fmt)

        self._text.setTextCursor(cursor)
        self._text.ensureCursorVisible()
