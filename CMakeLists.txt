cmake_minimum_required(VERSION 3.30)
project(SimPy)

set(CMAKE_CXX_STANDARD 20)

# znajduje wymagane pakiety
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(implot REQUIRED)
find_package(cereal REQUIRED)
find_package(stb REQUIRED)
find_package(portable-file-dialogs REQUIRED)

# =================== MAIN PROGRAM ===================
add_executable(SimPy
        src/main.cpp
        src/imgui_backends/imgui_impl_glfw.cpp
        src/imgui_backends/imgui_impl_opengl3.cpp
        # bloczki i inne dane
        src/core/structures/BacisBlock.cpp
        src/core/structures/Blocks.cpp
        src/core/structures/Model.cpp
        src/core/structures/Connection.cpp
        # przesył danych
        src/data/data_sender/DataChannelManager.cpp
        src/data/data_sender/ESPCommunication.cpp
        # matematyka
        src/data/math/matrix_operation/MatrixOperations.cpp
        src/data/math/solvers/SolverMethod.cpp
        src/data/math/solvers/Solver.cpp
        src/data/math/solvers/SolverManager.cpp
        src/data/math/digital_signal_processing/DSP.cpp
        src/data/math/math_help_fun/MathHelperFunctions.cpp
        # gui
        src/gui/GUICore.cpp
        src/gui/windows/DockableWindowManager.cpp
        src/gui/files/FileManager.cpp
        src/gui/viewport/ViewportManager.cpp
        src/gui/windows/BlocksManager.cpp
        src/gui/viewport/ConnectionManager.cpp
        # rdzenio-podobne funkcje
        src/core/project/ProjectMetadata.cpp
)

target_compile_definitions(SimPy PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_link_libraries(SimPy PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        glad::glad
        implot::implot
        cereal::cereal
        stb::stb
        portable-file-dialogs::portable-file-dialogs
)

target_include_directories(SimPy PRIVATE
        ${imgui_INCLUDE_DIRS}
        src/imgui_backends
        src/gui
        src/data
)

target_compile_definitions(SimPy PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

# =================== TESTS ===================
enable_testing()

# tworzymy target dla testów
add_executable(tests
        tests/data_tests/test_structures.cpp
        tests/test.cpp

        # bloczki i inne dane
        src/core/structures/BacisBlock.cpp
        src/core/structures/Blocks.cpp
        src/core/structures/Model.cpp
        src/core/structures/Connection.cpp
        # matematyka
        src/data/math/matrix_operation/MatrixOperations.cpp
        src/data/math/solvers/SolverMethod.cpp
        src/data/math/solvers/Solver.cpp
        src/data/math/solvers/SolverManager.cpp
        src/data/math/digital_signal_processing/DSP.cpp
        src/data/math/math_help_fun/MathHelperFunctions.cpp
        # przesył danych
        src/data/data_sender/DataChannelManager.cpp
        src/data/data_sender/ESPCommunication.cpp
)

target_include_directories(tests PRIVATE src)

target_compile_definitions(tests PRIVATE UNIT_TESTS)

target_link_libraries(tests PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        glad::glad
        implot::implot
)

# rejestracja testów w CTest - ale tego nie sprawdzałem
add_test(NAME t_structures COMMAND tests)