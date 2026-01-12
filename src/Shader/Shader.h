#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../vendor/stb_image/stb_image.h"
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"


class Shader {

    public:

        unsigned int ID;
        

        Shader(const char* vPath, const char *fPath);
        
        void use();

        void setBool(const std::string &name, bool value);
        void setInt(const std::string &name, int value);
        void setFloat(const std::string &name, float value);
        void setVec3(const std::string &name, glm::vec3 value);
        void setMat4(const std::string &name, glm::mat4 value);
};