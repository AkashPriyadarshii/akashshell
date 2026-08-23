# Changelog - akashshell

## [v0.1.1] - August 2026 - The "Elite" GUI Update
### Added
- **Direct2D Rendering Engine**: Replaced standard Win32 `EDIT` controls with a fully custom, GPU-accelerated Direct2D canvas.
- **DirectWrite Typography**: Integrated `Cascadia Code` with sub-pixel anti-aliasing via `IDWriteTextFormat`.
- **Warp-Style Block UI**: Transformed the terminal interface into distinct, visual command blocks (rounded rectangles mapping exact command and stdout).
- **Asynchronous Output Streaming**: `ExecThread` now buffers stdout into memory and signals `WM_USER_OUTPUT_UPDATE` to trigger native 60fps repaints, completely eliminating UI freezing during heavy IO operations.
- **DWM Integration**: Enforced Immersive Dark Mode and Mica materials on the root window.
- **History Serialization**: Added `load_history` and `save_history` to seamlessly read/write to `%USERPROFILE%\.akashshell_history` across sessions.

## [v0.1.0] - August 2026 - CLI Foundation
### Added
- Built the foundational pipe engine (`DuplicateHandle` chain).
- Handled built-ins (`cd`, `exit`).
- Wrapped executable resolution through `cmd.exe /c` to support native DOS built-ins (like `echo` and `dir`).
