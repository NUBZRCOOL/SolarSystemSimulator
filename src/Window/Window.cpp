#include "Window.h"
#include <iostream>

Window::Window(int width, int height, const char *title) : width(width), height(height) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) {
        std::cout << "Couldn't create window" << std::endl;
        glfwTerminate();
        std::exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Couldn't initialize GLAD" << std::endl;
        std::exit(-1);
    }
    
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frameBufferCallback);

    glEnable(GL_DEPTH_TEST);
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::frameBufferCallback(GLFWwindow *win, int w, int h) {
    glViewport(0, 0, w, h);
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
    self->width = w;
    self->height = h;
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window); }

void Window::swapBuffers() { glfwSwapBuffers(window); }

void Window::pollEvents() { glfwPollEvents(); }