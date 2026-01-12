#include "Scene.h"
#include <glad/glad.h>

void Scene::add(Object &obj) {
    objects.push_back(obj);
    objectNames.push_back(obj.getName());
}

void Scene::draw(Camera &cam, int w, int h) {
    glm::mat4 view = cam.getViewMat();
    glm::mat4 proj = glm::perspective(
        glm::radians(cam.Zoom),
        (float)w / h,
        0.1f,
        100.0f
    );

    for (auto &obj : objects) {
        obj.get().Draw(view, proj);
    }
}

std::vector<std::reference_wrapper<Object>> &Scene::getObjects() { return objects; }

std::vector<std::string> Scene::getObjectNames() { return objectNames; }