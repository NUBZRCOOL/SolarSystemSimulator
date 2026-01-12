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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


struct Vertex {
    glm::vec3 Position, Normal;
    glm::vec2 TexCoords;
};

struct TextStruct {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {

    public:

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<TextStruct> textures;
        glm::vec3 aabbMin;
        glm::vec3 aabbMax;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextStruct> textures, glm::vec3 min, glm::vec3 max);
        void Draw(Shader &shader);

    private:

        unsigned int VAO, VBO, EBO;
        void setupMesh();
};