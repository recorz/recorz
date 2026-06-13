#pragma once

#include "math.hpp"
#include "render/frame_packet.h"

namespace recorz::render {

math::Mat4 modelMatrixForCube(XrTime displayTime);
math::Mat4 mvpForView(const ViewData& view, XrTime displayTime);

} // namespace recorz::render
