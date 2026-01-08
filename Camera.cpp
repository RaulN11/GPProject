#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

    }
    glm::vec3 Camera::getPosition() const { // Correct definition
        return cameraPosition;
    }
    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += speed * cameraFrontDirection;
                cameraTarget += speed * cameraFrontDirection;
                break;
            case MOVE_BACKWARD:
                cameraPosition -= speed * cameraFrontDirection;
                cameraTarget -= speed * cameraFrontDirection;
                break;
            case MOVE_RIGHT:
                cameraPosition -= speed * cameraRightDirection;
                cameraTarget -= speed * cameraRightDirection;
                break;
            case MOVE_LEFT:
                cameraPosition += speed * cameraRightDirection;
                cameraTarget += speed * cameraRightDirection;
                break;
            case MOVE_UP:
                cameraPosition += speed * cameraUpDirection;
                cameraTarget += speed * cameraUpDirection;
                break;
            case MOVE_DOWN:
                cameraPosition -= speed * cameraUpDirection;
                cameraTarget -= speed * cameraUpDirection;
                break;
        }
    }



    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        pitch *= 0.1f; // Adjust sensitivity
        yaw *= 0.1f;

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), cameraUpDirection);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(pitch), cameraRightDirection);

        cameraFrontDirection = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(cameraFrontDirection, 0.0f)));
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
    void Camera::scale(float factor) {
        // Scale the camera position
        cameraPosition *= factor;

        // Scale the camera target
        cameraTarget *= factor;

        // The up direction might not need to be scaled if you want to maintain its direction
        // cameraUpDirection *= factor;

        // Recalculate front and right directions after scaling
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }



}