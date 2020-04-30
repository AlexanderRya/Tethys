#include "camera_util.hpp"

#include <tethys/window/window.hpp>

#include <GLFW/glfw3.h>

CameraUtil::CameraUtil() noexcept
    : yaw(-90.0f),
    pitch(0.0f) {
    update();
}

void CameraUtil::move(float delta_time) {
    float velocity = speed * delta_time;

    if (tethys::window::key_pressed(GLFW_KEY_W)) {
        cam_pos.x += cos(glm::radians(yaw)) * velocity;
        cam_pos.z += sin(glm::radians(yaw)) * velocity;
    }

    if (tethys::window::key_pressed(GLFW_KEY_S)) {
        cam_pos.x -= cos(glm::radians(yaw)) * velocity;
        cam_pos.z -= sin(glm::radians(yaw)) * velocity;
    }

    if (tethys::window::key_pressed(GLFW_KEY_A)) {
        cam_pos -= cam_right * velocity;
    }

    if (tethys::window::key_pressed(GLFW_KEY_D)) {
        cam_pos += cam_right * velocity;
    }

    if (tethys::window::key_pressed(GLFW_KEY_SPACE)) {
        cam_pos += cam_wup * velocity;
    }

    if (tethys::window::key_pressed(GLFW_KEY_LEFT_SHIFT)) {
        cam_pos -= cam_wup * velocity;
    }

    update();
}

glm::mat4 CameraUtil::view() const {
    return glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);
}

void CameraUtil::process(double xoffset, double yoffset) {
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }

    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    update();
}

void CameraUtil::update() {
    glm::vec3 front{
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };

    cam_front = glm::normalize(front);
    cam_right = glm::normalize(glm::cross(cam_front, cam_wup));
    cam_up = glm::normalize(glm::cross(cam_right, cam_front));
}