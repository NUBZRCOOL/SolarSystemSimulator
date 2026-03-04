#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include "Camera/Camera.h"
#include "math.h"
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/gtc/type_ptr.hpp"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "Window/Window.h"
#include "Input/Input.h"
#include "Scene/Scene.h"
#include "ImGuiLayer/ImGuiLayer.h"
#include "Renderer/Renderer.h"
#include "ParametricCurve/ParametricCurve.h"
#include "Planet/Planet.h"


int WIDTH = 1920;
int HEIGHT = 1080;

float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;
float deltaTime = 0.0f;
float fpsTimer = 0.0f;
int numFrames = 0;
double fps = 0.0f;

const double TARGET_FPS = 60.0;
const double TARGET_FRAME_TIME = 1 / TARGET_FPS;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));

float *camFOV = &camera.Zoom;
float *speed = &camera.MovementSpeed;

std::vector<std::reference_wrapper<Object>> objects = {};
static std::vector<std::string> objectNames;

static int selectedObjectIndex = 0;
static glm::vec3 objectPosition = glm::vec3(0.0f);
static glm::vec3 objectScale = glm::vec3(1.0f);
float uniformScale = 1.0f;
static glm::vec3 objectRotation = glm::vec3(0.0f);
static float rotationAngle = 0.0f;



int main(int argc, char **argv) {

    Window window(WIDTH, HEIGHT, "Solar System Simulation");

    Input::init(window.get(), &camera);
    ImGuiLayer::init(window.get());
    ImGuiLayer::setFontScale(2.0f);

    float cols[] = {0.862745098039, 0.596078431373, 0.2};

    Planet Earth("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\earth\\earth.glb", 3.0f, 0.5f, 0.0f, 0.0f);
    Object lamp("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\lamp\\lamp.glb");
    Planet Mars("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\mars\\mars.glb", 10.0f, 0.2f, 0.0f, 0.0f);

    Shader curveShader(
        "C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\shaders\\parametric\\vertex.vs",
        "C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\shaders\\parametric\\fragment.fs",
        "C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\shaders\\parametric\\geometry.gs"       
    );
    ParametricCurve curve;
    ParametricCurve curveM;

    Scene scene;
    scene.add(Earth.getPlanet());
    scene.add(lamp);
    scene.add(Mars.getPlanet());

    Renderer renderer;

    Light light;

    objects = scene.getObjects();
    objectNames = scene.getObjectNames();

    if (!objects.empty()) {
        objectPosition = objects[0].get().getPosition();
        objectScale = objects[0].get().getScale();
        uniformScale = objects[0].get().getScale().x;
        objectRotation = objects[0].get().getRotationEuler();
    }

    auto lastTime = std::chrono::steady_clock::now();
    while (!window.shouldClose()) {

        auto curTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<float>(curTime - lastTime).count();
        numFrames++;
        fpsTimer += deltaTime;
        if (fpsTimer > 0.25f) {
            fps = (float)1 / deltaTime;
            fpsTimer = 0; numFrames = 0;
        }
        lastTime = curTime;
        
        Input::update(deltaTime);
        Input::syncCursor();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        light.color = glm::vec3(cols[0], cols[1], cols[2]);
        light.position = lamp.getPosition();

        Earth.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Earth.solveEccAnom();
        Earth.update();

        Mars.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Mars.solveEccAnom();
        Mars.update();

        glm::mat4 proj = glm::perspective(
            glm::radians(camera.Zoom),
            (float)WIDTH / HEIGHT,
            2.0f,
            1000.0f
        );
        curve.init(0, 2 * AI_MATH_PI_F, 100);
        curve.updateCurve(Earth.getP());
        curve.render(curveShader, camera.getViewMat(), proj, 2.0, glm::vec2(WIDTH, HEIGHT));

        curveM.init(0, 2 * AI_MATH_PI_F, 100);
        curveM.updateCurve(Mars.getP());
        curveM.render(curveShader, camera.getViewMat(), proj, 2.0, glm::vec2(WIDTH, HEIGHT));
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        renderer.render(scene, camera, light, window.getWidth(), window.getHeight(), proj);

        ImGuiLayer::begin();
        ImGui::Begin("Debug");
        ImGui::Text("FPS: %.2f", fps);
        ImGui::Text("Camera pos: X: %.2f, Y: %.2f, Z: %.2f", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::Text("Camera rot: X: %.2f, Y: %.2f, Z: %.2f", camera.Front.x, camera.Front.y, camera.Front.z);
        ImGui::Text("Pitch: %.2f Yaw: %.2f", camera.Pitch, camera.Yaw);
        ImGui::SliderFloat("Speed", speed, 0.1, 15);
        ImGui::SliderFloat("Camera FOV", camFOV, 1, 89);
        // ImGui::Checkbox("Cross-view", &crossView);
        ImGui::Separator();
        ImGui::SliderFloat("Semi-major axis", &Earth.semiMaj, 0.1, 50);
        ImGui::SliderFloat("Eccentricity", &Earth.ecc, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::ColorEdit3("Light color", cols);
        ImGui::Separator();
        if (!objects.empty() && selectedObjectIndex >= 0 && selectedObjectIndex < objects.size()) {
            Object& selectedObj = objects[selectedObjectIndex].get();
            objectPosition = selectedObj.getPosition();
            objectScale = selectedObj.getScale();
            uniformScale = selectedObj.getScale().x;
            objectRotation = selectedObj.getRotationEuler();
        }
        if (ImGui::BeginCombo("Select Object", objectNames[selectedObjectIndex].c_str())) {
            for (int i = 0; i < objectNames.size(); i++) {
                bool isSelected = selectedObjectIndex == i;
                if (ImGui::Selectable(objectNames[i].c_str(), isSelected)) {
                    selectedObjectIndex = i;
                    objectPosition = objects[selectedObjectIndex].get().getPosition();
                    objectScale = objects[selectedObjectIndex].get().getScale();
                    uniformScale = objects[selectedObjectIndex].get().getScale().x;
                    objectRotation = objects[selectedObjectIndex].get().getRotationEuler();
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();
        ImGui::Text("Transform Controls");
        if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(500);
            if (ImGui::DragFloat3("Position", &objectPosition[0], 0.01f)) {
                objects[selectedObjectIndex].get().setPosition(objectPosition);
            }
            ImGui::PopItemWidth();
            if (ImGui::Button("Snap to origin")) {
                objectPosition = glm::vec3(0.0f);
                objects[selectedObjectIndex].get().setPosition(objectPosition);
            }
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(500);
            if (ImGui::DragFloat3("Scale", &objectScale[0], 0.01f)) {
                objects[selectedObjectIndex].get().setScale(objectScale);
            }
            if (ImGui::DragFloat("Uniform scale", &uniformScale, 0.01f)) {
                objectScale.x = objectScale.y = objectScale.z = uniformScale;
                objects[selectedObjectIndex].get().setScale(objectScale);
            }
            ImGui::PopItemWidth();
            if (ImGui::Button("Reset scale")) {
                objectScale = glm::vec3(1.0f);
                uniformScale = 1.0f;
                objects[selectedObjectIndex].get().setScale(objectScale);
            }
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Rotate", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(500);
            if (ImGui::DragFloat3("Rotate", &objectRotation[0], 0.5f)) {
                objects[selectedObjectIndex].get().setRotation(objectRotation);
            }
            ImGui::PopItemWidth();
            if (ImGui::Button("Reset rotation")) {
                objectRotation = glm::vec3(0.0f);
                objects[selectedObjectIndex].get().setRotation(objectRotation);
            }
        }
        ImGui::End();
        ImGuiLayer::end();

        window.swapBuffers();
        window.pollEvents();
    }

    ImGuiLayer::shutdown();

    glfwTerminate();
    return 0;
}