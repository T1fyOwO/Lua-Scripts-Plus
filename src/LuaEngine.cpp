#include "LuaEngine.hpp"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <chrono>
#include <map>
#include <vector>
#include <fstream>

using namespace geode::prelude;

namespace {
	lua_State* L = nullptr;
	// event name -> registry refs of handler functions
	std::map<std::string, std::vector<int>> g_handlers;

	int l_log(lua_State* s) {
		int n = lua_gettop(s);
		std::string out;
		for (int i = 1; i <= n; i++) {
			size_t len;
			char const* str = luaL_tolstring(s, i, &len);
			if (i > 1) out += "\t";
			out.append(str, len);
			lua_pop(s, 1);
		}
		log::info("[lua] {}", out);
		return 0;
	}

	int l_on(lua_State* s) {
		char const* name = luaL_checkstring(s, 1);
		luaL_checktype(s, 2, LUA_TFUNCTION);
		lua_pushvalue(s, 2);
		int ref = luaL_ref(s, LUA_REGISTRYINDEX);
		g_handlers[name].push_back(ref);
		return 0;
	}

	int l_scriptsDir(lua_State* s) {
		lua_pushstring(s, lsp::scriptsDir().string().c_str());
		return 1;
	}

	int l_notify(lua_State* s) {
		char const* msg = luaL_checkstring(s, 1);
		Notification::create(msg, NotificationIcon::None, 2.f)->show();
		return 0;
	}

	// lsp.time() -> seconds (monotonic, fractional)
	int l_time(lua_State* s) {
		auto now = std::chrono::steady_clock::now().time_since_epoch();
		lua_pushnumber(s, std::chrono::duration<double>(now).count());
		return 1;
	}

	// lsp.setLabel(text) -> shows text in the corner of the level; "" hides it
	int l_setLabel(lua_State* s) {
		char const* text = luaL_checkstring(s, 1);
		auto* pl = PlayLayer::get();
		if (!pl) return 0;
		auto* label = static_cast<CCLabelBMFont*>(pl->getChildByID("lsp-label"));
		if (!label) {
			label = CCLabelBMFont::create("", "bigFont.fnt");
			label->setID("lsp-label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 1.f});
			label->setOpacity(180);
			label->setZOrder(1000);
			auto win = CCDirector::get()->getWinSize();
			label->setPosition({8.f, win.height - 8.f});
			pl->addChild(label);
		}
		label->setString(text);
		label->setVisible(text[0] != '\0');
		return 0;
	}

	void registerApi() {
		lua_newtable(L);
		lua_pushcfunction(L, l_log);        lua_setfield(L, -2, "log");
		lua_pushcfunction(L, l_on);         lua_setfield(L, -2, "on");
		lua_pushcfunction(L, l_scriptsDir); lua_setfield(L, -2, "scriptsDir");
		lua_pushcfunction(L, l_notify);     lua_setfield(L, -2, "notify");
		lua_pushcfunction(L, l_time);       lua_setfield(L, -2, "time");
		lua_pushcfunction(L, l_setLabel);   lua_setfield(L, -2, "setLabel");
		lua_pushstring(L, "1.0.0");         lua_setfield(L, -2, "version");
		lua_setglobal(L, "lsp");
		// make print go to the Geode log too
		lua_pushcfunction(L, l_log);
		lua_setglobal(L, "print");
	}
}

namespace lsp {
	std::filesystem::path scriptsDir() {
		std::filesystem::path dir = (Mod::get()->getConfigDir() / "scripts").toStdPath();
		std::error_code ec;
		std::filesystem::create_directories(dir, ec);
		return dir;
	}

	void init() {
		if (L) {
			lua_close(L);
			g_handlers.clear();
		}
		L = luaL_newstate();
		luaL_openlibs(L);
		registerApi();
	}

	bool runFile(std::filesystem::path const& path) {
		if (!L) init();
		if (luaL_dofile(L, path.string().c_str()) != LUA_OK) {
			log::error("[lua] {}: {}", path.filename().string(), lua_tostring(L, -1));
			lua_pop(L, 1);
			return false;
		}
		return true;
	}

	std::vector<std::filesystem::path> listScripts() {
		std::vector<std::filesystem::path> files;
		std::error_code ec;
		for (auto const& e : std::filesystem::directory_iterator(scriptsDir(), ec)) {
			if (e.is_regular_file() && e.path().extension() == ".lua") {
				files.push_back(e.path());
			}
		}
		std::sort(files.begin(), files.end());
		return files;
	}

	void runAll() {
		for (auto const& f : listScripts()) {
			log::info("Running script {}", f.filename().string());
			runFile(f);
		}
	}

	void fire(std::string const& event) {
		if (!L) return;
		auto it = g_handlers.find(event);
		if (it == g_handlers.end()) return;
		for (int ref : it->second) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
				log::error("[lua] handler for '{}' failed: {}", event, lua_tostring(L, -1));
				lua_pop(L, 1);
			}
		}
	}
}
