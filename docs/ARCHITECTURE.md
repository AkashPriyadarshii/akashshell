# Architecture - akashshell v0.2

## The Pure C Paradigm
`akashshell` achieves elite performance by bypassing standard abstractions and managing its own UI state, network stack, and execution loops entirely in C.

## Core Systems
1. **The UI Node Tree (Direct2D):**
   - A structural tree of `UI_Node` structs.
   - Calculates flex-box style layouts mathematically before issuing `ID2D1HwndRenderTarget` draw commands.
   - Interpolates animations (expanding blocks, fading) via `QueryPerformanceCounter`.

2. **The Gap-Buffer Input:**
   - Replaces the Win32 `EDIT` control.
   - Handles `WM_CHAR` and `WM_KEYDOWN` natively.
   - Parses tokens in real-time for syntax highlighting.

3. **The Data Pipeline:**
   - Uses embedded `sqlite3.c` as the central state store.
   - Output from `CreateProcess` stdout pipes is parsed by `cJSON` and stored as structured data.

