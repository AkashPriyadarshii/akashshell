# DESIGN — akashshell v0.1

## Architecture
```
main()
  init_history(): load .akashshell_history
  loop:
    prompt("akashshell> ")
    fgets buffer
    trim_nl
    empty? continue
    builtins? handle, continue
    parse_pipe(buffer) → tokens[]
    spawn_chain(tokens) → CreateProcess loop w/ pipe handles
    wait(last_pid)
    add_history(buf)
  save_history()
```

## Modules (single file)
- `init_history` / `add_history` / `save_history` — `fgets`/`fprintf` to file
- `parse_pipe` — split on `|`, each part `strtok` on whitespace → args
- `spawn_chain` — walk cmds, create pipe per adj pair, `CreateProcess` with
  `STARTUPINFO.std_{in,out}`, inherit handles
- `builtins` — `cd` (`SetCurrentDirectory`), `exit`

## Dependencies
None beyond Win32 API (`kernel32.dll`) + C stdlib.

## File layout
Single file: `shell.c` (~150 lines).

## Error handling
Bad command: print `akashshell: error: <msg>` to stderr, loop continues. Never crash.
