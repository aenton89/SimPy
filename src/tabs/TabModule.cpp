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