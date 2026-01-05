//
// Created by tajbe on 25.10.2025.
//
#include "DockableWindowManager.h"
#include "../../GUICore.h"
#include "../../../core/structures/Model.h"
#include "../../../core/structures/Blocks.h"
#include "../../../data/math/solvers/SolverMethod.h"



ImVec2 DockableWindowManager::lerpVec2(const ImVec2& a, const ImVec2& b, float t) {
    // ease-out cubic dla płynniejszej animacji
    t = 1.0f - std::pow(1.0f - t, 3.0f);
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t
    };
}

DockPosition DockableWindowManager::checkDockPosition(ImVec2 windowPos, ImVec2 windowSize) const {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    // lewa krawędź
    if (windowPos.x < dockSnapDistance)
        return DockPosition::Left;
    // prawą krawędź
    if (windowPos.x + windowSize.x > displaySize.x - dockSnapDistance)
        return DockPosition::Right;
    // górna krawędź
    if (windowPos.y < dockSnapDistance)
        return DockPosition::Top;
    // dolną krawędź
    if (windowPos.y + windowSize.y > displaySize.y - dockSnapDistance)
        return DockPosition::Bottom;

    return DockPosition::None;
}

ImVec2 DockableWindowManager::calculateDockedPosition(DockPosition position, DockableWindowType windowType) const {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    switch (position) {
        case DockPosition::Left:
            // jeśli RUN WINDOW i MENU WINDOW jest już zadockowane po lewej, umieść poniżej
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && (menuWindow.position == DockPosition::Left || menuWindow.position == DockPosition::Top))
                return {0, lastMenuHeight + 1 + ImGui::GetFrameHeight()};
            return {0, ImGui::GetFrameHeight()};

        case DockPosition::Right:
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && menuWindow.position == DockPosition::Right)
                return {displaySize.x - DEFAULT_DOCKED_START_SIZE.x, lastMenuHeight + 1 + ImGui::GetFrameHeight()};
            return {displaySize.x - (windowType == DockableWindowType::Start ? DEFAULT_DOCKED_START_SIZE.x : DEFAULT_DOCKED_MENU_SIZE.x), ImGui::GetFrameHeight()};

        case DockPosition::Top:
            // rozsuń okna w poziomie
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && (menuWindow.position == DockPosition::Left || menuWindow.position == DockPosition::Top))
                return {DEFAULT_DOCKED_MENU_SIZE.x + 1, ImGui::GetFrameHeight()};
            return {0, ImGui::GetFrameHeight()};

        case DockPosition::Bottom:
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && menuWindow.position == DockPosition::Bottom)
                return {DEFAULT_DOCKED_MENU_SIZE.x + 1, displaySize.y - DEFAULT_DOCKED_START_SIZE.y};
            return {0, displaySize.y - (windowType == DockableWindowType::Start ? DEFAULT_DOCKED_START_SIZE.y : lastMenuHeight)};

        default:
            return {100, 100};
    }
}

ImVec2 DockableWindowManager::calculateDockedSize(DockPosition position, DockableWindowType windowType) const {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    if (windowType == DockableWindowType::Menu) {
        // oblicz dostępną wysokość
        float availableHeight = displaySize.y - ImGui::GetFrameHeight();

        // jeśli start jest też zdockowany do tej samej krawędzi to zostaw miejsce
        if (startWindow.isDocked && ((position == DockPosition::Left && startWindow.position == DockPosition::Left) || (position == DockPosition::Right && startWindow.position == DockPosition::Right)))
            // rezerwuj miejsce na start + odstęp
            availableHeight -= (DEFAULT_DOCKED_START_SIZE.y + 5);

        // szerokość i maksymalna wysokość
        return {DEFAULT_DOCKED_MENU_SIZE.x, max(150.0f, availableHeight)};
    }

    return DEFAULT_DOCKED_START_SIZE;
}

void DockableWindowManager::drawMenu() {
    ImVec2 menuPos;
    ImGuiWindowFlags menuFlags = ImGuiWindowFlags_AlwaysAutoResize;

    if (menuWindow.isDocked) {
        ImVec2 menuSize = calculateDockedSize(menuWindow.position, DockableWindowType::Menu);
        ImVec2 targetPos = calculateDockedPosition(menuWindow.position, DockableWindowType::Menu);

        // animacja dockowania
        if (menuAnimating) {
            ImGuiIO& io = ImGui::GetIO();
            menuAnimationProgress += io.DeltaTime * animationSpeed;

            if (menuAnimationProgress >= 1.0f) {
                menuAnimationProgress = 1.0f;
                menuAnimating = false;
            }

            menuPos = lerpVec2(menuAnimationStart, menuAnimationTarget, menuAnimationProgress);
        } else {
            menuPos = targetPos;
        }

        menuFlags |= ImGuiWindowFlags_NoMove;

        // constraints zamiast stałego rozmiaru
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(menuSize.x, 150.0f),
            ImVec2(menuSize.x, menuSize.y)
        );

        ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always);
    } else {
        menuPos = menuWindow.undockedPos;
        ImGui::SetNextWindowSizeConstraints(ImVec2(160, 150), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::SetNextWindowPos(menuPos, ImGuiCond_FirstUseEver);
    }

    ImGui::SetNextWindowBgAlpha(0.6f);

    if (!menuWindow.isDocked)
        ImGui::SetNextWindowSize(ImVec2(DEFAULT_DOCKED_MENU_SIZE.x, 0), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Menu", nullptr, menuFlags)) {
        // zapisz wysokość okna (dla pozycjonowania Start)
        if (menuWindow.isDocked) {
            lastMenuHeight = ImGui::GetWindowHeight();
            menuHeightCalculated = true;
        }

        // przyciski dodawania bloków

        // modul math
        if (ImGui::CollapsingHeader("Math")) {
            if (ImGui::Button("Add Sum Box"))
                guiCore->model.addBlock<SumBlock>();
            if (ImGui::Button("Add Multiply Box"))
                guiCore->model.addBlock<MultiplyBlock>();
            if (ImGui::Button("Add Division Box"))
                guiCore->model.addBlock<DivisionBlock>();
            if (ImGui::Button("Add Integrator Box"))
                guiCore->model.addBlock<IntegratorBlock>();
            if (ImGui::Button("Add Diff Box"))
                guiCore->model.addBlock<DifferentiatorBlock>();
            if (ImGui::Button("Add Trigonometric Funcion Box"))
                guiCore->model.addBlock<TrigonometricFunctionBlock>();
            if (ImGui::Button("Add Sqrt Box"))
                guiCore->model.addBlock<sqrtBlock>();
            if (ImGui::Button("Add Squered Box"))
                guiCore->model.addBlock<squaredBlock>();
        }

        // modul contorl
        if (ImGui::CollapsingHeader("Control Continous")) {
            if (ImGui::Button("Add Tf box"))
                guiCore->model.addBlock<TransferFuncionContinous>();
            if (ImGui::Button("Add PID box"))
                guiCore->model.addBlock<PID_regulator>();
            if (ImGui::Button("Add Gain Box"))
                guiCore->model.addBlock<GainBlock>();
            if (ImGui::Button("Add Saturation Box"))
                guiCore->model.addBlock<SaturationBlock>();
            if (ImGui::Button("Add DeadZone Box"))
                guiCore->model.addBlock<DeadZoneBlock>();
        }

        // modul inputy
        if (ImGui::CollapsingHeader("Input")) {
            if (ImGui::Button("Add Step Box"))
                guiCore->model.addBlock<StepBlock>();
            if (ImGui::Button("Add SinusInput Box"))
                guiCore->model.addBlock<SinusInputBlock>();
            if (ImGui::Button("Add PWM Input"))
                guiCore->model.addBlock<PWMInputBlock>();
            if (ImGui::Button("Add WhiteNoise Box"))
                guiCore->model.addBlock<WhiteNoiseInputBlock>();
            if (ImGui::Button("Add Input form File"))
                guiCore->model.addBlock<SignalFromFileBlock>();
        }

        // modul spketum czestoliwosciowego
        if (ImGui::CollapsingHeader("DSP")) {
            if (ImGui::Button("Add FFT Box"))
                guiCore->model.addBlock<FFTBlock>();
            // if (ImGui::Button("Add STFT Box (work in progres)"))
            //     guiCore->model.addBlock<STFT_block>();
            if (ImGui::Button("Add filter Box"))
                guiCore->model.addBlock<filterImplementationBlock>();
            if (ImGui::Button("Add MovAvg Box"))
                guiCore->model.addBlock<meanFilter1DBlock>();
            if (ImGui::Button("Add MedianFilter Box"))
                guiCore->model.addBlock<medianFilter1DBlock>();
            if (ImGui::Button("Add Window Box"))
                guiCore->model.addBlock<WindowApplayerBlock>();
            if (ImGui::Button("To Fixpoint box"))
                guiCore->model.addBlock<ToFixpoint>();
            if (ImGui::Button("From Fixpoint box"))
                guiCore->model.addBlock<FromFixpoint>();
        }

        // modul print/ploty
        if (ImGui::CollapsingHeader("Print")) {
            if (ImGui::Button("Add Print Box"))
                guiCore->model.addBlock<PrintBlock>();
            if (ImGui::Button("Add Plot Box"))
                guiCore->model.addBlock<PlotBlock>();
            if (ImGui::Button("Add Plot XY Box"))
                guiCore->model.addBlock<PlotXYBlock>();
            if (ImGui::Button("Add Spectogram Box (work in progres)"))
                guiCore->model.addBlock<PlotHeatmapBlock>();
        }

        // moduł code
        if (ImGui::CollapsingHeader("Code Box")) {
            if (ImGui::Button("Add Python Box"))
                guiCore->model.addBlock<pythonBlock>();
            if (ImGui::Button("Add C++ Box"))
                guiCore->model.addBlock<cppBlock>();
        }

        // bloki logiczne (sprawdzenie czy aktualna struktra sie do tego nadaje)
        if (ImGui::CollapsingHeader("Logic")) {
            if (ImGui::Button("Add OR Box"))
                guiCore->model.addBlock<logicORBlock>();
            if (ImGui::Button("Add AND Box"))
                guiCore->model.addBlock<logicANDBlock>();
            if (ImGui::Button("Add NOT Box"))
                guiCore->model.addBlock<logicNOTBlock>();
            if (ImGui::Button("Add NOR Box"))
                guiCore->model.addBlock<logicNORBlock>();
        }

        #ifdef __linux__
            // bloki zwzane z HIL i coderem esp
            if (ImGui::CollapsingHeader("ESP Coder")) {
                if (ImGui::Button("Add ESP output"))
                    guiCore->model.addBlock<ESPoutBlock>();
                if (ImGui::Button("Add ESP input"))
                    guiCore->model.addBlock<ESPinBlock>();
            }
        #endif

        // wysylanie danych
        if (ImGui::CollapsingHeader("Sender")) {
            if (ImGui::Button("Add Sender Box"))
                guiCore->model.addBlock<DataSenderBlock>();
        }

        ImGui::Separator();

        // przycisk do undocking
        if (menuWindow.isDocked) {
            if (ImGui::Button("Undock Menu")) {
                menuWindow.isDocked = false;
                menuWindow.position = DockPosition::None;

                // przesuń okno z dala od krawędzi i zresetuj rozmiar
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 displaySize = io.DisplaySize;
                menuWindow.undockedPos = ImVec2(displaySize.x * 0.3f, displaySize.y * 0.3f);
            }
        }

        // sprawdź docking tylko jeśli okno nie jest zadockowane
        if (!menuWindow.isDocked) {
            ImVec2 currentPos = ImGui::GetWindowPos();
            ImVec2 currentSize = ImGui::GetWindowSize();

            // zapisz pozycję jako undocked
            menuWindow.undockedPos = currentPos;

            // sprawdź czy powinno się zadockować
            static bool wasDragging = false;
            bool isDragging = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowFocused();

            if (wasDragging && !isDragging) {
                DockPosition newDockPos = checkDockPosition(currentPos, currentSize);
                if (newDockPos != DockPosition::None) {
                    // rozpocznij animację
                    menuAnimationStart = currentPos;
                    menuAnimationTarget = calculateDockedPosition(newDockPos, DockableWindowType::Menu);
                    menuAnimationProgress = 0.0f;
                    menuAnimating = true;

                    menuWindow.isDocked = true;
                    menuWindow.position = newDockPos;
                }
            }

            wasDragging = isDragging;
        }
    }
    ImGui::End();
}

void DockableWindowManager::drawStartButton() {
    ImVec2 startPos;
    ImGuiWindowFlags startFlags = 0;

    // ma zawsze staly rozmiar
    ImGui::SetNextWindowSize(calculateDockedSize(startWindow.position, DockableWindowType::Start), ImGuiCond_Always);

    if (startWindow.isDocked) {
        ImVec2 targetPos = calculateDockedPosition(startWindow.position, DockableWindowType::Start);

        // animacja dockowania
        if (startAnimating) {
            ImGuiIO& io = ImGui::GetIO();
            startAnimationProgress += io.DeltaTime * animationSpeed;

            if (startAnimationProgress >= 1.0f) {
                startAnimationProgress = 1.0f;
                startAnimating = false;
            }

            startPos = lerpVec2(startAnimationStart, startAnimationTarget, startAnimationProgress);
        } else {
            startPos = targetPos;
        }

        startFlags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        ImGui::SetNextWindowPos(startPos, ImGuiCond_Always);
    } else {
        startPos = startWindow.undockedPos;
        ImGui::SetNextWindowSizeConstraints(ImVec2(160, 150), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::SetNextWindowPos(startPos, ImGuiCond_FirstUseEver);
    }

    ImGui::SetNextWindowBgAlpha(0.6f);

    if (ImGui::Begin("Start / Stop", nullptr, startFlags)) {
        // szerokość w pikselach
        ImGui::PushItemWidth(70.0);
        ImGui::InputDouble("Sampling Time", &Model::timeStep);
        ImGui::InputDouble("Simulation Time", &Model::simTime);

        // combo odpowiedialne za wybor solver i precyzje obliczen (musisz Antek zrobic templeta do tego bo ja nie umim XD)
        static int current_solver = 2;
        const static char* solvers[] = {"RK1", "RK2", "RK4", "CN", "GL", "Gear"};

        static int current_precision = 0;
        const static char* precisions[] = {"int", "float", "double"};

        if (ImGui::BeginCombo("Solver Type", solvers[current_solver], false)) {
            for (int n = 0; n < IM_ARRAYSIZE(solvers); n++) {
                bool is_selected = (current_precision == n);
                if (ImGui::Selectable(solvers[n], is_selected)) {
                    current_solver = n;
                    guiCore->solverName = solvers[n];
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Precision Type", precisions[current_precision], false)) {
            for (int i = 0; i < IM_ARRAYSIZE(precisions); i++) {
                bool is_selected = (current_precision == i);
                if (ImGui::Selectable(precisions[i], is_selected)) {
                    current_precision = i;
                    guiCore->solverPrecison = precisions[i];
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // mapa do wyboru Solvera
        std::unordered_map<std::string, std::function<std::shared_ptr<ISolverMethod>()>> solverMap = {
            {"RK1", [](){ return std::make_shared<RK1Method>(); }},
            {"RK2", [](){ return std::make_shared<RK2Method>(); }},
            {"RK4", [](){ return std::make_shared<RK4Method>(); }},
            {"CN",  [](){ return std::make_shared<RK1Method>(); }},
            {"GL",  [](){ return std::make_shared<RK1Method>(); }},
            {"Gear",[](){ return std::make_shared<RK1Method>(); }}
        };

        if (guiCore->simulationRunning) {
            ImGui::BeginDisabled();
            ImGui::Button("Running...");
            ImGui::EndDisabled();
        } else {
            if (ImGui::Button("Run Simulation")) {
                guiCore->simulationRunning = true;
                auto method = solverMap[guiCore->solverName]();
                SolverManager::initSolver(Model::timeStep, method);
                // uruchom w osobnym wątku i nie czekaj na niego:
                std::thread([this]() {
                    // cleanup w bloczkach jeśli jest potrzeb
                    guiCore->model.cleanupBefore();
                    // guiCore->model.makeConnections();
                    for (int i = 0; i < (Model::simTime/ Model::timeStep) + 1; i++) {
                        guiCore->model.simulate();
                    }
                    guiCore->model.cleanupAfter();
                    guiCore->model.cleanSolver();
                    guiCore->simulationRunning = false;
                }).detach();
            }
        }

        ImGui::Separator();

        if (startWindow.isDocked) {
            if (ImGui::Button("Undock Window")) {
                startWindow.isDocked = false;
                startWindow.position = DockPosition::None;

                // przesuń okno z dala od krawędzi i zresetuj rozmiar
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 displaySize = io.DisplaySize;
                startWindow.undockedPos = ImVec2(displaySize.x * 0.5f, displaySize.y * 0.4f);
            }
        }

        if (!startWindow.isDocked) {
            ImVec2 currentPos = ImGui::GetWindowPos();
            ImVec2 currentSize = ImGui::GetWindowSize();

            startWindow.undockedPos = currentPos;

            // taka sama logika jak dla menu window
            static bool wasDraggingStart = false;
            bool isDragging = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowFocused();

            if (wasDraggingStart && !isDragging) {
                DockPosition newDockPos = checkDockPosition(currentPos, currentSize);
                if (newDockPos != DockPosition::None) {
                    // rozpocznij animację
                    startAnimationStart = currentPos;
                    startAnimationTarget = calculateDockedPosition(newDockPos, DockableWindowType::Start);
                    startAnimationProgress = 0.0f;
                    startAnimating = true;

                    startWindow.isDocked = true;
                    startWindow.position = newDockPos;
                }
            }

            wasDraggingStart = isDragging;
        }
    }
    ImGui::End();
}




