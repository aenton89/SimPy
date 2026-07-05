//
// Created by tajbe on 30.12.2025.
//
#include "TabManager.h"
#include <imgui_impl_opengl3.h>
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../gui/BluePrintTab.h"
#include "../ui/UIStyles.h"
#include "../ide/NotebookTab.h"



// TODO: narazie po prostu na sztywno dodajemy blueprintTab jako domyślną zakładkę
TabManager::TabManager() {
	tabs.push_back(std::make_unique<BluePrintTab>());
	currentTab = 0;
	tabs[0]->isActive = true;
	tabs[0]->setTabManager(this);
	workspace = new workSpace();
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
	UIStyles::applyDarkStyle();
}

void TabManager::drawWorkspace() {
    viewport = ImGui::GetMainViewport();

    if (workspace == nullptr) {
        std::cout << "nullptr" << std::endl;
        return;
    }

    float menuBarHeight = ImGui::GetFrameHeight();
    float topBarHeight  = 50.0f;

    float startY = viewport->Pos.y + menuBarHeight + topBarHeight;

    float availableHeight = viewport->Size.y - (menuBarHeight + topBarHeight);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar;

    // sIDEBAR
    float sidebarWidth = 40.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, startY));
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth, availableHeight));
    ImGui::SetNextWindowBgAlpha(0.8f);

    if (ImGui::Begin("Sidebar", nullptr, window_flags)) {
        ImVec2 buttonSize(20, 20);
        float xCenter = (ImGui::GetWindowSize().x - buttonSize.x) / 2.0f;
        float yCenter = 10.0f;

        ImGui::SetCursorPos(ImVec2(xCenter, yCenter));

        if (ImGui::Button("F", buttonSize)) {
            showWorkspace = !showWorkspace;
        }
    }
    ImGui::End();

    // WORKSPACE
    if (showWorkspace && workspace) {
        ImVec2 wsPos(viewport->Pos.x + sidebarWidth, startY);

        ImVec2 wsSize(250.0f, availableHeight);

        workspace->Render(wsPos, wsSize);
    }
}

ActiveArea TabManager::getActiveArea() {
	ImGuiViewport* vp = ImGui::GetMainViewport();
	float menuBarHeight = ImGui::GetFrameHeight();
	float topBarHeight = 50.0f;

	float offsetX = 40.0f;
	if (showWorkspace) {
		offsetX += 250.0f;
	}

	ActiveArea area;
	area.pos = ImVec2(offsetX, menuBarHeight + topBarHeight);
	area.size = ImVec2(vp->Size.x - offsetX, vp->Size.y - (menuBarHeight + topBarHeight));
	return area;
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
// + dodający zakładkę (narazie domyślnie otwiera tylko blueprintTab)
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
			ImGui::PushStyleColor(ImGuiCol_Button, UIStyles::SELECTION_COLOR);

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

	const char* popupName = "AddTabMenu";

	if (ImGui::Button("+")) {
		ImGui::OpenPopup(popupName);
	}

	if (ImGui::BeginPopup(popupName)) {
		if (ImGui::Selectable("Blueprints")) {
			openTab<BluePrintTab>();
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("CodeEditor")) {
			openTab<NotebookTab>();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::EndMainMenuBar();

	//std::cout << "debug" << std::endl;

	drawWorkspace();
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
