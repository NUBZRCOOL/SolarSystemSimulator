#ifdef _WIN32
#include <windows.h>
#endif
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
#include "../imgui/implot.h"
#include "../imgui/implot_internal.h"
#include <iomanip>
#include <sstream>
#include <ctime>
#include "Window/Window.h"
#include "Input/Input.h"
#include "Scene/Scene.h"
#include "ImGuiLayer/ImGuiLayer.h"
#include "Renderer/Renderer.h"
#include "ParametricCurve/ParametricCurve.h"
#include "Planet/Planet.h"
#include "Skybox/Skybox.h"
#include "Misc/Misc.h"
#include "Loading/Loading.h"

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))




int WIDTH = 1920;
int HEIGHT = 1080;

float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;
double deltaTime = 0.0f;
double fpsTimer = 0.0f;
int numFrames = 0;
double fps = 0.0f;

double epoch = 946684800.0;
float timeMultiplier = 1.0f;
float actualTimeMultiplier = 1.0f;
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

float smallRadiusScale = 1e6;    
float largeRadiusScale = smallRadiusScale / 3;  
float sunRadiusScale = smallRadiusScale / 25;
float semiMajScale = 1000;

bool curves = true;

static int selectedPlanetIdx = 0; 
static bool followPlanet = false;
static Planet* activeFollowPlanet = nullptr;

bool showSky = true;
bool showConstellations = false;
bool showConstBounds = false;
bool showGrid = false;

int timezoneOffsetHours = 0;



int main(int argc, char **argv) {

    Window window(WIDTH, HEIGHT, "Solar System Simulation");

    Shader loadShader("res/shaders/loading/loading.vs", "res/shaders/loading/loading.fs");
    LoadingScreen loader;
    loader.init(WIDTH, HEIGHT, "res/textures/loading/loading.png", loadShader);

    loader.render();

    Input::init(window.get(), &camera);
    ImGuiLayer::init(window.get());
    ImGuiLayer::setFontScale(2.0f);

    loader.render();

    float cols[] = {0.862745098039, 0.596078431373, 0.2};

    InitialParameters mercuryParams = {smallRadiusScale*1.63083872e-5, semiMajScale*0.38709843, 0.20563661, 7.00559432, 252.25032350, 77.45779628, 48.33076593};
    OrbitalDerivatives mercuryDerivs = {0, 0.00002123, -0.00590158, 149472.67486623, 0.15940013, -0.12214182, 0, 0, 0, 0};
    RotationParameters mercuryRots = {318.4100, 82.9900, 329.5480, 6.138503};

    InitialParameters venusParams = {smallRadiusScale*4.04537843e-5, semiMajScale*0.72333566, 0.00677672, 3.39467605, 181.97909950, 131.60246718, 76.67984255};
    OrbitalDerivatives venusDerivs = {-0.00000026, -0.00005107, 0.00043494, 58517.81560260, 0.05679648, -0.27274174, 0, 0, 0, 0};
    RotationParameters venusRots = {30.1860, 88.7610, 160.2000, -1.481369};

    InitialParameters earthParams = {smallRadiusScale*4.25875e-5, semiMajScale*1.00000261, 0.01671123, -0.00001531, 100.46457166, 102.93768193, 0.0};
    OrbitalDerivatives earthDerivs = {-0.00000003, -0.00003661, -0.01337178, 35999.37306329, 0.31795260, -0.24123856, 0, 0, 0, 0};
    RotationParameters earthRots = {90.0000, 66.5610, 190.1470, 360.985623};

    InitialParameters marsParams = {smallRadiusScale*2.2657003e-5, semiMajScale*1.52371034, 0.09339410, 1.84969142, -4.55343205, -23.94362959, 49.55953891};
    OrbitalDerivatives marsDerivs = {0.00000097, 0.00009149, -0.00724757, 19140.29934243, 0.45223625, -0.26852431, 0, 0, 0, 0};
    RotationParameters marsRots = {352.9060, 63.2820, 176.6300, 350.891982};

    InitialParameters jupiterParams = {largeRadiusScale*0.000477894503, semiMajScale*5.20288700, 0.04838624, 1.30439695, 34.39644051, 14.72847983, 100.47390909};
    OrbitalDerivatives jupDerivs = {-0.00002864, 0.00018026, -0.00322699, 3034.90371757, 0.18199196, 0.13024619, -0.00012452, 0.06064060, -0.35635438, 38.35125000};
    RotationParameters jupRots = {247.8140, 87.7830, 284.9500, 870.536000};

    InitialParameters saturnParams = {largeRadiusScale*0.000389256877, semiMajScale*9.53667594, 0.05386179, 2.48599187, 49.95424423, 92.59887831, 113.66242448};
    OrbitalDerivatives satDerivs = {-0.00003065, -0.00032044, 0.00451969, 1222.11494724, 0.54179478, -0.25015002, 0.00025899, -0.13434469, 0.87320147, 38.35125000};
    RotationParameters satRots = {79.5280, 61.9480, 38.9000, 810.793902};

    InitialParameters uranusParams = {largeRadiusScale*0.0001695345, semiMajScale*19.18916464, 0.04725744, 0.77263783, 313.23810451, 170.95427630, 74.01692503};
    OrbitalDerivatives uranDerivs = {-0.00020455, -0.00001550, -0.00180155, 428.49512595, 0.09266985, 0.05739699, 0.00058331, -0.97731848, 0.17689245, 7.67025000};
    RotationParameters uranRots = {257.6470, 7.7220, 203.8100, -501.160093};

    InitialParameters neptuneParams = {largeRadiusScale*0.000164587904, semiMajScale*30.06992276, 0.00859048, 1.77004347, -55.12002969, 44.96476227, 131.78422574};
    OrbitalDerivatives neptDerivs = {0.00006447, 0.00000818, 0.00022400, 218.46515314, 0.01009938, -0.00606302, -0.00041348, 0.68346318, -0.10162547, 7.67025000};
    RotationParameters neptRots = {318.7070, 61.5270, 253.1800, 536.312866};

    std::string baseObjects = "res/objects/";
    Planet Mercury((baseObjects + "mercury/mercury.glb").c_str(), mercuryParams, mercuryDerivs, mercuryRots);
    loader.render();
    Planet Venus((baseObjects + "venus/venus.glb").c_str(), venusParams, venusDerivs, venusRots);
    loader.render();
    Planet Earth((baseObjects + "earth/earth.glb").c_str(), earthParams, earthDerivs, earthRots);
    loader.render();
    Planet Mars((baseObjects + "mars/mars.glb").c_str(), marsParams, marsDerivs, marsRots);
    loader.render();
    Planet Jupiter((baseObjects + "jupiter/jupiter.glb").c_str(), jupiterParams, jupDerivs, jupRots);
    loader.render();
    Planet Saturn((baseObjects + "saturn/saturn.glb").c_str(), saturnParams, satDerivs, satRots);
    loader.render();
    Planet Uranus((baseObjects + "uranus/uranus.glb").c_str(), uranusParams, uranDerivs, uranRots);
    loader.render();
    Planet Neptune((baseObjects + "neptune/neptune.glb").c_str(), neptuneParams, neptDerivs, neptRots);
    loader.render();
    Object Sun((baseObjects + "sun/sun.glb").c_str());
    loader.render();
    
    
    std::string baseShaders = "res/shaders/";
    Shader curveShader(
        (baseShaders + "parametric/vertex.vs").c_str(),
        (baseShaders + "parametric/fragment.fs").c_str(),
        (baseShaders + "parametric/geometry.gs").c_str()
    );
    loader.render();
    
    Shader skyboxShader(
        (baseShaders + "skybox/skybox.vs").c_str(),
        (baseShaders + "skybox/skybox.fs").c_str()
    );
    loader.render();

    Skybox skybox("res/textures/skybox/stars/");
    loader.render();
    Skybox constSkybox("res/textures/skybox/constellations/");
    loader.render();
    Skybox constBounds("res/textures/skybox/bounds/");
    loader.render();
    Skybox skyGrid("res/textures/skybox/grid/");
    loader.render();


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

    std::vector<std::tuple<std::string, Planet*>> planets = {
        {"Mercury", &Mercury},
        {"Venus", &Venus},
        {"Earth", &Earth},
        {"Mars", &Mars},
        {"Jupiter", &Jupiter}, 
        {"Saturn", &Saturn},
        {"Uranus", &Uranus},
        {"Neptune", &Neptune}
    };
    
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

    auto now = std::chrono::system_clock::now();
    timeReal = (double)std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    timezoneOffsetHours = GetSystemTimezoneOffset();

    ImPlot::CreateContext();
    
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

        // 1. Advance Simulation Time
        timeReal += deltaTime * (signbit(timeMultiplier) ? -1 : 1) * pow(fabs(timeMultiplier), 7.30103);
        
        Input::update(deltaTime);
        Input::syncCursor();

        // 2. Clear Buffers
        glClearColor(0, 0, 0, 1.0f);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 3. Update Planets (Ensures physics are fresh for this frame)
        for (auto& [name, planet] : planets) {
            planet->calcMeanAnom(timeReal);
            planet->solveEccAnom(timeReal);
            planet->update(timeReal);
        }

        // 4. Handle Deactivation of Camera Tracking via Input
        if (followPlanet && 
            (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_SPACE) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {

            followPlanet = false;
            activeFollowPlanet = nullptr;
        }

        // 5. Update Camera Position (Glues camera to the freshly updated planet)
        if (followPlanet && activeFollowPlanet) {
            glm::vec3 planetPos = activeFollowPlanet->planet.getPosition();
            float radius = activeFollowPlanet->initParams.r;
            camera.setPosition(planetPos + glm::vec3(0.0f, 1.2f * radius, 0.0f));
        }

        // 6. Set Up Projection Matrix
        glm::mat4 proj = glm::mat4(0.0f);
        {
            float n = 0.1;
            float r = n * tan(glm::radians(camera.Zoom));
            float t = r * (float)HEIGHT/WIDTH;
            proj[0][0] = n / r; proj[1][1] = n / t; proj[2][2] = -1; proj[2][3] = -1; proj[3][2] = -2 * n;
        }

        // 7. Draw Orbital Curves
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

        light.color = glm::vec3(cols[0], cols[1], cols[2]);
        light.position = Sun.getPosition();
        
        // 8. RENDER 3D SCENE HERE (Before ImGui)
        renderer.render(scene, camera, light, window.getWidth(), window.getHeight(), proj);

        // 8.5 im lwk rendering the skybox ////////////////
        glDepthFunc(GL_LEQUAL);
        glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMat()));
        glm::mat4 skyboxRotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        skyboxRotation = glm::rotate(skyboxRotation, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        skyboxRotation = glm::rotate(skyboxRotation, glm::radians(90.0f - (float)earthRots.b0), glm::vec3(0.0f, 0.0f, 1.0f));
        view = view * skyboxRotation;
        
        if (showSky) skybox.render(skyboxShader, view, proj);
        if (showConstellations) {
            glBlendFunc(GL_ONE, GL_ONE);
            constSkybox.render(skyboxShader, view, proj);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (showConstBounds) {
            glBlendFunc(GL_ONE, GL_ONE);
            constBounds.render(skyboxShader, view, proj);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (showGrid) {
            glBlendFunc(GL_ONE, GL_ONE);
            skyGrid.render(skyboxShader, view, proj);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        glDepthFunc(GL_LESS);
        /////////////////////////////////////////////

        // 9. Draw ImGui HUD on top of the 3D Scene
        ImGuiLayer::begin();
        ImGui::Begin("Debug");
        ImGui::Text("FPS: %.2f", fps);
        ImGui::Text("Camera pos: X: %.2f, Y: %.2f, Z: %.2f", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::Text("Camera rot: X: %.2f, Y: %.2f, Z: %.2f", camera.Front.x, camera.Front.y, camera.Front.z);
        ImGui::Text("Pitch: %.2f Yaw: %.2f", camera.Pitch, camera.Yaw);
        ImGui::SliderFloat("Speed", speed, 500, 10000);
        ImGui::SliderFloat("Camera FOV", camFOV, 1, 89);
        ImGui::Checkbox("Draw Curves?", &curves);
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Skybox options")) {
            ImGui::Checkbox("Show sky", &showSky);
            ImGui::Checkbox("Show constellations", &showConstellations);
            ImGui::Checkbox("Show constellation boundaries", &showConstBounds);
            ImGui::Checkbox("Show grid", &showGrid);
        }
        ImGui::Separator();
        ImGui::Text("Time (s): %f", timeReal);
        // ImGui::Checkbox("Cross-view", &crossView);
        ImGui::Separator();
        if (ImGui::Button("Change Date/Time")) {
            ImGui::OpenPopup("SelectDateTime");
        }

        // Pass context safely into the picker loop
        ImPlot_DateTimePicker("SelectDateTime", timeReal, timezoneOffsetHours);

        // Print out current synchronized timestamp to screen
        std::string display_str = TimestampToString(timeReal, timezoneOffsetHours);
        ImGui::Text("Current Global Time: %s", display_str.c_str());
        // ImGui::SliderFloat("Semi-major axis", &Earth.semiMaj, 0.1, 50);
        // ImGui::SliderFloat("Eccentricity", &Earth.ecc, 0.0f, 1.0f);
        ImGui::SliderFloat("Exponential Time Multiplier", &timeMultiplier, -10, 10);
        ImGui::Text("Actual Time Multiplier: %f", (signbit(timeMultiplier) ? -1 : 1) * pow(fabs(timeMultiplier), 7.30103));
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
        /*if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(500);
            if (ImGui::DragFloat3("Position", &objectPosition[0], 0.01f)) {
                objects[selectedObjectIndex].get().setPosition(objectPosition);
            }
            ImGui::PopItemWidth();
            if (ImGui::Button("Snap to origin")) {
                objectPosition = glm::vec3(0.0f);
                objects[selectedObjectIndex].get().setPosition(objectPosition);
            }
        }*/
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
        /*ImGui::Separator();
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
        }*/


        ImGui::End();


        if (followPlanet && 
            (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_SPACE) == GLFW_PRESS ||
            glfwGetKey(window.get(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {

            followPlanet = false;
            activeFollowPlanet = nullptr;
        }

        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Navigation Panel");
        ImGui::PushItemWidth(500.0f);
        ImGui::Separator();

        ImGui::Checkbox("Follow Destination Planet", &followPlanet);
        if (!followPlanet) {
            activeFollowPlanet = nullptr; // Clear target if they uncheck it manually
        }

        if (ImGui::BeginListBox("##PlanetList", ImVec2(-1.0f, 250.0f))) {
            
            for (int i = 0; i < planets.size(); i++) {
                const bool isSelected = (selectedPlanetIdx == i);
                std::string planetName = std::get<0>(planets[i]);
                
                if (ImGui::Selectable(planetName.c_str(), isSelected)) {
                    selectedPlanetIdx = i;
                    
                    Planet* p = std::get<1>(planets[selectedPlanetIdx]);
                    
                    if (p) { 
                        glm::vec3 planetPos = p->planet.getPosition();
                        float radius = p->initParams.r;
                        camera.setPosition(planetPos + glm::vec3(0.0f, 1.2 * radius, 0.0f));
                        
                        if (followPlanet) {
                            activeFollowPlanet = p;
                        }
                    }
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
        ImGui::PopItemWidth();
        ImGui::End();

        if (followPlanet && activeFollowPlanet) {
            glm::vec3 planetPos = activeFollowPlanet->planet.getPosition();
            float radius = activeFollowPlanet->initParams.r;
            
            camera.setPosition(planetPos + glm::vec3(0.0f, 1.2 * radius, 0.0f));
        }



        ImGuiLayer::end();
        
        window.swapBuffers();
        window.pollEvents();
        
    }
    
    ImPlot::DestroyContext();
    ImGuiLayer::shutdown();

    glfwTerminate();
    return 0;
}