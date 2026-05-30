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

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))


struct DatePickerState {
    bool is_initialized = false;
    std::tm stored_tm = {};   // Track calendar components cleanly
    double stored_ms = 0.0;   // Track precise sub-seconds
    int picker_level = 0;
};


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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline time_t timegm(std::tm *tm) // calculate seconds without timezone
{
#ifdef _WIN32
    return _mkgmtime(tm);
#else
    return timegm(tm);
#endif
}

double StringToTimestamp(const std::string& datetime_str) {
    std::tm tm = {};
    std::stringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    std::time_t s = timegm(&tm);
    double ms = 0.0;
    
    size_t dot = datetime_str.find_first_of('.');
    if (dot != std::string::npos) {
        ms = std::stod("0." + datetime_str.substr(dot + 1));
    }
    return static_cast<double>(s) + ms;
}

// Formats Unix timestamp back to UTC string 
std::string TimestampToString(double t) {
    std::time_t s = static_cast<std::time_t>(t);
    int ms = static_cast<int>(std::round((t - s) * 1000.0));
    if (ms >= 1000) { s += 1; ms -= 1000; } // Handle rounding overflows
    
    std::tm *tm = std::gmtime(&s);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    
    std::stringstream ss;
    ss << buffer << "." << std::setw(3) << std::setfill('0') << ms;
    return ss.str();
}

void ImPlot_DateTimePicker(const char* popup_id, double& current_unix_time, int timezone_offset_hours) {
    if (ImGui::BeginPopup(popup_id)) {
        
        // Use a clean, simple static state container that mirrors standard GUI logic
        static bool is_initialized = false;
        static int sel_year = 2000, sel_month = 0, sel_day = 1;
        static int sel_hour = 0, sel_min = 0, sel_sec = 0;
        static int sel_ms = 0;

        // 1. First frame layout sync
        if (!is_initialized) {
            double local_time = current_unix_time + (timezone_offset_hours * 3600.0);
            std::time_t full_s = static_cast<std::time_t>(std::floor(local_time));
            double sub_seconds = local_time - full_s;
            
            std::tm* tm_local = std::gmtime(&full_s);
            if (tm_local) {
                sel_year  = tm_local->tm_year + 1900;
                sel_month = tm_local->tm_mon; // 0-11
                sel_day   = tm_local->tm_mday;
                sel_hour  = tm_local->tm_hour;
                sel_min   = tm_local->tm_min;
                sel_sec   = tm_local->tm_sec;
                sel_ms    = static_cast<int>(sub_seconds * 1000.0 + 0.5);
            }
            is_initialized = true;
        }

        // 2. NATIVE IMGUI MONTH & YEAR PICKERS (Guaranteed not to freeze or snap back)
        ImGui::Text("Date Selection");
        
        // Year Input
        ImGui::PushItemWidth(6 * ImGui::GetFontSize());
        ImGui::InputInt("##year_input", &sel_year, 1, 10);
        if (sel_year < 1) sel_year = 1;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Month Dropdown
        const char* months[] = { "January", "February", "March", "April", "May", "June", 
                                 "July", "August", "September", "October", "November", "December" };
        ImGui::PushItemWidth(8 * ImGui::GetFontSize());
        if (ImGui::BeginCombo("##month_combo", months[sel_month])) {
            for (int i = 0; i < 12; i++) {
                bool is_selected = (sel_month == i);
                if (ImGui::Selectable(months[i], is_selected)) {
                    sel_month = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Day Input
        // Calculate max days allowed in the chosen month
        std::tm days_lookup = {};
        days_lookup.tm_year = sel_year - 1900;
        days_lookup.tm_mon = sel_month + 1; // Next month
        days_lookup.tm_mday = 0;            // Day 0 of next month = Last day of current month
        timegm(&days_lookup);
        int max_days = days_lookup.tm_mday;

        ImGui::PushItemWidth(4 * ImGui::GetFontSize());
        ImGui::InputInt("##day_input", &sel_day, 1, 5);
        if (sel_day < 1) sel_day = 1;
        if (sel_day > max_days) sel_day = max_days;
        ImGui::PopItemWidth();

        ImGui::Separator();

        // 3. TIME PICKER
        ImGui::Text("Time Selection");
        
        // Construct temporary frame structure to feed ImPlot's visual clock interface safely
        std::tm frame_tm = {};
        frame_tm.tm_year = sel_year - 1900;
        frame_tm.tm_mon  = sel_month;
        frame_tm.tm_mday = sel_day;
        frame_tm.tm_hour = sel_hour;
        frame_tm.tm_min  = sel_min;
        frame_tm.tm_sec  = sel_sec;
        
        ImPlotTime im_time = ImPlotTime::FromDouble(static_cast<double>(timegm(&frame_tm)) + (sel_ms / 1000.0));
        ImPlot::GetStyle().Use24HourClock = true;
        
        if (ImPlot::ShowTimePicker("##time", &im_time)) {
            double t_val = im_time.ToDouble();
            std::time_t t_secs = static_cast<std::time_t>(std::floor(t_val));
            std::tm* decoded = std::gmtime(&t_secs);
            if (decoded) {
                sel_hour = decoded->tm_hour;
                sel_min  = decoded->tm_min;
                sel_sec  = decoded->tm_sec;
            }
        }

        // Millisecond precision sub-slider
        ImGui::SameLine(); ImGui::Text("."); ImGui::SameLine();
        ImGui::PushItemWidth(4 * ImGui::GetFontSize());
        if (ImGui::InputInt("##ms", &sel_ms, 0, 0)) {
            if (sel_ms < 0) sel_ms = 0;
            if (sel_ms > 999) sel_ms = 999;
        }
        ImGui::PopItemWidth();

        ImGui::Separator();

        // 4. DIALOG PROCESSING BUTTONS
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            std::tm final_tm = {};
            final_tm.tm_year = sel_year - 1900;
            final_tm.tm_mon  = sel_month;
            final_tm.tm_mday = sel_day;
            final_tm.tm_hour = sel_hour;
            final_tm.tm_min  = sel_min;
            final_tm.tm_sec  = sel_sec;

            std::time_t final_secs = timegm(&final_tm);
            double final_local_time = static_cast<double>(final_secs) + (sel_ms / 1000.0);
            
            // Adjust back to global simulation time scale removing local offset configuration
            current_unix_time = final_local_time - (timezone_offset_hours * 3600.0);
            
            is_initialized = false; // Reset setup trigger flag for subsequent popup clicks
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            is_initialized = false; // Discard changes cleanly
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {

    Window window(WIDTH, HEIGHT, "Solar System Simulation");

    Input::init(window.get(), &camera);
    ImGuiLayer::init(window.get());
    ImGuiLayer::setFontScale(2.0f);

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
    Planet Venus((baseObjects + "venus/venus.glb").c_str(), venusParams, venusDerivs, venusRots);
    Planet Earth((baseObjects + "earth/earth.glb").c_str(), earthParams, earthDerivs, earthRots);
    Planet Mars((baseObjects + "mars/mars.glb").c_str(), marsParams, marsDerivs, marsRots);
    Planet Jupiter((baseObjects + "jupiter/jupiter.glb").c_str(), jupiterParams, jupDerivs, jupRots);
    Planet Saturn((baseObjects + "saturn/saturn.glb").c_str(), saturnParams, satDerivs, satRots);
    Planet Uranus((baseObjects + "uranus/uranus.glb").c_str(), uranusParams, uranDerivs, uranRots);
    Planet Neptune((baseObjects + "neptune/neptune.glb").c_str(), neptuneParams, neptDerivs, neptRots);
    Object Sun((baseObjects + "sun/sun.glb").c_str());
    
    
    std::string baseShaders = "res/shaders/";
    Shader curveShader(
        (baseShaders + "parametric/vertex.vs").c_str(),
        (baseShaders + "parametric/fragment.fs").c_str(),
        (baseShaders + "parametric/geometry.gs").c_str()
    );

    Shader skyboxShader(
        (baseShaders + "skybox/skybox.vs").c_str(),
        (baseShaders + "skybox/skybox.fs").c_str()
    );
    Skybox skybox("res/textures/skybox/");


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

        timeReal += deltaTime * (signbit(timeMultiplier) ? -1 : 1) * pow(fabs(timeMultiplier), 7.30103);
        
        Input::update(deltaTime);
        Input::syncCursor();
        glClearColor(0, 0, 0, 1.0f);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        light.color = glm::vec3(cols[0], cols[1], cols[2]);
        light.position = Sun.getPosition();

        for (auto& [name, planet] : planets) {
            planet->calcMeanAnom(timeReal);
            planet->solveEccAnom(timeReal);
            planet->update(timeReal);
        }
       
        // glm::mat4 proj;
        // proj = glm::perspective(
        //     glm::radians(camera.Zoom),
        //     (float)window.getWidth() / (float)window.getHeight(),
        //     0.1f,
        //     100000.0f
        // );

        glm::mat4 proj = glm::mat4(0.0f);
        {
            float n = 0.1;
            float r = n * tan(glm::radians(camera.Zoom));
            float t = r * (float)HEIGHT/WIDTH;
            proj[0][0] = n / r; proj[1][1] = n / t; proj[2][2] = -1; proj[2][3] = -1; proj[3][2] = -2 * n;
        }

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

        glDepthFunc(GL_LEQUAL);
        glm::mat4 skyView = glm::mat4(glm::mat3(camera.getViewMat()));
        skybox.render(skyboxShader, skyView, proj);
        glDepthFunc(GL_LESS);

        ImGuiLayer::begin();
        ImGui::Begin("Debug");
        ImGui::Text("FPS: %.2f", fps);
        ImGui::Text("Camera pos: X: %.2f, Y: %.2f, Z: %.2f", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::Text("Camera rot: X: %.2f, Y: %.2f, Z: %.2f", camera.Front.x, camera.Front.y, camera.Front.z);
        ImGui::Text("Pitch: %.2f Yaw: %.2f", camera.Pitch, camera.Yaw);
        ImGui::SliderFloat("Speed", speed, 500, 10000);
        ImGui::SliderFloat("Camera FOV", camFOV, 1, 89);
        ImGui::Checkbox("Draw Curves?", &curves);
        ImGui::Text("Time (s): %f", timeReal);
        // ImGui::Checkbox("Cross-view", &crossView);
        ImGui::Separator();
        if (ImGui::Button("Change Date/Time")) {
            ImGui::OpenPopup("SelectDateTime");
        }

        // Pass context safely into the picker loop
        ImPlot_DateTimePicker("SelectDateTime", timeReal, -7);

        // Print out current synchronized timestamp to screen
        std::string display_str = TimestampToString(timeReal);
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

        // Static variable to remember the current selection index
        static int selectedPlanetIdx = 0; 
        // Set a default window size so it isn't tiny
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        ImGui::Begin("Navigation Panel");
        ImGui::PushItemWidth(500.0f);
        ImGui::Text("Select Destination:");
        ImGui::Separator();

        // Make the list box take up the full available width (-1.0f) 
        // and a specific height (e.g., 250 pixels)
        if (ImGui::BeginListBox("##PlanetList", ImVec2(-1.0f, 250.0f))) {
            
            for (int i = 0; i < planets.size(); i++) {
                const bool isSelected = (selectedPlanetIdx == i);
                std::string planetName = std::get<0>(planets[i]);
                
                // Render each planet as a selectable item
                if (ImGui::Selectable(planetName.c_str(), isSelected)) {
                    selectedPlanetIdx = i;
                    
                    // Trigger the teleportation logic immediately upon click
                    Planet* p = std::get<1>(planets[selectedPlanetIdx]);
                    
                    if (p) { 
                        glm::vec3 planetPos = p->planet.getPosition();
                        float radius = p->initParams.r;
                        
                        camera.setPosition(planetPos + glm::vec3(0.0f, 2 * radius, 0.0f));
                        //camTarget = planetPos; 
                    }
                }

                // Keep the active selection in view if the list scrolls
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
        ImGui::PopItemWidth();
        ImGui::End();
        ImGuiLayer::end();
        
        window.swapBuffers();
        window.pollEvents();
    }
    
    ImPlot::DestroyContext();
    ImGuiLayer::shutdown();

    glfwTerminate();
    return 0;
}