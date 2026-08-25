# AKASHSHELL — UX & DESIGN GUIDELINES

## 1. THE VIBE
`akashshell` must feel like elite hacker software from 2026. It is fast, silent when successful, and highly communicative when failing. It respects the user's time.

## 2. ZERO LATENCY RULE
*   The prompt must **never** block.
*   If a command takes more than 50ms, the shell UI must remain responsive (cursor blinks, interruptible via Ctrl+C).
*   UI thread and execution thread are strictly separated.

## 3. INTELLIGENT DEFAULTS
*   **Syntax Highlighting:** Valid commands are green, invalid commands are red. Strings are yellow. Flags are cyan. This happens *while typing*.
*   **Auto-Suggestions:** Faded gray text appears ahead of the cursor based on history (Fish-style). Pressing `Right Arrow` accepts the suggestion.
*   **Smart Prompts:** The prompt line shows git branch, current directory, and previous command exit status (red `X` if failed).

## 4. STRUCTURAL OUTPUT VISUALIZATION
*   When a pipeline returns a structured `Value` (like a Record or List), it should be natively rendered as a beautifully formatted table.
*   Borders should use clean, modern Unicode box-drawing characters.
*   Colors should be used to denote data types (Numbers = Magenta, Booleans = Blue, Strings = Default).
