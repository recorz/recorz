#pragma once

// Temporary math wrapper (using GLM for now)
// Goal: Keep GLM usage isolated so it can be replaced later with custom SIMD math.

#include <openxr/openxr.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

namespace recorz::math {

using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
using Quat = glm::quat;

inline Mat4 perspective(float fovy, float aspect, float nearZ, float farZ) {
    return glm::perspective(fovy, aspect, nearZ, farZ);
}

inline Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    return glm::lookAt(eye, center, up);
}

inline Mat4 translate(const Mat4& m, const Vec3& v) {
    return glm::translate(m, v);
}

inline Mat4 rotate(const Mat4& m, float angle, const Vec3& axis) {
    return glm::rotate(m, angle, axis);
}

inline Mat4 mat4FromPose(const XrPosef& pose) {
    const Quat orientation(
        pose.orientation.w,
        pose.orientation.x,
        pose.orientation.y,
        pose.orientation.z);
    Mat4 matrix = glm::mat4_cast(orientation);
    matrix[3] = Vec4(pose.position.x, pose.position.y, pose.position.z, 1.0f);
    return matrix;
}

inline Mat4 viewMatrixFromPose(const XrPosef& pose) {
    return glm::inverse(mat4FromPose(pose));
}

// Asymmetric projection from OpenXR per-eye FOV (Vulkan depth range 0..1).
inline Mat4 projectionFromFov(const XrFovf& fov, float nearZ, float farZ) {
    const float tanLeft = std::tan(fov.angleLeft);
    const float tanRight = std::tan(fov.angleRight);
    const float tanDown = std::tan(fov.angleDown);
    const float tanUp = std::tan(fov.angleUp);

    const float tanWidth = tanRight - tanLeft;
    const float tanHeight = tanUp - tanDown;

    Mat4 matrix(0.0f);
    matrix[0][0] = 2.0f / tanWidth;
    matrix[1][1] = -2.0f / tanHeight;
    matrix[2][0] = (tanRight + tanLeft) / tanWidth;
    matrix[2][1] = (tanUp + tanDown) / tanHeight;
    matrix[2][2] = farZ / (nearZ - farZ);
    matrix[2][3] = -1.0f;
    matrix[3][2] = (farZ * nearZ) / (nearZ - farZ);
    return matrix;
}

} // namespace recorz::math
