#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class ImGuiLayer {
    public:

        static void init(GLFWwindow *window);
        static void begin();
        static void end();
        static void shutdown();
        static void setFontScale(float scale);
};