//
// Created by patryk on 10.02.26.
//

#include "mardownCell.h"
#include <../../include/imgui_markdown.h>

ImGui::MarkdownConfig mdConfig;

MardownCell::MardownCell() {
    markdown_text = "";
    focused = false;
    focusedPrev = false;
    is_preview = false;
}

bool MardownCell::Draw(int id) {
    bool requestDelete = false;
    ImGui::PushID(id);
    ImVec2 screenSize = ImGui::GetWindowSize();
    float width = screenSize.x * 5.0f / 6.0f;

    if (!is_preview) {
        // Tworzymy bufor char na podstawie aktualnego stringa
        char buffer[4096]; // Ustaw odpowiedni limit (np. 4KB)
        strncpy(buffer, markdown_text.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        if (ImGui::InputTextMultiline("##md_edit", buffer, sizeof(buffer),
                                       ImVec2(width, screenSize.y / 6))) {
            // Jeśli użytkownik coś wpisał, aktualizujemy std::string
            markdown_text = buffer;
                                       }
        this->focused = ImGui::IsItemActive();
    } else {
        extern ImGui::MarkdownConfig mdConfig;
        ImGui::Markdown(markdown_text.c_str(), markdown_text.length(), mdConfig);
        this->focused = false;
    }

    // Przycisk przełączania (zmień is_preview)
    if (ImGui::Button(is_preview ? "Edit" : "Preview")) {
        is_preview = !is_preview;
    }

    ImGui::SameLine();
    if (ImGui::Button("Del Cell")) requestDelete = true;

    ImGui::PopID();
    return requestDelete;
}

void MardownCell::setInputText(const std::string& input) {
    this->markdown_text = input;
}

std::string MardownCell::getInputText() const {
    return this->markdown_text;
}



