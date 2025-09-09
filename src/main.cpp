//
// Created by tajbe on 18.04.2025.
//

#include "gui/guiClass.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <vector>
#include <memory>


int main() {
    // setup glfw + open gl bedzie potrzebny tutaj
    // ---------------------------------------------------------------------------------------


    // setup windows
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);


    // create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "SimPy", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);

    // enable vsync
    glfwSwapInterval(1);

    // tie window context to glad's openGL loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw("unable to context to openGL");

    int screen_width, screen_height;
    glfwGetWindowSize(window, &screen_width, &screen_height);
    glViewport(0, 0, screen_width, screen_height);
    // background color
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);


    // ---------------------------------------------------------------------------------------
    guiClass guiInterface;

    guiInterface.init(window, glsl_version);
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        // czyszczenie, bo inaczej zostają artefakty
        glClear(GL_COLOR_BUFFER_BIT);

        guiInterface.newFrame();
        guiInterface.update();
        guiInterface.render();

        // basically says draw what's on the screen xd
        glfwSwapBuffers(window);
    }
    guiInterface.shutdown();
}