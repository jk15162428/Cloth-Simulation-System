#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum CameraMovement 
{
    DIR_FORWARD,
    DIR_BACKWARD,
    DIR_LEFT,
    DIR_RIGHT
};

class Camera
{
private:
    const float frustumRatio = 1.0f;
    const float moveSpeed = 0.25f;

public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;

    float Zoom = 45.0f;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 30.0f), 
           glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        Position = position;
        Front = front;
        Up = up;
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    glm::mat4 GetProjectionMatrix()
    {
        return glm::perspective(glm::radians(Zoom), frustumRatio, 0.1f, 100.0f);
    }

    void ProcessKeyboard(CameraMovement direction)
    {
        if (direction == DIR_FORWARD)
            Position.y += moveSpeed;
        if (direction == DIR_BACKWARD)
            Position.y -= moveSpeed;
        if (direction == DIR_LEFT)
            Position -= glm::normalize(glm::cross(Front, Up)) * moveSpeed;
        if (direction == DIR_RIGHT)
            Position += glm::normalize(glm::cross(Front, Up)) * moveSpeed;
    }

    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 90.0f)
            Zoom = 90.0f;
    }
};
Camera camera;