#include "Renderer.h"
#include <glad/glad.h>

void Renderer::render(Scene &scene, Camera &camera, const Light &light, int w, int h, glm::mat4 proj) {
    glm::mat4 view = camera.getViewMat();
    // glm::mat4 proj = glm::perspective(
    //     glm::radians(camera.Zoom),
    //     (float)w / h,
    //     0.1f,
    //     100.0f
    // );

    for (auto &objRef : scene.getObjects()) {
        Object &obj = objRef.get();
        std::shared_ptr<Shader> shader= obj.shader;

        shader->use();
        shader->setVec3("lightCol", light.color);
        shader->setVec3("lightPos", light.position);
        shader->setVec3("viewPos", camera.Position);

        obj.Draw(view, proj);
    }
}