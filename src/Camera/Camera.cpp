#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 up, float yaw, float pitch)
 : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSense(SENSITIVITY), Zoom(ZOOM) {
    
    Position = pos;
    Up = up;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCamVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
 : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSense(SENSITIVITY), Zoom(ZOOM) {

    Position = glm::vec3(posX, posY, posZ);
    Up = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCamVectors();
}

glm::mat4 Camera::getViewMat() { return glm::lookAt(Position, Position + Front, Up); }

void Camera::processKeys(CameraMovement direction, float deltaTime) {

    float vel = MovementSpeed * deltaTime;
    glm::vec3 tempFront = Front;
    tempFront.y = 0;
    tempFront = glm::normalize(tempFront);
    if (direction == FORWARD) { Position += tempFront * vel; }
    if (direction == BACKWARD) { Position -= tempFront * vel; }
    if (direction == RIGHT) { Position += Right * vel; }
    if (direction == LEFT) { Position -= Right * vel; }
    if (direction == UP) { Position += WorldUp * vel; }
    if (direction == DOWN) { Position -= WorldUp * vel; }
}

void Camera::processMouse(float xOffset, float yOffset, GLboolean constrainPitch) {

    xOffset *= MouseSense;
    yOffset *= MouseSense;

    Yaw = glm::mod(Yaw + xOffset, 360.0f);
    Pitch += yOffset;

    if (constrainPitch) {
        if (Pitch > 89.0f) {
            Pitch = 89.0f;
        }
        if (Pitch < -89.0f) {
            Pitch = -89.0f;
        }
    }
    updateCamVectors();
}

void Camera::processScroll(float yOffset) {
    Zoom -= (float)yOffset;
    if (Zoom < 1.0f) {
        Zoom = 1.0f;
    }
    if (Zoom > 45.0f) {
        Zoom = 45.0f;
    }
}

void Camera::updateCamVectors() {

    glm::vec3 newFront;
    newFront.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    newFront.y = sin(glm::radians(Pitch));
    newFront.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(newFront);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}