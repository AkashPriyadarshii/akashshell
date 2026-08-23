# Changelog

## [0.1.0] - 2026-08-23
### Added
- Core CLI shell engine (`cd`, `exit`, `|` pipes, history).
- Pipe inheritance fixed using `DuplicateHandle`.
- Modern Windows 11 GUI client (`gui.c`) featuring:
  - DWM Immersive Dark Mode and Mica background.
  - Asynchronous background worker thread for long-running pipelines.
  - Up/Down arrow key command history navigation.
  - Real-time `stdout`/`stderr` streaming to the GUI output canvas.
