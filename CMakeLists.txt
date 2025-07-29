cmake_minimum_required(VERSION 3.10)  # Dodajemy minimalną wersję CMake
project(patrykcpp)  # Określamy nazwę projektu

set(CMAKE_CXX_STANDARD 20)

# Znajdujemy wymagane pakiety
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(implot REQUIRED)
find_package(nlohmann_json REQUIRED)

# =================== GŁÓWNY PROGRAM ===================
add_executable(patrykcpp
        src/main.cpp
        src/imgui_backends/imgui_impl_glfw.cpp
        src/imgui_backends/imgui_impl_opengl3.cpp
        src/code_edit_block/CodeCell.h
        src/code_edit_block/CodeCell.cpp
        src/python_kernel/PythonKernel.h
        src/python_kernel/PythonKernel.cpp
        src/code_edit_block/MarkDownCell.h
        src/code_edit_block/MarKDownCell.cpp
        src/code_edit_block/BaseCell.h
        src/GUI_module/main_window.h
        src/GUI_module/main_window.cpp
        src/json_config_module/file_menager.h
        src/json_config_module/file_menager.cpp
        src/json_config_module/openFile.h
        src/json_config_module/openFile.cpp
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
)

target_include_directories(patrykcpp PRIVATE
        ${imgui_INCLUDE_DIRS}
        ${implot_INCLUDE_DIRS}
        src/imgui_backends
        src/python_kernel
        src/code_edit_block
        src/GUI_module
        src/json_config_module
        ${CMAKE_SOURCE_DIR}/include
)
