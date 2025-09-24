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

# =================== MAIN PROGRAM ===================
add_executable(SimPy
        src/main.cpp
        src/imgui_backends/imgui_impl_glfw.cpp
        src/imgui_backends/imgui_impl_opengl3.cpp
        src/gui/guiClass.cpp
        src/data/blocks.cpp
        src/data/structures.cpp
        src/data/data_sender/data_channel_manager.cpp
        src/data/math/matrix_operation/matrix_op.cpp
        src/data/math/solvers/solverMethod.cpp
        src/data/math/solvers/solver.cpp
        src/data/math/solvers/solverManager.cpp
        src/data/math/digital_signal_processing/DSP.cpp
        src/data/math/math_help_fun/math_help_fun.cpp
        src/data/data_sender/ESP_communication.h
        src/data/data_sender/ESP_communication.cpp
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

        src/data/blocks.cpp
        src/data/structures.cpp
        src/data/data_sender/data_channel_manager.cpp
        src/data/math/matrix_operation/matrix_op.cpp
        src/data/math/solvers/solverMethod.cpp
        src/data/math/solvers/solver.cpp
        src/data/math/solvers/solverManager.cpp
        src/data/math/digital_signal_processing/DSP.cpp
        src/data/math/math_help_fun/math_help_fun.cpp
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