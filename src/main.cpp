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
double deltaTime = 0.0f;
double fpsTimer = 0.0f;
int numFrames = 0;
double fps = 0.0f;

double epoch = 946684800.0;
double timeReal = epoch;

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

float radiusScale = 2e6;    
float sunRadiusScale = radiusScale / 50;
float semiMajScale = 10000;

bool curves = true;

int main(int argc, char **argv) {

    Window window(WIDTH, HEIGHT, "Solar System Simulation");

    Input::init(window.get(), &camera);
    ImGuiLayer::init(window.get());
    ImGuiLayer::setFontScale(2.0f);

    float cols[] = {0.862745098039, 0.596078431373, 0.2};

    OrbitalParameters mercuryParams = {radiusScale*1.63083872e-5, semiMajScale*0.38709843, 0.20563661, 7.00559432, 252.25032350, 77.45779628, 48.33076593};
    OrbitalParameters venusParams = {radiusScale*4.04537843e-5, semiMajScale*0.72333566, 0.00677672, 3.39467605, 181.97909950, 131.60246718, 76.67984255};
    OrbitalParameters earthParams = {radiusScale*4.25875e-5, semiMajScale*1.00000261, 0.01671123, -0.00001531, 100.46457166, 102.93768193, 0.0};
    OrbitalParameters marsParams = {radiusScale*2.2657003e-5, semiMajScale*1.52371034, 0.09339410, 1.84969142, -4.55343205, -23.94362959, 49.55953891};
    OrbitalParameters jupiterParams = {radiusScale*0.000477894503, semiMajScale*5.20288700, 0.04838624, 1.30439695, 34.39644051, 14.72847983, 100.47390909};
    OrbitalParameters saturnParams = {radiusScale*0.000389256877, semiMajScale*9.53667594, 0.05386179, 2.48599187, 49.95424423, 92.59887831, 113.66242448};
    OrbitalParameters uranusParams = {radiusScale*0.0001695345, semiMajScale*19.18916464, 0.04725744, 0.77263783, 313.23810451, 170.95427630, 74.01692503};
    OrbitalParameters neptuneParams = {radiusScale*0.000164587904, semiMajScale*30.06992276, 0.00859048, 1.77004347, -55.12002969, 44.96476227, 131.78422574};

    Planet Mercury("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\mercury\\mercury.glb", mercuryParams);
    Planet Venus("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\venus\\venus.glb", venusParams);
    Planet Earth("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\earth\\earth.glb", earthParams);
    Planet Mars("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\mars\\mars.glb", marsParams);
    Planet Jupiter("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\jupiter\\jupiter.glb", jupiterParams);
    Planet Saturn("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\saturn\\saturn.glb", saturnParams);
    Planet Uranus("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\uranus\\uranus.glb", uranusParams);
    Planet Neptune("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\neptune\\neptune.glb", neptuneParams);
    Object Sun("C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\objects\\sun\\sun.glb");
    
    

    Shader curveShader(
        "C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\shaders\\parametric\\vertex.vs",
        "C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\shaders\\parametric\\fragment.fs",
        "C:\\msys64\\home\\sraina\\SolarSystemSimulator\\res\\shaders\\parametric\\geometry.gs"       
    );

    Scene scene;
    scene.add(Mercury.getPlanet());
    scene.add(Venus.getPlanet());
    scene.add(Earth.getPlanet());
    scene.add(Mars.getPlanet());
    scene.add(Jupiter.getPlanet());
    scene.add(Saturn.getPlanet());
    scene.add(Uranus.getPlanet());
    scene.add(Neptune.getPlanet());
    scene.add(Sun);
    
    Sun.setScale(glm::vec3(sunRadiusScale*0.00465479256));

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

        timeReal += deltaTime;
        
        Input::update(deltaTime);
        Input::syncCursor();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        light.color = glm::vec3(cols[0], cols[1], cols[2]);
        light.position = Sun.getPosition();

        Mercury.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Mercury.solveEccAnom();
        Mercury.update();

        Venus.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Venus.solveEccAnom();
        Venus.update();

        Earth.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Earth.solveEccAnom();
        Earth.update();

        Mars.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Mars.solveEccAnom();
        Mars.update();

        Jupiter.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Jupiter.solveEccAnom();
        Jupiter.update();
        
        Saturn.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Saturn.solveEccAnom();
        Saturn.update();

        Uranus.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Uranus.solveEccAnom();
        Uranus.update();

        Neptune.meanAnom = ((2 * AI_MATH_PI) / 10.0f) * glfwGetTime();
        Neptune.solveEccAnom();
        Neptune.update();

        glm::mat4 proj = glm::perspective(
            glm::radians(camera.Zoom),
            (float)WIDTH / HEIGHT,
            0.1f,
            100000.0f
        );

        if (curves) {
            Mercury.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
            Venus.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
            Earth.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
            Mars.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
            Jupiter.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
            Saturn.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
            Uranus.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
            Neptune.drawCurve(curveShader, camera.getViewMat(), proj, glm::vec2(WIDTH, HEIGHT));
        }

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
        ImGui::SliderFloat("Radius Scale", &radiusScale, 1, 1e5);
        ImGui::SliderFloat("Semi major Scale", &semiMajScale, 1, 1e5);
        ImGui::Checkbox("Draw Curves?", &curves);
        ImGui::Text("Time (s): %f", timeReal);
        // ImGui::Checkbox("Cross-view", &crossView);
        ImGui::Separator();
        // ImGui::SliderFloat("Semi-major axis", &Earth.semiMaj, 0.1, 50);
        // ImGui::SliderFloat("Eccentricity", &Earth.ecc, 0.0f, 1.0f);
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