//
// Created by patryk on 23.07.25.
//

#include <cstring>
#include <imgui_markdown.h>
#include <imgui.h>
#include <string>
#include "MarkDownCell.h"
#include <map>

#include "CodeCell.h"

MarkDownCell::MarkDownCell()
{
    memset(this->text, 0, sizeof(this->text));
}

// ---- Globalne czcionki i konfiguracja ----
static ImFont* fontDefault = nullptr;
static ImFont* fontH1 = nullptr;
static ImFont* fontH2 = nullptr;
static ImFont* fontH3 = nullptr;

static ImGui::MarkdownConfig mdConfig;

void SetupMarkdownFonts(ImGuiIO& io)
{
    fontDefault = io.Fonts->AddFontDefault();

    fontH1 = io.Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 26.0f);
    fontH2 = io.Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 22.0f);
    fontH3 = io.Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 20.0f);

    mdConfig.linkCallback = [](ImGui::MarkdownLinkCallbackData data)
    {
        printf("Kliknięto link: %s\n", data.link);
    };
    mdConfig.tooltipCallback = nullptr;
    mdConfig.imageCallback = nullptr;

    mdConfig.headingFormats[0] = { fontH1, true };   // # H1 pogrubiony
    mdConfig.headingFormats[1] = { fontH2, true };   // ## H2 pogrubiony
    mdConfig.headingFormats[2] = { fontH3, false };  // ### H3 zwykły

    io.FontGlobalScale = 1.0f; // skalowanie globalne
}

bool MarkDownCell::Draw(int id)
{
    bool requestDelete = false;

    // Użyj mapy previewModes, aby przechowywać stan podglądu dla każdego obiektu
    static std::map<int, bool> previewModes;

    // Jeśli nie ma tego obiektu w mapie, dodaj go
    if (previewModes.find(id) == previewModes.end())
    {
        previewModes[id] = false;  // Ustawienie początkowego stanu
    }

    ImVec2 screenSize = ImGui::GetWindowSize();
    ImGui::PushID(id);

    bool focused = false;

    // Użyj previewModes[id] zamiast previewMode
    if (!previewModes[id])
    {
        std::string markdownId = "Markdown Cell " + std::to_string(id + 1);
        focused = ImGui::InputTextMultiline(markdownId.c_str(), this->text, sizeof(this->text),
                                            ImVec2(screenSize.x * 5 / 6, screenSize.y / 4),
                                            ImGuiInputTextFlags_AllowTabInput);
    }
    else
    {
        ImGui::Markdown(this->text, strlen(this->text), mdConfig);
    }

    // Przełącznik trybu
    if (ImGui::Button("Toggle Preview (Ctrl+Enter)"))
        previewModes[id] = !previewModes[id];  // Zmiana stanu dla konkretnego obiektu

    ImGui::SameLine();
    if (ImGui::Button("Delete"))
        requestDelete = true;

    // Sprawdzenie kombinacji Shift + Enter, gdy pole jest aktywne
    if (ImGui::IsKeyDown(ImGuiKey_RightShift) && ImGui::IsKeyPressed(ImGuiKey_Enter) && focused)
    {
        previewModes[id] = !previewModes[id];  // Zmiana stanu przy kombinacji Shift + Enter
    }

    ImGui::PopID();
    return requestDelete;
}

void MarkDownCell::setInput(std::string input)
{
    std::strcpy(this->text, input.c_str());
}

void MarkDownCell::execMardown()
{
    ImGui::Markdown(this->text, strlen(this->text), mdConfig);
}

std::string MarkDownCell::getInput() const
{
    return this->text;
}


