#pragma once

// Temporary math wrapper (using GLM for now)
// Goal: Keep GLM usage isolated so it can be replaced later with custom SIMD math.

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace recorz::math {

using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
using Quat = glm::quat;

// Common matrix helpers (we can extend these later)
inline Mat4 perspective(float fovy, float aspect, float near, float far) {
    return glm::perspective(fovy, aspect, near, far);
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

} // namespace recorz::math