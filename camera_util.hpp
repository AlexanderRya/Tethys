#ifndef TEST_CAMERA_UTIL_HPP
#define TEST_CAMERA_UTIL_HPP

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraUtil {
    static constexpr float speed = 2.f;
    static constexpr float sensitivity = 8.0e-2f;

    double yaw;
    double pitch;

public:
    glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cam_right = glm::vec3();
    constexpr static glm::vec3 cam_wup = glm::vec3(0.0f, 1.0f, 0.0f);

    CameraUtil() noexcept;
    void move(float delta_time);
    [[nodiscard]] glm::mat4 view() const;
    void process(double xoffset, double yoffset);
    void update();
};

#endif //TEST_CAMERA_UTIL_HPP
