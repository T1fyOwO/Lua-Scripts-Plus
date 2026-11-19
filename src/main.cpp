#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <imgui-cocos.hpp>
#include "LuaEngine.hpp"

using namespace geode::prelude;

namespace {
	bool g_showUI = false;
	std::vector<std::filesystem::path> g_scripts;
	std::string g_status = "Ready.";

	void refreshScripts() {
		g_scripts = lsp::listScripts();
	}

	// Lua touches the scene graph, so never run it from inside the ImGui draw
	// callback: queue it for the start of the next frame instead.
	void runLater(std::function<void()> fn) {
		Loader::get()->queueInMainThread(std::move(fn));
	}

	void drawUI() {
		if (!g_showUI) return;

		ImGui::SetNextWindowSize({380.f, 320.f}, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Lua Scripts+", &g_showUI)) {
			bool autorun = Mod::get()->getSettingValue<bool>("autorun");
			ImGui::TextDisabled("Run on startup: %s", autorun ? "on" : "off");
			ImGui::Separator();

			if (ImGui::Button("Run all")) {
				runLater([] {
					lsp::runAll();
					g_status = "Ran all scripts.";
				});
			}
			ImGui::SameLine();
			if (ImGui::Button("Refresh")) refreshScripts();
			ImGui::SameLine();
			if (ImGui::Button("Reset Lua")) {
				runLater([] {
					lsp::init();
					g_status = "Lua state reset (handlers cleared).";
				});
			}
			ImGui::SameLine();
			if (ImGui::Button("Open folder")) {
				file::openFolder(lsp::scriptsDir().string());
			}

			ImGui::Separator();
			if (g_scripts.empty()) {
				ImGui::TextWrapped("No scripts found. Put .lua files in:\n%s",
					lsp::scriptsDir().string().c_str());
			}
			for (auto const& path : g_scripts) {
				auto name = path.filename().string();
				ImGui::PushID(name.c_str());
				if (ImGui::Button("Run")) {
					runLater([path, name] {
						g_status = lsp::runFile(path)
							? "Ran " + name
							: "Error in " + name + " (see Geode log)";
					});
				}
				ImGui::SameLine();
				ImGui::TextUnformatted(name.c_str());
				ImGui::PopID();
			}

			ImGui::Separator();
			ImGui::TextWrapped("%s", g_status.c_str());
		}
		ImGui::End();
	}
}

$on_mod(Loaded) {
	lsp::init();
	refreshScripts();
	if (Mod::get()->getSettingValue<bool>("autorun")) {
		lsp::runAll();
	}
	lsp::fire("load");

	ImGuiCocos::get().setup([] {
		// Open Sans for the script runner window (falls back to ImGui's default font)
		std::filesystem::path dir = Mod::get()->getResourcesDir().toStdPath();
		for (auto const& p : { dir / "OpenSans.ttf", dir / "resources" / "OpenSans.ttf" }) {
			std::error_code ec;
			if (!std::filesystem::exists(p, ec)) continue;
			auto& io = ImGui::GetIO();
			if (auto* font = io.Fonts->AddFontFromFileTTF(p.string().c_str(), 18.f)) {
				io.FontDefault = font;
			}
			break;
		}
	}).draw([] { drawUI(); });
}

class $modify(LSPMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		// logo button that opens the script runner
		auto spr = CCSprite::create("lsp-button.png"_spr);
		if (spr) {
			spr->setScale(0.3f);
			auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(LSPMenuLayer::onLSP));
			auto menu = CCMenu::create();
			menu->setID("lsp-menu");
			menu->setPosition(CCPointZero);
			btn->setPosition({32.f, CCDirector::get()->getWinSize().height - 32.f});
			menu->addChild(btn);
			this->addChild(menu);
		}

		lsp::fire("menu");
		return true;
	}

	void onLSP(CCObject*) {
		g_showUI = !g_showUI;
		if (g_showUI) refreshScripts();
	}
};

class $modify(LSPBaseGameLayer, GJBaseGameLayer) {
	void handleButton(bool down, int button, bool isPlayer1) {
		GJBaseGameLayer::handleButton(down, button, isPlayer1);
		if (down) lsp::fire("click");
	}
};

class $modify(LSPPlayLayer, PlayLayer) {
	bool init(GJGameLevel* level, bool dontCreateObjects) {
		if (!PlayLayer::init(level, dontCreateObjects)) return false;
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
