from __future__ import annotations
import os
from pathlib import Path
from typing import List

from PyQt6.QtCore import QThread, pyqtSignal
from PyQt6.QtGui import QColor, QTextCharFormat, QTextCursor
from PyQt6.QtWidgets import QVBoxLayout, QLabel, QTextEdit, QWidget

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

# Only lines containing one of these tags are shown in the GUI.
# The full log file still receives every entry.
_SHOW_TAGS = ("[FAULT:FAIL]", "[FAULT:PASS]", "[GROUP:FAIL]", "[GROUP:PASS]")


def _norm(path: str) -> str:
    """Normalise a path for comparison: absolute + OS case rules."""
    return os.path.normcase(os.path.abspath(path))


class _LogWatcher(FileSystemEventHandler, QThread):
    """Tails the log file in a background QThread; emits new_lines on change."""
    new_lines = pyqtSignal(list)

    def __init__(self, log_path: str) -> None:
        FileSystemEventHandler.__init__(self)
        QThread.__init__(self)
        self._log_path_norm = _norm(log_path)
        self._log_path      = Path(log_path)
        self._offset        = 0
        self._observer      = Observer()

    def run(self) -> None:
        self._read_new_lines()
        self._observer.schedule(self, str(self._log_path.parent), recursive=False)
        self._observer.start()
        self._observer.join()

    def stop_watching(self) -> None:
        self._observer.stop()

    # Handle both on_created (file appears after watcher starts) and on_modified
    def on_created(self, event) -> None:
        if not event.is_directory and _norm(event.src_path) == self._log_path_norm:
            self._offset = 0          # reset so we read from the beginning
            self._read_new_lines()

    def on_modified(self, event) -> None:
        if not event.is_directory and _norm(event.src_path) == self._log_path_norm:
            self._read_new_lines()

    def _read_new_lines(self) -> None:
        if not self._log_path.exists():
            return
        try:
            with open(self._log_path, "rb") as f:
                f.seek(self._offset)
                raw = f.read()
                self._offset = f.tell()
        except OSError:
            return
        if raw:
            lines = raw.decode("utf-8", errors="replace").splitlines()
            if lines:
                self.new_lines.emit(lines)


class LogViewerWidget(QWidget):
    _COLOURS = {
        "[FAULT:FAIL]": QColor("#e53935"),   # red
        "[GROUP:FAIL]": QColor("#e53935"),
        "[FAULT:PASS]": QColor("#43a047"),   # green
        "[GROUP:PASS]": QColor("#43a047"),
    }

    def __init__(self, log_path: str, parent=None) -> None:
        super().__init__(parent)

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
        # Show only fault/group state-change lines — filter out SEND/RECV/TIMEOUT noise
        visible = [ln for ln in lines if any(tag in ln for tag in _SHOW_TAGS)]
        if not visible:
            return

        cursor = self._text.textCursor()
        cursor.movePosition(QTextCursor.MoveOperation.End)

        default_fmt = QTextCharFormat()
        default_fmt.setForeground(QColor("#d4d4d4"))

        for line in visible:
            fmt = QTextCharFormat(default_fmt)
            for tag, colour in self._COLOURS.items():
                if tag in line:
                    fmt.setForeground(colour)
                    break
            cursor.insertText(line + "\n", fmt)

        self._text.setTextCursor(cursor)
        self._text.ensureCursorVisible()
