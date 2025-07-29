//
// Created by patryk on 23.07.25.
//

#include <iostream>
#include "CodeCell.h"
#include <PythonKernel.h>

#include "CodeCell.h"
#include <cstring>

Cell::Cell(PythonKernel& kernel) : notebook(&kernel) {
    this->text[0] = '\0';
    this->output_text[0] = '\0';
}

Cell::Cell(Cell&& other) noexcept
    : notebook(other.notebook) {
    std::strcpy(this->text, other.text);
    std::strcpy(this->output_text, other.output_text);

    other.text[0] = '\0';
    other.output_text[0] = '\0';
}

Cell& Cell::operator=(Cell&& other) noexcept {
    if (this != &other) {
        notebook = other.notebook;
        std::strcpy(this->text, other.text);
        std::strcpy(this->output_text, other.output_text);

        other.text[0] = '\0';
        other.output_text[0] = '\0';
    }
    return *this;
}

void Cell::setKernel(PythonKernel* kernel) {
    notebook = kernel;
}

bool Cell::Draw(int id) {
    bool requestDelete = false;

    ImVec2 screenSize = ImGui::GetWindowSize();
    ImGui::PushID(id);

    bool focused = ImGui::InputTextMultiline(
        "Input Cell", this->text, sizeof(this->text),
        ImVec2(screenSize.x * 5 / 6, screenSize.y / 4),
        ImGuiInputTextFlags_AllowTabInput);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(13, 12, 18, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
    ImGui::InputTextMultiline(
        "Output Cell", this->output_text, sizeof(this->output_text),
        ImVec2(screenSize.x * 5 / 6, screenSize.y / 6),
        ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(2);

    if (ImGui::Button("Exec") ||
       (focused && ImGui::IsKeyDown(ImGuiKey_RightShift) && ImGui::IsKeyPressed(ImGuiKey_Enter)))
    {
        std::string result = notebook->executeCode(this->text);
        std::strcpy(this->output_text, result.c_str());
    }

    ImGui::SameLine();
    if (ImGui::Button("Del Cell")) {
        requestDelete = true;
    }

    ImGui::PopID();
    return requestDelete;
}

void Cell::setCodeInput(std::string input)
{
    strcpy(this->text, input.c_str());
}

void Cell::setCodeOutput(std::string output)
{
    strcpy(this->output_text, output.c_str());
}

std::string Cell::getCodeInput() const
{
    return this->text;
}

std::string Cell::getCodeOutput() const
{
    return this->output_text;
}


