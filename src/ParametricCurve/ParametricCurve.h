#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include "../vendor/stb_image/stb_image.h"
#include "../vendor/glm/glm.hpp"
#include "../Shader/Shader.h"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"

class ParametricCurve {
    private:

        unsigned int VAO, VBO;
        std::vector<glm::vec3> vertices;
        int segments;
        float tStart, tEnd;
        
        
        public:
        
        void init(float tStart, float tEnd, int segments);
        void render(Shader &shader, glm::mat4 view, glm::mat4 proj, float lineWidth, glm::vec2 viewportSize);
        void updateCurve(float (*p[3])(float));
        
        void generateCurve(float tStart, float tEnd, int segments, float (*p[3])(float));
};