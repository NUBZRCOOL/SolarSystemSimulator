#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include "../Mesh/Mesh.h"
#include "../Texture/Texture.h"
#include "../Shader/Shader.h"
#include "../vendor/stb_image/stb_image.h"
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>


class Model {

    public:

        Model(const char* path);
        void Draw(Shader &shader);
        glm::vec3 centerOfMeshes();

    private:

        std::vector<Mesh> meshes;
        std::string directory;

        std::vector<TextStruct> textures_loaded;

        void loadModel(std::string path);
        void processNodes(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<TextStruct> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string name, const aiScene *scene);

        unsigned int TextureFromFile(const char* path, const std::string &directory, const aiScene *scene);
};