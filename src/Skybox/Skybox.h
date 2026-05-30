#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include "../Texture/Texture.h"
#include "../Shader/Shader.h"
#include "../vendor/stb_image/stb_image.h"
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"

class Skybox {

public:

    Skybox(const char *path);
    ~Skybox();

    void render(Shader &shader, glm::mat4 view, glm::mat4 proj);

    unsigned int loadCubeMap(std::vector<std::string> faces);

    const char *path;
    unsigned int skyboxVBO, skyboxVAO, cubeMapTexture;
};