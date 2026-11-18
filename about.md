# Lua Scripts+

Inject and run your own Lua scripts inside Geometry Dash.

## Features

- Runs every `.lua` file in the mod's `scripts` folder.
- Optional **Run scripts on startup** setting (off by default).
- Small `lsp` API available to your scripts:
  - `lsp.log(...)` and `print(...)` write to the Geode log
  - `lsp.on(event, fn)` registers a handler for `load`, `menu`, `level_enter` or `level_exit`
  - `lsp.notify(msg)` shows an in-game notification
  - `lsp.scriptsDir()` returns the scripts folder path
  - `lsp.version` is the mod version

## Example

```lua
lsp.on("menu", function()
    lsp.notify("Hello from Lua!")
end)
```
