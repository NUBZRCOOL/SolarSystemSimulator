#pragma once
#define GLFW_INCLUDE_NONE
#include "../Scene/Scene.h"
#include "../Object/Object.h"
#include "../Camera/Camera.h"

struct Light {
    glm::vec3 position;
    glm::vec3 color;
};

class Renderer {
    public:
        void render(Scene &scene, Camera &camera, const Light &light, int w, int h);
};