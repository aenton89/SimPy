cmake_minimum_required(VERSION 3.30)
project(SimPy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/Zc:__cplusplus /std:c++20)
endif()

set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG Release)
set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL Release)
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

add_definitions(-DNOMINMAX)

# znajduje wymagane pakiety
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(implot REQUIRED)
find_package(cereal REQUIRED)
find_package(stb REQUIRED)
find_package(portable-file-dialogs REQUIRED)
find_package(nlohmann_json REQUIRED)

# =================== MAIN PROGRAM ===================
add_executable(SimPy
        src/main.cpp
        include/imgui_backends/imgui_impl_glfw.cpp
        include/imgui_backends/imgui_impl_opengl3.cpp

        ## karty jak w chrome
        src/tabs/TabManager.cpp
        src/tabs/TabModule.cpp

        ## BLUEPRINTS
        # bloczki i inne dane
        src/blueprints/core/structures/BasicBlock.cpp
        src/blueprints/core/structures/Blocks.cpp
        src/blueprints/core/structures/Model.cpp
        src/blueprints/core/structures/Connection.cpp
        # przesył danych
        src/blueprints/data/data_sender/DataChannelManager.cpp
        src/blueprints/data/data_sender/ESPCommunication.cpp
        # matematyka
        src/blueprints/data/math/matrix_operation/MatrixOperations.cpp
        src/blueprints/data/math/solvers/SolverMethod.cpp
        src/blueprints/data/math/solvers/Solver.cpp
        src/blueprints/data/math/solvers/SolverManager.cpp
        src/blueprints/data/math/digital_signal_processing/DSP.cpp
        src/blueprints/data/math/math_help_fun/MathHelperFunctions.cpp
        # gui
        src/blueprints/gui/GUICore.cpp
        src/blueprints/gui/managers/windows/DockableWindowManager.cpp
        src/blueprints/gui/managers/files/FileManager.cpp
        src/blueprints/gui/managers/viewport/ViewportManager.cpp
        src/blueprints/gui/managers/windows/BlocksManager.cpp
        src/blueprints/gui/managers/viewport/ConnectionManager.cpp
        src/blueprints/gui/managers/BasicManager.cpp
        # rdzenio-podobne funkcje
        src/blueprints/core/project/ProjectMetadata.cpp

        ## IDE
        # jakieś coś pod ide
        src/ide/mainWindow.h
        src/ide/gui/code_editor/notebookTile.h
        src/ide/gui/code_editor/notebookTile.cpp
        src/ide/gui/code_editor/cells/baseCell.h
        src/ide/gui/code_editor/cells/codeCell.h
        src/ide/gui/code_editor/cells/codeCell.cpp
        src/ide/gui/code_editor/cells/mardownCell.h
        src/ide/mainWindow.cpp
        src/ide/gui/code_editor/cells/marddownCell.cpp
        src/ide/gui/code_editor/textEditorTile.h
        src/ide/gui/code_editor/textEditorTile.cpp
        src/ide/gui/ui/workSpace.h
        src/ide/gui/ui/workSpace.cpp
        src/ide/core/file_manipulation/FileSystemService.h
        src/ide/core/file_manipulation/FileSystemService.cpp
        src/ide/core/python_kernel/PythonKernel.h
        src/ide/core/python_kernel/PythonKernel.cpp
        src/ide/gui/code_editor/BaseTile.h
        src/ide/core/file_manipulation/FileUtils.h
        src/ide/core/file_manipulation/FileUtils.cpp
        src/ide/gui/managers/TileManager.h
        src/ide/gui/managers/TileManager.cpp

        # zewnerzne biblioteki wymagajace linkowania
        include/ide_dependencies/TextEditor.cpp
)

target_compile_definitions(SimPy PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_precompile_headers(SimPy PRIVATE
        src/pch.h
)

target_link_libraries(SimPy PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        glad::glad
        implot::implot
        cereal::cereal
        stb::stb
        nlohmann_json::nlohmann_json
        portable-file-dialogs::portable-file-dialogs
)

target_include_directories(SimPy PRIVATE
        ${imgui_INCLUDE_DIRS}
        include/imgui_backends
        src/blueprints/gui
        src/blueprints/data
)

target_compile_definitions(SimPy PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

# =================== TESTS ===================
enable_testing()

# tworzymy target dla testów
add_executable(tests
        tests/data_tests/test_structures.cpp
        tests/test.cpp

        # bloczki i inne dane
        src/blueprints/core/structures/BasicBlock.cpp
        src/blueprints/core/structures/Blocks.cpp
        src/blueprints/core/structures/Model.cpp
        src/blueprints/core/structures/Connection.cpp
        # matematyka
        src/blueprints/data/math/matrix_operation/MatrixOperations.cpp
        src/blueprints/data/math/solvers/SolverMethod.cpp
        src/blueprints/data/math/solvers/Solver.cpp
        src/blueprints/data/math/solvers/SolverManager.cpp
        src/blueprints/data/math/digital_signal_processing/DSP.cpp
        src/blueprints/data/math/math_help_fun/MathHelperFunctions.cpp
        # przesył danych
        src/blueprints/data/data_sender/DataChannelManager.cpp
        src/blueprints/data/data_sender/ESPCommunication.cpp
)

target_include_directories(tests PRIVATE src)

target_compile_definitions(tests PRIVATE UNIT_TESTS)

target_precompile_headers(SimPy PRIVATE
        src/pch.h
)

target_link_libraries(tests PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        glad::glad
        implot::implot
)

# rejestracja testów w CTest - ale tego nie sprawdzałem
add_test(NAME t_structures COMMAND tests)