//
// Created by tajbe on 30.12.2025.
//
#include "TabManager.h"
#include <imgui_impl_opengl3.h>
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../gui/GUICore.h"



// TODO: narazie po prostu na sztywno dodajemy GUICore jako domyślną zakładkę
TabManager::TabManager() {
	tabs.push_back(std::make_unique<GUICore>());
	currentTab = 0;
	tabs[0]->isActive = true;
	tabs[0]->setTabManager(this);
}

// initialization of evertything regarding ImGui
void TabManager::init(GLFWwindow* win, const char* version) {
	window = win;
	glsl_version = version;

	// jakaś defaultowa ikonka, potem zmienie
	GLFWimage images[1];
	std::string iconPath = std::string(ASSETS_DIR) + "/app_icons/icon_v3.png";
	images[0].pixels = stbi_load(iconPath.c_str(), &images[0].width, &images[0].height, nullptr, 4);
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);
	if (!images[0].pixels) {
		std::cerr << "ERROR: couldn't load icon.png!" << std::endl;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImPlot::CreateContext();

	// setup platform/ renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// optional: setup style and custom colors
	ImGui::StyleColorsClassic();
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 gray = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

	style.Colors[ImGuiCol_WindowBg] = gray;
	style.Colors[ImGuiCol_ChildBg] = gray;
	style.Colors[ImGuiCol_PopupBg]= gray;
}

void TabManager::updateOpen() {
	for (auto& tab : tabs) {
		if (tab->isActive) {
			tab->update();
			break;
		}
	}
}

// TODO: zmienia zakładkę na wybraną
void TabManager::changeTab(int index) {
	if (index >= 0 && index < tabs.size()) {
		// dezaktywuj poprzednią zakładkę
		if (currentTab >= 0 && currentTab < tabs.size())
			tabs[currentTab]->isActive = false;

		// aktywuj nową zakładkę
		currentTab = index;
		tabs[currentTab]->isActive = true;
	}
}

// TODO: czy to ma korzystać z int index? czy coś innego będzie lepsze?
void TabManager::closeTab(int index) {
	if (index >= 0 && index < tabs.size()) {
		tabs[index]->isActive = false;
		tabs.erase(tabs.begin() + index);

		// jeśli zamknięto aktywną zakładkę, ustaw nową aktywną
		if (index == currentTab) {
			if (!tabs.empty()) {
				currentTab = std::min(index, static_cast<int>(tabs.size()) - 1);
				if (currentTab >= 0)
					tabs[currentTab]->isActive = true;

			} else {
				currentTab = -1;
			}
		} else if (index < currentTab) {
			currentTab--;
		}
	}
}

// TODO: zmienić, żeby jako argument (? template ?) podać typ zakładki
template<typename T>
void TabManager::openTab() {
	auto newTab = std::make_unique<T>();
	newTab->setTabManager(this);
	tabs.push_back(std::move(newTab));
	changeTab(tabs.size() - 1);
}

// TODO: tutaj można UI menadżera zakładek - pasek wyboru zakładek itp.
// pasek wyrobu zakładek
// + dodający zakładkę (narazie domyślnie otwiera tylko GUICore)
// x zamykający zakładkę
void TabManager::update() {
	updateOpen();

	// pasek zakładek na górze
	ImGui::BeginMainMenuBar();

	// wyświetl wszystkie zakładki
	for (int i = 0; i < tabs.size(); i++) {
		ImGui::PushID(i);

		bool isActive = (i == currentTab);
		if (isActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));

		if (ImGui::Button(tabs[i]->title.c_str()))
			changeTab(i);

		if (isActive)
			ImGui::PopStyleColor();

		ImGui::SameLine();

		// przycisk zamknięcia (X)
		if (tabs.size() > 1) {
			// nie pozwól zamknąć ostatniej zakładki
			if (ImGui::SmallButton("X"))
				closeTab(i);

			ImGui::SameLine();
		}

		ImGui::PopID();
	}

	// przycisk dodawania nowej zakładki
	if (ImGui::Button("+"))
		openTab();

	ImGui::EndMainMenuBar();
}

void TabManager::newFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void TabManager::render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void TabManager::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	ImPlot::DestroyContext();
}
