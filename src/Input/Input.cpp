#include "Input.h"
#include "../../imgui/imgui.h"


GLFWwindow *Input::window = nullptr;
Camera *Input::camera = nullptr;
bool Input::mouseCaptured = true;
bool Input::firstMouse = true;
bool Input::escPressed = false;
float Input::lastX = 0.0f;
float Input::lastY = 0.0f;

void Input::init(GLFWwindow *win, Camera *cam) {
    window = win;
    camera = cam;

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Input::update(float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_Q)) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressed) {
        mouseCaptured = false;
        escPressed = true;
    } if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) escPressed = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseCaptured && !ImGui::GetIO().WantCaptureMouse) {
        mouseCaptured = true;
    }

    if (ImGui::GetIO().WantCaptureKeyboard || !mouseCaptured) return;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->processKeys(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->processKeys(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->processKeys(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->processKeys(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera->processKeys(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera->processKeys(DOWN, deltaTime);
}

void Input::mouseCallback(GLFWwindow *win, double x, double y) {
    if (!mouseCaptured) return;

    float xpos = static_cast<float>(x);
    float ypos = static_cast<float>(y);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera->processMouse(xoffset, yoffset);
}

void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!mouseCaptured) return;
    camera->processScroll(static_cast<float>(yoffset));
}

void Input::syncCursor() {
    ImGuiIO& io = ImGui::GetIO();

    if (!mouseCaptured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    }
}