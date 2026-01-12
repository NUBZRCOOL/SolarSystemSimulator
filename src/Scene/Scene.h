#pragma once
#define GLFW_INCLUDE_NONE
#include <vector>
#include "../Object/Object.h"
#include "../Camera/Camera.h"

class Scene {
    public:

        void add(Object &obj);
        void draw(Camera &cam, int w, int h);

        std::vector<std::reference_wrapper<Object>> &getObjects();
        std::vector<std::string> getObjectNames();

    private:

        std::vector<std::reference_wrapper<Object>> objects;
        std::vector<std::string> objectNames;
};