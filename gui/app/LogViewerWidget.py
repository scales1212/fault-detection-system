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
# The full log file still receives every entry (SEND, RECV, TIMEOUT, …).
_SHOW_TAGS = ("[FAULT:FAIL]", "[FAULT:PASS]", "[GROUP:FAIL]", "[GROUP:PASS]")


def _norm(path: str) -> str:
    """Normalise a path for reliable comparison on Windows.

    Uses os.path.normcase so that drive-letter case differences and mixed
    separators do not cause the watchdog event path to be mistakenly ignored.
    """
    return os.path.normcase(os.path.abspath(path))


class _LogWatcher(FileSystemEventHandler, QThread):
    """Tails the log file in a background QThread and emits new lines on change.

    Extends both FileSystemEventHandler (watchdog) and QThread (Qt) so that
    the watchdog Observer can run on a background thread while new line data
    is delivered to the GUI thread via a Qt signal.

    Attributes:
        new_lines: Emitted with a list of raw log lines whenever new data is
                   appended to the watched file.
    """

    new_lines = pyqtSignal(list)

    def __init__(self, log_path: str) -> None:
        FileSystemEventHandler.__init__(self)
        QThread.__init__(self)
        self._log_path_norm = _norm(log_path)
        self._log_path      = Path(log_path)
        self._offset        = 0           # byte offset for incremental reads
        self._observer      = Observer()

    def run(self) -> None:
        """QThread entry point: read any existing content, then start watching."""
        self._read_new_lines()
        self._observer.schedule(self, str(self._log_path.parent), recursive=False)
        self._observer.start()
        self._observer.join()

    def stop_watching(self) -> None:
        """Stop the watchdog observer (call before QThread.wait())."""
        self._observer.stop()

    def on_created(self, event) -> None:
        """Handle the case where the log file appears after the watcher starts.

        Resets the byte offset to zero so the entire file is read from scratch.
        This can happen if the monitor starts after the GUI.
        """
        if not event.is_directory and _norm(event.src_path) == self._log_path_norm:
            self._offset = 0
            self._read_new_lines()

    def on_modified(self, event) -> None:
        """Handle incremental appends to the log file (the common case)."""
        if not event.is_directory and _norm(event.src_path) == self._log_path_norm:
            self._read_new_lines()

    def _read_new_lines(self) -> None:
        """Read bytes appended since the last read and emit them as lines."""
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
    """Read-only log panel that displays fault state-change lines in colour.

    Watches the log file in a background thread (via _LogWatcher) and appends
    new lines to a QTextEdit whenever the monitor writes them.  Lines that do
    not contain a recognised fault tag are filtered out — only FAULT/GROUP
    state-change entries appear.

    Colour scheme:
        - ``[FAULT:FAIL]`` / ``[GROUP:FAIL]`` → red  (#e53935)
        - ``[FAULT:PASS]`` / ``[GROUP:PASS]`` → green (#43a047)
    """

    _COLOURS = {
        "[FAULT:FAIL]": QColor("#e53935"),
        "[GROUP:FAIL]": QColor("#e53935"),
        "[FAULT:PASS]": QColor("#43a047"),
        "[GROUP:PASS]": QColor("#43a047"),
    }

    def __init__(self, log_path: str, parent=None) -> None:
        """
        Args:
            log_path: Absolute or relative path to fault_monitor.log.
            parent:   Optional Qt parent widget.
        """
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
        """Start the background file-watcher thread."""
        self._watcher.start()

    def stop_watching(self) -> None:
        """Stop the watcher and wait for the background thread to finish."""
        self._watcher.stop_watching()
        self._watcher.wait()

    def _append_lines(self, lines: List[str]) -> None:
        """Filter and append lines to the text widget on the GUI thread.

        Only lines containing a tag from ``_SHOW_TAGS`` are displayed.
        Each line is coloured based on the first matching tag.
        """
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
