//
// Created by patryk on 10.02.26.
//

#include "notebookTab.h"

NotebookTab::NotebookTab(const fs::path& pythonPath) {
    kernel = new PythonKernel(pythonPath);
    std::cout << "debug" << std::endl;
    kernel->start(fs::current_path());
    std::cout << "debug" << std::endl;

}

NotebookTab::~NotebookTab() {
    delete kernel;
}


void NotebookTab::setKernel(PythonKernel *kernel) {
    this->kernel = kernel;
}


void NotebookTab::Draw() {
    // 1. Inicjalizacja focus_index jeśli wektor nie jest pusty, a index to -1
    if (focus_index == -1 && !cells.empty()) {
        focus_index = 0;
    }

    if (ImGui::Button("Add Code Cell")) {
        // Wstawiamy pod komórkę z fokusem
        int insert_pos = std::clamp(focus_index + 1, 0, (int)cells.size());
        this->cells.insert(this->cells.begin() + insert_pos, std::make_unique<CodeCell>(*this->kernel));

        // Po dodaniu nowa komórka staje się nowym punktem odniesienia
        focus_index = insert_pos;
    }

    ImGui::Separator();

    for (int i = 0; i < (int)cells.size(); ++i) {
        ImGui::PushID(i);
        bool requestDelete = cells[i]->Draw(i);

        // 2. Logika śledzenia fokusu (Twój stary mechanizm)
        if (!cells[i]->focusedPrev && cells[i]->focused) {
            focus_index = i;
            // Resetujemy flagi w innych komórkach, by tylko jedna była "prowadząca"
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


void NotebookTab::Update() {

}


