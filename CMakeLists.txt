cmake_minimum_required(VERSION 3.30)
project(patrykcpp)

set(CMAKE_CXX_STANDARD 20)

# znajduje wymagane pakiety
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(implot REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(stb REQUIRED)

# =================== MAIN PROGRAM ===================
add_executable(patrykcpp
        src/main.cpp
        src/imgui_backends/imgui_impl_glfw.cpp
        src/imgui_backends/imgui_impl_opengl3.cpp
        src/gui/guiClass.cpp
        src/data/blocks.cpp
        src/data/structures.cpp
        src/data/data_sender/data_channel_manager.cpp
        src/data/math/matrix_operation/matrix_op.cpp
        src/data/math/solvers/solverMethod.cpp
)

target_compile_definitions(patrykcpp PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_link_libraries(patrykcpp PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        glad::glad
        implot::implot
        nlohmann_json::nlohmann_json
        stb::stb
)

target_include_directories(patrykcpp PRIVATE
        ${imgui_INCLUDE_DIRS}
        src/imgui_backends
        src/gui
        src/data
)

# =================== TESTS ===================
enable_testing()

# tworzymy target dla testów
add_executable(tests
        tests/data_tests/test_structures.cpp
        src/data/blocks.cpp
        src/data/structures.cpp
        src/data/data_sender/data_channel_manager.cpp
        tests/test.cpp
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