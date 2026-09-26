/*---------------------------------------------------------*\
| VirtualController.h                                       |
|                                                           |
|   Virtual controller for visual map plugin                |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#define NA 0xFFFFFFFF

#include <functional>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <thread>
#include <QImage>
#include "ControllerZone.h"
#include "LedRouting.h"
#include "RGBControllerInterface.h"
#include "ImageRouting.h"
#include <FrameRouting/OpenRGBImagePluginAPI.h>

class VirtualController : public room_image::RGBControllerImageInterface
{
public:
    static std::string VIRTUAL_CONTROLLER_SERIAL;

    VirtualController();
    ~VirtualController();

    /*-----------------------------------------------------*\
    | Virtual RGBController Functions                       |
    \*-----------------------------------------------------*/
    void                            DeviceUpdateLEDs();

    /*-----------------------------------------------------*\
    | Internals                                             |
    \*-----------------------------------------------------*/
    void                            Add(ControllerZone*);
    void                            ApplyImage(const QImage&);
    void                            ApplyToDevice(const QImage&);
    void                            ApplyToZone(ControllerZone*, const QImage&);
    void                            Clear();
    std::string                     GetName();
    unsigned int                    GetTotalLeds();
    std::vector<ControllerZone*>    GetZones();
    bool                            HasZone(ControllerZone*);
    bool                            IsEmpty();
    void                            Register(bool, bool);
    void                            Remove(ControllerZone*);
    void                            SetName(std::string name);
    void                            SetPostUpdateCallBack(std::function<void(const QImage&)>);
    void                            UpdateSize(int,int);
    void                            UpdateVirtualZone();
    bool GetImageOutput(unsigned, room_image::Output&) const override;
    room_image::SubmitResult SubmitImage(unsigned, std::shared_ptr<const room_image::Frame>,
                                         const room_image::Mapping&, unsigned lease_ms) override;
    bool GetImagePreview(unsigned, std::shared_ptr<const room_image::Frame>&,
                         room_image::Mapping&) const override;

    /*-----------------------------------------------------*\
    | Static lifecycle management                           |
    \*-----------------------------------------------------*/
    static void                     UnregisterAll();

private:
    RGBControllerInterface*         virtual_controller;
    std::atomic<unsigned>           width{1}, height{1};
    bool                            registered = false;
    bool                            members_hidden = false;
    std::function<void(QImage)>     callback;
    std::vector<ControllerZone*>    added_zones;
    std::unordered_map<ControllerZone*, std::vector<LedRouting::LedRoute>> led_routes;
    struct ImageRoute
    {
        ControllerZone* member;
        RGBControllerInterface* controller;
        unsigned zone, start;
        visual_image::Plan plan;
        std::chrono::steady_clock::time_point last_submit{};
    };
    std::vector<ImageRoute>          image_routes;
    std::mutex                      added_zones_mutex;
    // Cached settings are rebuilt on the GUI thread. Workers never read mutable
    // ControllerZoneSettings while the editor is dragging/resizing a member.
    mutable std::mutex              image_mutex;
    std::condition_variable         image_changed;
    std::thread                     image_worker;
    bool                            image_running = true, image_dirty = false;
    std::shared_ptr<const room_image::Frame> image_frame;
    room_image::Mapping             image_mapping;
    std::chrono::steady_clock::time_point image_expiry{};
    room_image::PluginAPI*          image_api = nullptr;
    bool                            image_capable = false;
    std::atomic<bool>               image_attached{false};
    std::atomic<uint64_t>           image_sequence{0};
    RGBController_Setup             setup;

    void                            ForceDirectMode();
    void                            ImageLoop();
    void                            StopImages();
    bool                            RouteImage(const std::shared_ptr<const room_image::Frame>&,
                                               room_image::Mapping, unsigned);
    bool                            WouldCreateCycle(RGBControllerInterface*) const;
    std::set<RGBControllerInterface*> graph_targets; // instances_mutex

    static void                     DeviceUpdateLEDs_func(void* object_ptr);

    /*-----------------------------------------------------*\
    | Static instance tracking                              |
    \*-----------------------------------------------------*/
    static std::vector<VirtualController*> instances;
    static std::mutex                    instances_mutex;
};
