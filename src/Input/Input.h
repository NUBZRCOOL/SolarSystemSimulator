#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../Camera/Camera.h"

class Input {

    public:

        static void init(GLFWwindow *window, Camera *cam);
        static void update(float deltaTime);
        static void syncCursor();

        static bool mouseCaptured;

    private:
    
        static GLFWwindow *window;
        static Camera *camera;

        static bool firstMouse;
        static float lastX, lastY;
        static bool escPressed;

        static void mouseCallback(GLFWwindow *, double, double);
        static void scrollCallback(GLFWwindow *, double, double);
};