cmake_minimum_required(VERSION 3.30)
project(patrykcpp)

set(CMAKE_CXX_STANDARD 20)

# Znajdujemy wymagane pakiety
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# Dodajemy pliki źródłowe, w tym implementacje backendów
add_executable(patrykcpp
        src/main.cpp
        src/imgui_backends/imgui_impl_glfw.cpp
        src/imgui_backends/imgui_impl_opengl3.cpp
        # Jeśli potrzebujesz dodać własną implementację backendów, możesz to zrobić tutaj
)

# Definicje preprocesora dla imgui
target_compile_definitions(patrykcpp PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

# Linkujemy biblioteki
target_link_libraries(patrykcpp PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
)

# Jeśli używasz plików nagłówkowych i implementacji backendów z imgui
# upewnij się, że katalogi są dostępne w ścieżce include
target_include_directories(patrykcpp PRIVATE
        ${imgui_INCLUDE_DIRS}
        src/imgui_backends
)