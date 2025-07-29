#include "CodeCell.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <memory>
#include "main_window.h"
#include <filesystem>
namespace fs = std::filesystem;

void RenderUI() {
    ImVec2 dispSize = ImGui::GetIO().DisplaySize;
    static MainWindow main_window;

    main_window.refresDisp(dispSize);
    main_window.mainWindow();
}


/*
void RenderUI() {
    ImGui::Begin("File Browser");
    static std::string selected_path;
    fs::path root = "/home/patryk/Desktop";

    if (ImGui::TreeNode(root.filename().string().c_str())) {
        ShowDirectoryTree(root, selected_path);

        if (!selected_path.empty()) {
            std::cout << selected_path << std::endl;
            selected_path.clear();
        }

        ImGui::TreePop();
    }
    ImGui::End();
}
*/


int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        std::cerr << "Failed to get primary monitor" << std::endl;
        glfwTerminate();
        return -1;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode) {
        std::cerr << "Failed to get video mode" << std::endl;
        glfwTerminate();
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "SimPy Code Editor", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    glfwSetWindowPos(window, 0, 0);  // Ustawienie okna na (0, 0)
    glfwSetWindowSize(window, mode->width, mode->height); // Okno na pełną szerokość i wysokość ekranu

    // Pętla renderująca GUI w głównym oknie
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Renderujemy GUI
        RenderUI();

        ImGui::Render();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);  // Wyczyść ekran
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

