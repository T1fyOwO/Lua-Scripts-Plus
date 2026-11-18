#pragma once
#include <Geode/Geode.hpp>
#include <string>

struct lua_State;

namespace lsp {
	// Starts (or restarts) the Lua state and registers the `lsp` API.
	void init();
	// Runs every .lua file in the scripts folder.
	void runAll();
	// Runs a single file; returns false and logs on error.
	bool runFile(std::filesystem::path const& path);
	// Fires a named event to all handlers registered with lsp.on(name, fn).
	void fire(std::string const& event);
	std::filesystem::path scriptsDir();
}
