//
// Created by tajbe on 18.04.2025.
//


#include "guiClass.h"
#include <iostream>
#include <thread>



// initialization of evertything regarding ImGui
void guiClass::init(GLFWwindow* win, const char* version) {
    window = win;
    glsl_version = version;

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


void guiClass::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void guiClass::update() {
    // minimalny i maksymalny rozmiar okna menu
    ImGui::SetNextWindowSizeConstraints(ImVec2(160, 150), ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::Begin("Menu")) {
        if (ImGui::Button("Add Sum Box")) {
            model.getBlocks().push_back(std::make_unique<SumBlock>(next_id++));
            // boxes.push_back(std::make_unique<FloatBox>(next_id++));
        }
        if (ImGui::Button("Add Multiply Box")) {
            model.getBlocks().push_back(std::make_unique<MultiplyBlock>(next_id++));
            // boxes.push_back(std::make_unique<StringBox>(next_id++));
        }
        if (ImGui::Button("Add Integrator Box")) {
            model.getBlocks().push_back(std::make_unique<IntegratorBlock>(next_id++));
        }
        if (ImGui::Button("Add Input Box")) {
            model.getBlocks().push_back(std::make_unique<InputBlock>(next_id++));
        }
        if (ImGui::Button("Add Print Box")) {
            model.getBlocks().push_back(std::make_unique<PrintBlock>(next_id++));
        }
        if (ImGui::Button("Add Plot Box")) {
            model.getBlocks().push_back(std::make_unique<PlotBlock>(next_id++));
        }
        if (ImGui::Button("Add Gain Box")) {
            model.getBlocks().push_back(std::make_unique<GainBlock>(next_id++));
        }
    }
    ImGui::End();

    for (auto& box : model.getBlocks()) {
        if (box->open)
            drawBox(*box);
    }

    drawConnections();

    drawStartButton();

    // zbieramy ID boxów do usunięcia
    std::vector<int> to_remove;
    for (auto& box : model.getBlocks()) {
        if (!box->open) {
            to_remove.push_back(box->id);
        }
    }

    // usuwamy połączenia do/z tych boxów
    for (auto& box : model.getBlocks()) {
        auto& conns = box->connections;
        conns.erase(std::remove_if(conns.begin(), conns.end(),
            [&](int id) {
                return std::find(to_remove.begin(), to_remove.end(), id) != to_remove.end();
            }), conns.end());
    }

    // usuwamy same boxy
    model.getBlocks().erase(std::remove_if(model.getBlocks().begin(), model.getBlocks().end(),
        [](const std::unique_ptr<Block>& box) {
            return !box->open;
        }), model.getBlocks().end());
}


void guiClass::drawBox(Block& box) {
    std::string title = "Box #" + std::to_string(box.id);
    ImGui::SetNextWindowPos(box.position, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 120), ImGuiCond_Once);

    bool is_hovered = false;

    if (!ImGui::Begin(title.c_str(), &box.open)) {
        ImGui::End();
        return;
    }

    box.drawContent();

    // znak + po lewej stronie box'a
    // pozycję i rozmiar okna boxa
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // TODO: wziąć w pętle w zależności od ilości outputów
    // środek po prawej krawędzi
    ImVec2 center = ImVec2(windowPos.x + windowSize.x - 15, windowPos.y + windowSize.y * 0.5f);

    // kursor do InvisibleButton tak, żeby kółko dało się kliknąć
    ImGui::SetCursorScreenPos(ImVec2(center.x - 10, center.y - 10));
    ImGui::InvisibleButton(("##link" + std::to_string(box.id)).c_str(), ImVec2(20, 20));

    bool isHovered = ImGui::IsItemHovered();
    bool isClicked = ImGui::IsItemClicked();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // kolor przycisku w zależności od stanu
    if (box.getNumOutputs() > 0) {
        ImU32 buttonColor = isClicked ? IM_COL32(255, 0, 0, 255) : (isHovered ? IM_COL32(255, 255, 0, 255) : IM_COL32(200, 200, 0, 255));
        draw_list->AddCircleFilled(center, 8.0f, buttonColor);
        draw_list->AddText(ImVec2(center.x - 4, center.y - 7), IM_COL32(0, 0, 0, 255), "+");
    }
    // TODO: aż do tąd pętle


    // zaczynamy przeciąganie
    if (isClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        dragging_from = box.id;
    }

    // zaczynamy przeciąganie
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        dragging_from = box.id;
    }

    // zbieramy pozycję i czy box jest pod myszą
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        is_hovered = true;
    }

    box.position = ImGui::GetWindowPos();

    // naprawa linii po resize'ie
    box.size = ImGui::GetWindowSize();

    ImGui::End();
}

void guiClass::drawConnections() {
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    // for (auto& box : boxes) {
    for (auto& box : model.getBlocks()) {
        for (auto it = box->connections.begin(); it != box->connections.end();) {
            // Szukamy boxa docelowego (target)
            auto targetIt = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b) { return b->id == *it; });
            if (targetIt != model.getBlocks().end()) {
                // naprawa linii po resize'ie
                ImVec2 p1 = (*targetIt)->position;
                ImVec2 size1 = (*targetIt)->size;
                ImVec2 p2 = box->position;
                ImVec2 size2 = box->size;

                // target: prawa krawędź, środek pionowo
                p1.x += size1.x;
                p1.y += size1.y * 0.5f;
                // source: lewa krawędź, środek pionowo
                p2.x += 0;
                p2.y += size2.y * 0.5f;


                // sprawdzamy "hoverowanie"
                const float detect_radius = 10.0f;
                bool hovered = false;

                // sample line geometry in 20 steps and check distance to mouse
                for (int step = 0; step <= 20; ++step) {
                    float t = step / 20.0f;
                    ImVec2 pt = BezierCubicCalc(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, t);
                    if (LengthSqr(pt, mousePos) < detect_radius * detect_radius) {
                        hovered = true;
                        break;
                    }
                }

                // kolor w zależności od "hoverowania"
                ImU32 color = hovered ? IM_COL32(255, 100, 100, 255) : IM_COL32(255, 255, 0, 255);
                float thickness = hovered ? 5.0f : 3.0f;

                // rysowanie krzywej
                draw_list->AddBezierCubic(
                    p1,
                    ImVec2(p1.x + 50, p1.y),
                    ImVec2(p2.x - 50, p2.y),
                    p2,
                    color,
                    thickness
                );

                // kliknięcie PPM = rozłączenie
                if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    it = box->connections.erase(it);
                    // nie inkrementujemy
                    continue;
                }
            }
            ++it;
        }
    }

    // rysowanie dynamicznej linki (w trakcie przeciągania)
    if (dragging_from) {
        auto it = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b) { return b->id == *dragging_from; });
        if (it != model.getBlocks().end()) {
            // P1 to początek (box źródłowy)
            ImVec2 p1 = (*it)->position;

            // naprawa linii po resize'ie
            ImVec2 size = (*it)->size;
            p1.x += size.x; p1.y += size.y * 0.5f;

            // P2 to koniec (mysz)
            ImVec2 p2 = ImGui::GetMousePos();
            draw_list->AddBezierCubic(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, IM_COL32(255, 255, 0, 100), 2.0f);
        }
    }

    // próbujemy zakończyć przeciąganie – jeśli mysz została puszczona
    if (dragging_from && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        bool connected = false;

        for (auto& box : model.getBlocks()) {
            ImVec2 boxMin = box->position;
            ImVec2 boxMax = ImVec2(boxMin.x + 300, boxMin.y + 120);

            if (mousePos.x >= boxMin.x && mousePos.x <= boxMax.x && mousePos.y >= boxMin.y && mousePos.y <= boxMax.y) {
                // zmieniamy kierunek połączenia
                if (box->id != *dragging_from) {
                    box->connections.push_back(*dragging_from);
                }
                connected = true;
                break;
            }
        }

        dragging_from = std::nullopt;
    }
}

void guiClass::drawStartButton() {
    if (ImGui::Begin("Start / Stop")) {
        if (simulationRunning) {
            ImGui::BeginDisabled();
            ImGui::Button("Running...");
            ImGui::EndDisabled();
        } else {
            if (ImGui::Button("Run Simulation")) {
                simulationRunning = true;
                // uruchom w osobnym wątku i nie czekaj na niego:
                std::thread([this]() {
                    model.makeConnections();
                    // cleanup w bloczkach jeśli jest potrzebny
                    model.cleanup();
                    // TODO: tu na ogół nie ma być na stałe pętli do 1000
                    // patryk chyba chciał to jakoś ustawiać, idk w sumie nie pamiętam
                    for (int i = 0; i < 1000; i++) {
                        model.simulate();
                    }
                    simulationRunning = false;
                }).detach();
            }
        }
    }
    ImGui::End();
}



void guiClass::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void guiClass::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
}