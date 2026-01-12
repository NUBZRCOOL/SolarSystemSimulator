#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include "../Model/Model.h"
#include "../Texture/Texture.h"
#include "../Shader/Shader.h"
#include "../vendor/stb_image/stb_image.h"
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class Object {

    public:

        Object(const char *path);

        void Draw(glm::mat4 &view, glm::mat4 &proj);

        void setName(const char *name);
        void setPosition(glm::vec3 vector);
        void setRotation(glm::vec3 vector);
        void setScale(glm::vec3 vector);
        void setOrigin(glm::vec3 pos);

        void parseMaterial();

        std::string getName();
        glm::vec3 getPosition();
        glm::vec3 getRotationEuler();
        glm::vec3 getScale();

        void updateModelMatrix();
        void updateShader();
        std::shared_ptr<Shader> shader;
    private:

        std::string name;
        std::string path;
        Model model;
        glm::mat4 modelMat;

        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
        glm::vec3 pivot;

};