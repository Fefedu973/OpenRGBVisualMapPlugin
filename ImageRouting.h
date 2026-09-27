/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include "ControllerZone.h"
#include "LedRouting.h"
#include <FrameRouting/RGBControllerImageInterface.h>
#include <QSize>
#include <set>

namespace visual_image
{
// Scene coordinates never depend on this compatibility grid or on source pixels.
QSize CompatibilitySize(unsigned width, unsigned height);
room_image::Mapping Compose(const room_image::Mapping& outer, const room_image::Mapping& inner);
struct Sample { unsigned led; double u, v; };
struct Plan
{
    std::vector<Sample> samples;
    bool affine_surface = false;
    room_image::Mapping surface;
    double brightness = 1.0;
};
Plan BuildPlan(const ControllerZone* zone, unsigned scene_width, unsigned scene_height);
std::shared_ptr<const room_image::Frame> FromImage(const QImage& image, uint64_t sequence);
QImage Preview(const room_image::Frame& frame, const room_image::Mapping& mapping, QSize scene);
}
