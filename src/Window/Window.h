#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Window {

    public:

        Window(int width, int height, const char *title);
        ~Window();

        bool shouldClose() const;
        void swapBuffers();
        void pollEvents();

        GLFWwindow *get() const { return window; }
        int getWidth() { return width; };
        int getHeight() { return height; };

    private:

        GLFWwindow *window;
        int width, height;

        static void frameBufferCallback(GLFWwindow *win, int w, int h);
};