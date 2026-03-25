#include "ParametricCurve.h"

void ParametricCurve::init(float tStart, float tEnd, int segments) {
    this->segments = segments;
    this->tStart = tStart;
    this->tEnd = tEnd;

    vertices.clear();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void ParametricCurve::render(Shader &shader, glm::mat4 view, glm::mat4 proj, float lineWidth, glm::vec2 viewportSize) {
    
    if (vertices.empty()) return;

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("proj", proj);
    shader.setFloat("lineWidth", lineWidth);
    shader.setVec2("viewportSize", viewportSize);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertices.size());
    glBindVertexArray(0);
}

void ParametricCurve::generateCurve(float tStart, float tEnd, int segments, std::array<std::function<double(double)>, 3> p) {
    float step = (tEnd - tStart) / segments;
    for (int i = 0; i <= segments; i++) {
        float t = tStart + i * step;
        vertices.push_back(glm::vec3(
           p[0](t),
           p[1](t),
           p[2](t)
        ));
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParametricCurve::updateCurve(std::array<std::function<double(double)>, 3> p) {
    generateCurve(tStart, tEnd, segments, p);
}