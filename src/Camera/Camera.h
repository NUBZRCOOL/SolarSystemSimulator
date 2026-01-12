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


enum CameraMovement {
    FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
};

const float YAW         = -90.0;
const float PITCH       = 0.0;
const float SPEED       = 3.5;
const float SENSITIVITY = 0.07;
const float ZOOM        = 45.0;

class Camera {

    public:

        glm::vec3 Position, Front, Up, Right, WorldUp;
        float Yaw, Pitch, MovementSpeed, MouseSense, Zoom;

        Camera(glm::vec3 pos, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
        Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
        
        glm::mat4 getViewMat();

        void processKeys(CameraMovement direction, float deltaTime);
        void processMouse(float xOffset, float yOffset, GLboolean constrainPitch = true);
        void processScroll(float yOffset);

    private:

        void updateCamVectors();
};