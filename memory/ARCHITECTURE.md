# Architecture Notes

- `CreateProcess` over `system()` — gives real pipe control, no shell-interpret
- Handle inheritance via `SetHandleInformation` + `STARTUPINFO.cb=sizeof`; `STARTF_USESTDHANDLES`
- History array: `static char history[MAXHIST][256]`, ring not needed yet
- `parse_pipe`: naive split on `|`, whitespace-trim each segment
