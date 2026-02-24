//
// Created by tajbe on 07.11.2025.
//
#include "ViewportManager.h"
#include "../../GUICore.h"
#include "../../../../ui/UIStyles.h"



void ViewportManager::zoomAndPanning() {
	ImGuiIO& io = ImGui::GetIO();

	// obsługa zoom'u względem pozycji myszki
	if (io.MouseWheel != 0.0f && !canvasDragging && !guiCore->blocksManager.isDraggingWindow) {
		float zoom_speed = 0.1f;
		float old_zoom = zoomAmount;

		// ogranicz max scroll/frame
		float wheel = std::clamp(io.MouseWheel, -3.0f, 3.0f);
		zoomAmount += wheel * zoom_speed;
		zoomAmount = std::clamp(zoomAmount, 0.1f, 10.0f);

		if (zoomAmount != old_zoom) {
			ImVec2 mouse_pos = io.MousePos;
			viewOffset.x = mouse_pos.x - ((mouse_pos.x - viewOffset.x) / old_zoom) * zoomAmount;
			viewOffset.y = mouse_pos.y - ((mouse_pos.y - viewOffset.y) / old_zoom) * zoomAmount;
		}
	}

	// rozpoczęcie przeciągania canvas'u - pod (TYLKO) środkowym przyciskiem
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && !ImGui::IsAnyItemActive() && !guiCore->blocksManager.isDraggingWindow && !canvasDragging) {
		canvasDragging = true;
		dragStartPos = io.MousePos;
		dragStartOffset = viewOffset;
	}

	// kontynuacja przeciągania canvas'u
	if (canvasDragging) {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
			ImVec2 delta = ImVec2(io.MousePos.x - dragStartPos.x, io.MousePos.y - dragStartPos.y);
			viewOffset = ImVec2(dragStartOffset.x + delta.x, dragStartOffset.y + delta.y);
		} else {
			// środkowy przycisk został puszczony
			canvasDragging = false;
		}
	}

	// jeśli przeciągamy okno, zatrzymaj przeciąganie canvas'u
	if (guiCore->blocksManager.isDraggingWindow && canvasDragging)
		canvasDragging = false;

	// żeby uniknąć artefaktów renderowania
	viewOffset.x = std::round(viewOffset.x);
	viewOffset.y = std::round(viewOffset.y);
}

void ViewportManager::drawGrid() const {
    if (!guiCore->uiPreferences.gridEnabled)
        return;

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 canvas_pos = ImVec2(0, 20);
    ImVec2 canvas_size = ImVec2(io.DisplaySize.x, io.DisplaySize.y - 20);

    // calculate world space bounds visible on screen
    ImVec2 world_min = ImVec2(
        (canvas_pos.x - guiCore->viewportManager.viewOffset.x) / guiCore->viewportManager.zoomAmount,
        (canvas_pos.y - guiCore->viewportManager.viewOffset.y) / guiCore->viewportManager.zoomAmount
    );
    ImVec2 world_max = ImVec2(
        (canvas_pos.x + canvas_size.x - guiCore->viewportManager.viewOffset.x) / guiCore->viewportManager.zoomAmount,
        (canvas_pos.y + canvas_size.y - guiCore->viewportManager.viewOffset.y) / guiCore->viewportManager.zoomAmount
    );

    // use fixed grid spacing in world coordinates (like Blender [yupii blender mentioned])
    float world_spacing = guiCore->uiPreferences.gridSpacing;

    // calculate grid start positions
    float start_x = floor(world_min.x / world_spacing) * world_spacing;
    float start_y = floor(world_min.y / world_spacing) * world_spacing;

    // make grid lines fade out when too close or far
    float screen_spacing = world_spacing * guiCore->viewportManager.zoomAmount;
    float alpha_factor = 1.0f;

    // fade out when too dense
    if (screen_spacing < 10.0f) {
        alpha_factor = screen_spacing / 10.0f;
    }
    // fade out when too far
    else if (screen_spacing > 200.0f) {
        alpha_factor = 200.0f / screen_spacing;
    }

    // apply alpha to grid color
    ImU32 faded_color = (guiCore->uiPreferences.gridColor & 0x00FFFFFF) | (static_cast<int>(((guiCore->uiPreferences.gridColor >> 24) & 0xFF) * alpha_factor) << 24);

    // skip drawing if too faded
    if (alpha_factor < 0.1f)
        return;

    // vertical lines
    for (float x = start_x; x <= world_max.x + world_spacing; x += world_spacing) {
        float screen_x = x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x;

        if (screen_x < canvas_pos.x - 1 || screen_x > canvas_pos.x + canvas_size.x + 1)
            continue;

        draw_list->AddLine(
            ImVec2(screen_x, canvas_pos.y),
            ImVec2(screen_x, canvas_pos.y + canvas_size.y),
            faded_color,
            guiCore->uiPreferences.gridThickness
        );
    }

    // horizontal lines
    for (float y = start_y; y <= world_max.y + world_spacing; y += world_spacing) {
        float screen_y = y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y;

        if (screen_y < canvas_pos.y - 1 || screen_y > canvas_pos.y + canvas_size.y + 1)
            continue;

        draw_list->AddLine(
            ImVec2(canvas_pos.x, screen_y),
            ImVec2(canvas_pos.x + canvas_size.x, screen_y),
            faded_color,
            guiCore->uiPreferences.gridThickness
        );
    }
}

void ViewportManager::lightMode() const {
    if (guiCore->uiPreferences.lightMode)
        UIStyles::applyLightStyle();
    else
        UIStyles::applyDarkStyle();
}

// włączanie i wyłączanie light mode przez CTRL+L
void ViewportManager::turnLightModeOnShortcut(const ImGuiIO &io) const {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_L, false))
		guiCore->uiPreferences.lightMode = !guiCore->uiPreferences.lightMode;
}

// włączanie i wyłączanie siatki przez CTRL+G
void ViewportManager::turnGridOnShortcut(const ImGuiIO &io) const {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_G, false))
		guiCore->uiPreferences.gridEnabled = !guiCore->uiPreferences.gridEnabled;
}
