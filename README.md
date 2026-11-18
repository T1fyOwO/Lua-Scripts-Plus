# Lua Scripts+

A [Geode](https://geode-sdk.org) mod for Geometry Dash that injects and runs your own Lua scripts.

## Usage

Put `.lua` files in the mod's `scripts` folder (inside its Geode config directory).
Scripts run on startup if **Run scripts on startup** is enabled in the mod settings.

See [about.md](about.md) for the scripting API.

## Building

1. Install the [Geode SDK](https://docs.geode-sdk.org/getting-started/) and set the `GEODE_SDK` environment variable.
2. Run `geode build` in the project folder.

Lua 5.4 is fetched automatically through CPM.

