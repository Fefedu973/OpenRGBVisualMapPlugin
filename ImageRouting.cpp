/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "ImageRouting.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace visual_image
{
QSize CompatibilitySize(unsigned width, unsigned height)
{
    if(!width || !height) return {};
    // 127² cells keep both LED count and canonical SDK matrix bytes below 65535.
    const double factor = std::min(1.0, 127.0 / std::max(width, height));
    return QSize(std::max(1, int(std::floor(width * factor))),
                 std::max(1, int(std::floor(height * factor))));
}

room_image::Mapping Compose(const room_image::Mapping& a, const room_image::Mapping& b)
{
    room_image::Mapping c;
    a.Point(b.origin_x, b.origin_y, c.origin_x, c.origin_y);
    c.u_x = a.u_x*b.u_x + a.v_x*b.u_y;
    c.u_y = a.u_y*b.u_x + a.v_y*b.u_y;
    c.v_x = a.u_x*b.v_x + a.v_x*b.v_y;
    c.v_y = a.u_y*b.v_x + a.v_y*b.v_y;
    c.brightness = a.brightness*b.brightness;
    return c;
}

Plan BuildPlan(const ControllerZone* zone, unsigned sw, unsigned sh)
{
    Plan result;
    if(!sw || !sh || !LedRouting::ValidGeometry(zone))return result;
    result.brightness=zone->settings.brightness;
    const unsigned count = zone->led_count();
    const auto cells = LedRouting::BuildCells(zone);
    for(const auto& cell : cells)
    {
        if(cell.led_index >= count) continue;
        const QPointF p = cell.local_rect.center() + QPointF(zone->settings.x, zone->settings.y);
        if(std::isfinite(p.x()) && std::isfinite(p.y()))
            result.samples.push_back({cell.led_index, p.x()/sw, p.y()/sh});
    }
    // A segment cannot submit an entire output. Arbitrary custom shapes remain
    // LED routes: an affine rectangle must not paint their holes or missing cells.
    if(zone->is_segment || !zone->isCustomShape() || cells.size() != count) return result;
    auto* controller = zone->controller;
    const unsigned mw = controller->GetZoneMatrixMapWidth(zone->zone_idx);
    const unsigned mh = controller->GetZoneMatrixMapHeight(zone->zone_idx);
    const auto* map = controller->GetZoneMatrixMapData(zone->zone_idx);
    const auto* shape = zone->settings.custom_shape;
    if(!map || mw < 2 || mh < 2 || uint64_t(mw)*mh != count || count > 1024u*1024u
       || shape->w*shape->h != count) return result;
    std::vector<QPointF> placed(count);
    std::vector<bool> known(count, false), source_known(count, false);
    std::set<std::pair<qreal,qreal>> occupied;
    const auto transform=LedRouting::LocalTransform(zone);
    const qreal sample_offset=zone->settings.point_is_center?0.0:0.5;
    for(const auto* point : shape->led_positions)
    {
        if(!point || point->led_num >= count || known[point->led_num]
           || point->point.x() < 0 || point->point.y() < 0
           || !std::isfinite(point->point.x()) || !std::isfinite(point->point.y())
           || point->point.x() >= shape->w || point->point.y() >= shape->h
           || !occupied.emplace(point->point.x(), point->point.y()).second) return result;
        known[point->led_num] = true;
        placed[point->led_num] = transform.map(point->point+QPointF(sample_offset,sample_offset))
                              +QPointF(zone->settings.x,zone->settings.y);
    }
    for(unsigned i=0; i<count; ++i)
    {
        if(map[i] >= count || source_known[map[i]] || !known[map[i]]) return result;
        source_known[map[i]] = true;
    }
    const QPointF p = placed[map[0]], du = placed[map[1]]-p, dv = placed[map[mw]]-p;
    if(std::abs(du.x()*dv.y()-du.y()*dv.x()) < 1e-10) return result;
    for(unsigned y=0; y<mh; ++y)
    for(unsigned x=0; x<mw; ++x)
    {
        const QPointF delta = placed[map[size_t(y)*mw+x]] - (p + du*x + dv*y);
        if(std::abs(delta.x()) > 1e-7 || std::abs(delta.y()) > 1e-7) return result;
    }
    auto& m = result.surface;
    const QPointF origin = p - (du+dv)*0.5;
    m.origin_x = origin.x()/sw; m.origin_y = origin.y()/sh;
    m.u_x = du.x()*mw/sw; m.u_y = du.y()*mw/sh;
    m.v_x = dv.x()*mh/sw; m.v_y = dv.y()*mh/sh;
    m.brightness=result.brightness;
    result.affine_surface = m.Valid();
    return result;
}

std::shared_ptr<const room_image::Frame> FromImage(const QImage& image, uint64_t sequence)
{
    if(image.isNull() || image.width()>16384 || image.height()>16384
       || uint64_t(image.width())*image.height()*4 > room_image::MaxFrameBytes) return {};
    const QImage rgb = image.convertToFormat(QImage::Format_RGB32);
    auto pixels = std::make_shared<std::vector<uint8_t>>(size_t(rgb.width())*rgb.height()*4);
    for(int y=0; y<rgb.height(); ++y)
    {
        const auto* row = reinterpret_cast<const QRgb*>(rgb.constScanLine(y));
        auto* out = pixels->data() + size_t(y)*rgb.width()*4;
        for(int x=0; x<rgb.width(); ++x)
        {
            out[4*x]=qBlue(row[x]); out[4*x+1]=qGreen(row[x]); out[4*x+2]=qRed(row[x]); out[4*x+3]=255;
        }
    }
    auto frame = std::make_shared<room_image::Frame>();
    frame->width=rgb.width(); frame->height=rgb.height(); frame->stride=rgb.width()*4;
    frame->sequence=sequence; frame->pixels=std::move(pixels);
    return frame;
}

QImage Preview(const room_image::Frame& frame, const room_image::Mapping& mapping, QSize scene)
{
    if(!frame.Valid() || !mapping.Valid() || scene.isEmpty()) return {};
    scene.scale(320, 200, Qt::KeepAspectRatio);
    QImage image(scene, QImage::Format_RGB32);
    for(int y=0; y<image.height(); ++y)
    {
        auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
        for(int x=0; x<image.width(); ++x)
            row[x]=room_image::SampleBGRA(frame,mapping,(x+0.5)/image.width(),(y+0.5)/image.height());
    }
    return image;
}
}
