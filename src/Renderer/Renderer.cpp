#include "Renderer.h"
#include <glad/glad.h>

void Renderer::render(Scene &scene, Camera &camera, const Light &light, int w, int h, glm::mat4 proj) {
    // glm::mat4 view = camera.getViewMat();
    glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMat()));

    for (auto &objRef : scene.getObjects()) {
        Object &obj = objRef.get();
        std::shared_ptr<Shader> shader= obj.shader;

        shader->use();
        shader->setVec3("lightCol", light.color);
        shader->setVec3("lightPos", light.position - (glm::vec3)camera.Position);
        shader->setVec3("viewPos", glm::vec3(0.0f));

        obj.Draw(view, proj);
    }
}