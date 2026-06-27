//
// Created by patryk on 10.02.26.
//

#include "notebookTile.h"

NotebookTile::NotebookTile(const fs::path& pythonPath) {
    kernel = new PythonKernel(pythonPath);
    std::cout << "debug" << std::endl;
    kernel->start(fs::current_path());
    std::cout << "debug" << std::endl;
}

NotebookTile::~NotebookTile() {
    delete kernel;
}


void NotebookTile::setKernel(PythonKernel *kernel) {
    this->kernel = kernel;
}


void NotebookTile::Draw() {
    ImGui::SetNextWindowSize(GetSize());
    ImGui::SetNextWindowPos(GetPos());

    if (focus_index == -1 && !cells.empty()) {
        focus_index = 0;
    }
    if (ImGui::Begin("Notebook", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        if (ImGui::Button("Add Code Cell")) {
            int insert_pos = std::clamp(focus_index + 1, 0, (int)cells.size());
            this->cells.insert(this->cells.begin() + insert_pos, std::make_unique<CodeCell>(*this->kernel));

            focus_index = insert_pos;
        }

        if (ImGui::Button("Add MarkDown Cell")) {
            int insert_pos = std::clamp(focus_index + 1, 0, (int)cells.size());
            this->cells.insert(this->cells.begin() + insert_pos, std::make_unique<MardownCell>());

            focus_index = insert_pos;
        }

        ImGui::Separator();

        for (int i = 0; i < (int)cells.size(); ++i) {
            ImGui::PushID(i);
            bool requestDelete = cells[i]->Draw(i);

            if (!cells[i]->focusedPrev && cells[i]->focused) {
                focus_index = i;
                for (int j = 0; j < (int)cells.size(); ++j) {
                    if (j != i) cells[j]->focused = false;
                }
            }
            cells[i]->focusedPrev = cells[i]->focused;

            if (requestDelete) {
                cells.erase(cells.begin() + i);
                if (focus_index >= i) focus_index = std::max(0, focus_index - 1);
                --i;
                ImGui::PopID();
                continue;
            }
            ImGui::PopID();
        }
    }
    ImGui::End();
}


void NotebookTile::Update() {

}


