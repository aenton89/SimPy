//
// Created by patryk on 10.02.26.
//

#include "codeCell.h"
#include "../../core/file_manipulation/FileUtils.h"

#include <imgui.h>
#include <regex>
#include <cstring>
#include <iostream>

#include <glad/glad.h>

CodeCell::CodeCell(PythonKernel& kernelRef)
    : kernel(&kernelRef)
{
    output_buffer[0] = '\0';
    inputEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
}

CodeCell::CodeCell(CodeCell&& other) noexcept
    : kernel(other.kernel),
      exec_future(std::move(other.exec_future)),
      is_executing(other.is_executing.load()),
      execution_requested(other.execution_requested),
      texture(other.texture),
      texture_width(other.texture_width),
      texture_height(other.texture_height),
      base64(std::move(other.base64)),
      inputEditor(std::move(other.inputEditor))
{
    std::memcpy(output_buffer, other.output_buffer, sizeof(output_buffer));

    other.texture = 0;
}

CodeCell& CodeCell::operator=(CodeCell&& other) noexcept
{
    if (this != &other)
    {
        kernel = other.kernel;
        exec_future = std::move(other.exec_future);
        is_executing = other.is_executing.load();
        execution_requested = other.execution_requested;

        texture = other.texture;
        texture_width = other.texture_width;
        texture_height = other.texture_height;
        base64 = std::move(other.base64);

        inputEditor = std::move(other.inputEditor);
        std::memcpy(output_buffer, other.output_buffer, sizeof(output_buffer));

        other.texture = 0;
    }
    return *this;
}

bool CodeCell::Draw(int id)
{
    bool requestDelete = false;

    ImGui::PushID(id);

    ImVec2 screenSize = ImGui::GetWindowSize();
    float width = screenSize.x * 5.0f / 6.0f;

    // ===== INPUT =====
    std::string text = inputEditor.GetText();
    int numLines = std::count(text.begin(), text.end(), '\n') + 1;

    float lineHeight = ImGui::GetTextLineHeight();
    float maxHeight = screenSize.y / 4.0f;
    float height = std::max(numLines * lineHeight, maxHeight);

    inputEditor.Render("Input Cell", ImVec2(width, height));

    // ===== BUTTONS =====
    if (ImGui::Button("Exec") && !is_executing)
        execution_requested = true;

    ImGui::SameLine();

    if (ImGui::Button("Del Cell"))
        requestDelete = true;

    if (execution_requested)
    {
        startExecution();
        execution_requested = false;
    }

    if (is_executing)
        ImGui::Text("Executing...");

    // ===== CHECK RESULT =====
    pollExecutionResult();

    // ===== OUTPUT =====
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(13, 12, 18, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));

    ImGui::InputTextMultiline(
        "Output Cell",
        output_buffer,
        sizeof(output_buffer),
        ImVec2(screenSize.x * 5 / 6, screenSize.y / 6),
        ImGuiInputTextFlags_ReadOnly
    );

    ImGui::PopStyleColor(2);

    // ===== IMAGE =====
    if (texture != 0)
    {
        float aspect = (float)texture_width / (float)texture_height;
        float img_width = 512.0f;
        float img_height = img_width / aspect;

        ImGui::Text("Generated plot:");
        ImGui::Image((void*)(intptr_t)texture, ImVec2(img_width, img_height));
    }

    ImGui::PopID();
    return requestDelete;
}

void CodeCell::startExecution()
{
    if (!kernel) return;

    is_executing = true;

    exec_future = std::async(std::launch::async, [this]()
    {
        return kernel->executeCode(inputEditor.GetText());
    });
}

void CodeCell::pollExecutionResult()
{
    if (!is_executing || !exec_future.valid())
        return;

    if (exec_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        auto result = exec_future.get();

        std::strncpy(output_buffer, result.c_str(), sizeof(output_buffer) - 1);
        output_buffer[sizeof(output_buffer) - 1] = '\0';

        std::string plot = kernel->getPlot();
        processPlot(plot);

        is_executing = false;
    }
}

void CodeCell::processPlot(const std::string& rawPlot)
{
    if (rawPlot.empty())
        return;

    base64 = rawPlot;

    std::string cleaned = std::regex_replace(
        rawPlot,
        std::regex("\\[\\[OK\\]]\\s*$"),
        ""
    );

    updateTextureFromBase64(cleaned);
}

void CodeCell::updateTextureFromBase64(const std::string& base64Data)
{
    std::string decoded = ImgOpen::base64_decode(base64Data);

    int w = 0, h = 0;
    GLuint new_texture =
        ImgOpen::LoadTextureFromMemory(decoded.data(), decoded.size(), w, h);

    if (new_texture == 0)
    {
        std::cerr << "Failed to load texture\n";
        return;
    }

    clearTexture();

    texture = new_texture;
    texture_width = w;
    texture_height = h;
}

void CodeCell::clearTexture()
{
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
}

void CodeCell::setKernel(PythonKernel* kernelPtr)
{
    kernel = kernelPtr;
}

void CodeCell::setCodeInput(const std::string& input)
{
    inputEditor.SetText(input);
}

void CodeCell::setCodeOutput(const std::string& output)
{
    std::strncpy(output_buffer, output.c_str(), sizeof(output_buffer) - 1);
    output_buffer[sizeof(output_buffer) - 1] = '\0';
}

std::string CodeCell::getCodeInput() const
{
    return inputEditor.GetText();
}

std::string CodeCell::getCodeOutput() const
{
    return output_buffer;
}

std::string CodeCell::getBase64() const
{
    return base64;
}





