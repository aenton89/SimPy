//
// Created by tajbe on 30.12.2025.
//
#include "TabModule.h"
#include "TabManager.h"



TabModule::TabModule(const std::string& tabTitle) : title(tabTitle) {}

void TabModule::setTabManager(TabManager *manager) {
	tabManager = manager;
}
void TabModule::update() {
	drawMenuBar();
	drawTopBar();
}

void TabModule::drawMenuBar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			menuBarFile();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			menuBarEdit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Settings")) {
			menuBarSettings();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Other")) {
			// TODO: usunąć tą linijke później
			if (ImGui::MenuItem("Nothing to see", "Alt+F4")) {/* narazie nic nie ma, placeholder */ }

			menuBarOther();
			ImGui::EndMenu();
		}

		// TODO: narazie takie coś, żeby oddzielić od zawartości TabManager'a
		ImGui::Separator();

		ImGui::EndMainMenuBar();
	}
}

void TabModule::drawTopBar() {
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// Te wartości możesz pobierać dynamicznie lub mieć jako stałe
	float menuBarHeight = ImGui::GetFrameHeight();
	float topBarHeight = 50.0f;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoScrollbar;

	// GÓRNY PASEK
	ImGui::SetNextWindowPos(ImVec2(0, viewport->Pos.y + menuBarHeight));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, topBarHeight));
	//ImGui::SetNextWindowBgAlpha(0.8f);

	if (ImGui::Begin("TopBar", nullptr, window_flags)) {

		drawTopBarContent();

	}
	ImGui::End();
}

ImVec2 TabModule::getPose() {
	if (tabManager) {return tabManager->getActiveArea().pos;}
	return ImVec2(40, 70);
}

ImVec2 TabModule::getSize() {
	if (tabManager) {return tabManager->getActiveArea().size;}
	return ImGui::GetIO().DisplaySize;
}