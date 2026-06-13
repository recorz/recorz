#include "render/view_matrices.h"

namespace recorz::render {
namespace {

constexpr float kNearPlane = 0.05f;
constexpr float kFarPlane = 100.0f;
constexpr float kRotationSpeed = 1.5f;

} // namespace

math::Mat4 modelMatrixForCube(XrTime displayTime) {
    const float seconds = static_cast<float>(displayTime) * 1.0e-9f;
    const float angle = seconds * kRotationSpeed;

    math::Mat4 model(1.0f);
    model = math::translate(model, math::Vec3(0.0f, 1.4f, -2.0f));
    model = math::rotate(model, angle, math::Vec3(0.0f, 1.0f, 0.0f));
    return model;
}

math::Mat4 mvpForView(const ViewData& view, XrTime displayTime) {
    const math::Mat4 projection = math::projectionFromFov(view.fov, kNearPlane, kFarPlane);
    const math::Mat4 viewMatrix = math::viewMatrixFromPose(view.pose);
    const math::Mat4 model = modelMatrixForCube(displayTime);
    return projection * viewMatrix * model;
}

} // namespace recorz::render
