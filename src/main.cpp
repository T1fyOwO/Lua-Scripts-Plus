#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "LuaEngine.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
	lsp::init();
	if (Mod::get()->getSettingValue<bool>("autorun")) {
		lsp::runAll();
	}
	lsp::fire("load");
}

class $modify(LSPMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;
		lsp::fire("menu");
		return true;
	}
};

class $modify(LSPBaseGameLayer, GJBaseGameLayer) {
	void handleButton(bool down, int button, bool isPlayer1) {
		GJBaseGameLayer::handleButton(down, button, isPlayer1);
		if (down) lsp::fire("click");
	}
};

class $modify(LSPPlayLayer, PlayLayer) {
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
		lsp::fire("level_enter");
		return true;
	}

	void postUpdate(float dt) {
		PlayLayer::postUpdate(dt);
		lsp::fire("update");
	}

	void onQuit() {
		lsp::fire("level_exit");
		PlayLayer::onQuit();
	}
};
