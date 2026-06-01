#pragma once

#include "../Shader/Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class LoadingScreen {
public:

    LoadingScreen() {};

    void init(int width, int height, const char *texturePath, Shader shader);

    bool loadTexture(const char *path);

    void setupQuad();

    void render();

    void shutdown();


    Shader shader;
    GLuint texID = 0;
    GLuint VAO = 0, VBO = 0;
    int windowWidth, windowHeight;
    bool initialized = false;
};