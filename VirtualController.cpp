/*---------------------------------------------------------*\
| VirtualController.cpp                                     |
|                                                           |
|   Virtual controller for visual map plugin                |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <set>
#include "OpenRGBVisualMapPlugin.h"
#include "RGBControllerInterface.h"
#include "VirtualController.h"

std::string VirtualController::VIRTUAL_CONTROLLER_SERIAL = "VISUAL_MAP_VISUAL_CONTROLLER_SERIAL";

std::vector<VirtualController*> VirtualController::instances;
std::mutex                      VirtualController::instances_mutex;

VirtualController::VirtualController()
{
    width                                               = 1;
    height                                              = 1;

    /*-----------------------------------------------------*\
    | Setup controller details                              |
    \*-----------------------------------------------------*/
    setup.object_ptr                                    = this;
    setup.name                                          = "Visual Map Controller";
    setup.vendor                                        = "OpenRGB Visual Map Plugin";
    setup.description                                   = "Virtual controller provided by the OpenRGB Visual Map Plugin";
    setup.version                                       = VERSION_STRING;
    setup.serial                                        = VIRTUAL_CONTROLLER_SERIAL;
    setup.location                                      = "Somewhere over the rainbow";
    setup.type                                          = DEVICE_TYPE_VIRTUAL;
    setup.active_mode                                   = 0;

    /*-----------------------------------------------------*\
    | Setup zone                                            |
    \*-----------------------------------------------------*/
    setup.zones.resize(1);

    setup.zones[0].name                                 = "Virtual Zone";
    setup.zones[0].type                                 = ZONE_TYPE_MATRIX;

    /*-----------------------------------------------------*\
    | Setup mode details                                    |
    \*-----------------------------------------------------*/
    setup.modes.resize(1);

    setup.modes[0].name                                 = "Direct";
    setup.modes[0].value                                = 0;
    setup.modes[0].flags                                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    setup.modes[0].brightness                           = 100;
    setup.modes[0].brightness_max                       = 100;
    setup.modes[0].brightness_min                       = 0;
    setup.modes[0].color_mode                           = MODE_COLORS_PER_LED;

    /*-----------------------------------------------------*\
    | Setup function pointers                               |
    \*-----------------------------------------------------*/
    setup.DeviceConfigureZone                           = nullptr;
    setup.DeviceUpdateLEDs                              = DeviceUpdateLEDs_func;
    setup.DeviceUpdateZoneLEDs                          = nullptr;
    setup.DeviceUpdateSingleLED                         = nullptr;
    setup.DeviceUpdateMode                              = nullptr;
    setup.DeviceSaveMode                                = nullptr;
    setup.DeviceUpdateZoneMode                          = nullptr;
    setup.DeviceUpdateDeviceSpecificConfiguration       = nullptr;
    setup.DeviceUpdateDeviceSpecificZoneConfiguration   = nullptr;

    virtual_controller = OpenRGBVisualMapPlugin::api->CreateVirtualRGBController(&setup);
    image_api = dynamic_cast<room_image::PluginAPI*>(OpenRGBVisualMapPlugin::api);
    if(image_api && image_api->ImageAPIVersion() >= 1)
        image_attached = image_api->AttachImageInterface(virtual_controller, this);
    image_capable = image_attached.load();
    image_worker = std::thread(&VirtualController::ImageLoop, this);

    /*-----------------------------------------------------*\
    | Track this instance for lifecycle management          |
    \*-----------------------------------------------------*/
    {
        std::lock_guard<std::mutex> lock(instances_mutex);
        instances.push_back(this);
    }
}

VirtualController::~VirtualController()
{
    StopImages();
    if(virtual_controller && OpenRGBVisualMapPlugin::api)
    {
        Register(false, false);
        OpenRGBVisualMapPlugin::api->DeleteVirtualRGBController(virtual_controller);
        virtual_controller = nullptr;
    }
    /*-----------------------------------------------------*\
    | Remove this instance from the tracking list           |
    \*-----------------------------------------------------*/
    {
        std::lock_guard<std::mutex> lock(instances_mutex);
        instances.erase(std::remove(instances.begin(), instances.end(), this), instances.end());
    }
}

void VirtualController::UpdateVirtualZone()
{
    if(!virtual_controller) return;
    std::lock_guard<std::mutex> lock(added_zones_mutex);
    const QSize grid = image_capable ? visual_image::CompatibilitySize(width.load(), height.load())
                                    : QSize(width.load(), height.load());
    const unsigned grid_width = grid.width(), grid_height = grid.height();
    /*-----------------------------------------------------*\
    | Resize matrix map                                     |
    \*-----------------------------------------------------*/
    setup.zones[0].matrix_map.height                    = grid_height;
    setup.zones[0].matrix_map.width                     = grid_width;
    setup.zones[0].matrix_map.map.resize(grid_height * grid_width);

    std::vector<std::vector<std::string>> real_leds;
    real_leds.resize(setup.zones[0].matrix_map.map.size());

    /*-----------------------------------------------------*\
    | Fill the map with NA                                  |
    \*-----------------------------------------------------*/
    for(std::size_t led_idx = 0; led_idx < setup.zones[0].matrix_map.map.size(); led_idx++)
    {
        setup.zones[0].matrix_map.map[led_idx]          = NA;
    }

    /*-----------------------------------------------------*\
    | Iterate controllers, count and place LEDs             |
    | Count real LEDs in the same loop                      |
    \*-----------------------------------------------------*/
    unsigned int    map_leds_count                      = 0;

    led_routes.clear();
    image_routes.clear();

    for(ControllerZone* ctrl_zone: added_zones)
    {
        RGBControllerInterface* controller = ctrl_zone->controller;
        std::vector<LedRouting::LedRoute> routes = LedRouting::BuildRoutes(
            ctrl_zone,
            QPointF(ctrl_zone->settings.x, ctrl_zone->settings.y),
            grid, QSizeF(width.load(), height.load()));
        const unsigned start = ctrl_zone->start_idx();
        image_routes.push_back({ctrl_zone, controller, ctrl_zone->zone_idx, start,
            visual_image::BuildPlan(ctrl_zone, width.load(), height.load())});

        for(const LedRouting::LedRoute& route : routes)
        {
            const std::string led_name = controller->GetLEDName(start + route.led_index);

            for(const LedRouting::PixelWeight& overlap : route.overlaps)
            {
                const unsigned int xy = overlap.pixel.y() * grid_width + overlap.pixel.x();

                if(real_leds[xy].empty())
                {
                    map_leds_count++;
                }

                real_leds[xy].push_back(led_name);
            }
        }

        led_routes[ctrl_zone] = std::move(routes);
    }

    /*-----------------------------------------------------*\
    | Update zone data                                      |
    \*-----------------------------------------------------*/
    setup.zones[0].leds_count                       = map_leds_count;
    setup.zones[0].leds_min                         = map_leds_count;
    setup.zones[0].leds_max                         = map_leds_count;
    setup.leds.resize(map_leds_count);

    /*-----------------------------------------------------*\
    | Update LED names and positions in matrix map          |
    \*-----------------------------------------------------*/
    int i = 0;

    for(unsigned int h = 0; h < grid_height; h++)
    {
        for(unsigned int w = 0; w < grid_width; w++)
        {
            unsigned int xy = (h*grid_width) + w;

            if(!real_leds[xy].empty())
            {
                setup.zones[0].matrix_map.map[xy]   = i;

                if(real_leds[xy].size() > 1)
                {
                    setup.leds[i].name = "Multiple LEDs (" + std::to_string(real_leds[xy].size()) + ")";
                }
                else
                {
                    setup.leds[i].name = real_leds[xy][0];
                }

                i++;
            }
        }
    }

    OpenRGBVisualMapPlugin::api->UpdateVirtualRGBController(virtual_controller, &setup);
}

void VirtualController::DeviceUpdateLEDs()
{
    {
        std::lock_guard<std::mutex> lock(image_mutex);
        if(!image_running || (image_frame && std::chrono::steady_clock::now() < image_expiry)) return;
    }
    std::unique_lock<std::mutex> lock(added_zones_mutex);
    const unsigned grid_width = setup.zones[0].matrix_map.width;
    const unsigned grid_height = setup.zones[0].matrix_map.height;
    float           brightness = virtual_controller->GetModeBrightness(0) / 100.f;
    unsigned int    color_index = 0;
    QImage          image(grid_width, grid_height, QImage::Format_ARGB32);
    QColor          transparent("#00000000");

    for(unsigned int h = 0; h < grid_height; h++)
    {
        for(unsigned int w = 0; w < grid_width; w++)
        {
            QColor color;

            if(setup.zones[0].matrix_map.map[(h*grid_width) + w] == NA)
            {
                color = transparent;
            }
            else
            {
                const RGBColor rgb = virtual_controller->GetColor(color_index++);
                color = QColor(RGBGetRValue(rgb) * brightness, RGBGetGValue(rgb)* brightness, RGBGetBValue(rgb)* brightness);
            }

            image.setPixelColor(w, h, color);
        }
    }

    lock.unlock();
    ApplyToDevice(image);
}

void VirtualController::UpdateSize(int w, int h)
{
    // Matches the existing editor range and bounds allocations for imported JSON.
    width   = std::clamp(w, 1, 1024);
    height  = std::clamp(h, 1, 1024);

    UpdateVirtualZone();
}

void VirtualController::SetName(std::string name)
{
    setup.name = name;
    if(!virtual_controller) return;
    OpenRGBVisualMapPlugin::api->UpdateVirtualRGBController(virtual_controller, &setup);   
}

void VirtualController::SetPostUpdateCallBack(std::function<void(const QImage&)> callback)
{
    std::lock_guard<std::mutex> lock(image_mutex);
    this->callback = callback;
}

void VirtualController::Register(bool state, bool hide_members)
{
    if(!virtual_controller) return;
    if(state)
    {
        if(!registered)
        {
            ForceDirectMode();

            if(hide_members)
            {
                std::set<RGBControllerInterface*> controllers;

                for(ControllerZone* ctrl_zone: added_zones)
                {
                    controllers.insert(ctrl_zone->controller);
                }

                /*-----------------------------------------*\
                | Ensure controller is in the latest list   |
                | as this function can be called during     |
                | list updates                              |
                \*-----------------------------------------*/
                std::vector<RGBControllerInterface*> available_controllers = OpenRGBVisualMapPlugin::api->GetRGBControllers();

                for(std::set<RGBControllerInterface*>::iterator it = controllers.begin(); it != controllers.end(); )
                {
                    if(std::find(available_controllers.begin(), available_controllers.end(), *it) == available_controllers.end())
                    {
                        it = controllers.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }

                for(RGBControllerInterface* controller : controllers)
                {
                    controller->SetHidden(true);
                }

                members_hidden = true;
            }

            OpenRGBVisualMapPlugin::api->RegisterVirtualRGBControllerInThread(virtual_controller);
            registered = true;
        }
    }
    else
    {
        if(registered)
        {
            OpenRGBVisualMapPlugin::api->UnregisterVirtualRGBController(virtual_controller);
            registered = false;

            if(members_hidden)
            {
                std::set<RGBControllerInterface*> controllers;

                for(ControllerZone* ctrl_zone: added_zones)
                {
                    controllers.insert(ctrl_zone->controller);
                }

                /*-----------------------------------------*\
                | Ensure controller is in the latest list   |
                | as this function can be called during     |
                | list updates                              |
                \*-----------------------------------------*/
                std::vector<RGBControllerInterface*> available_controllers = OpenRGBVisualMapPlugin::api->GetRGBControllers();

                for(std::set<RGBControllerInterface*>::iterator it = controllers.begin(); it != controllers.end(); )
                {
                    if(std::find(available_controllers.begin(), available_controllers.end(), *it) == available_controllers.end())
                    {
                        it = controllers.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }

                for(RGBControllerInterface* controller : controllers)
                {
                    controller->SetHidden(false);
                }

                members_hidden = false;
            }
        }
    }
}

void VirtualController::ForceDirectMode()
{
    std::set<RGBControllerInterface*> controllers;

    for(ControllerZone* ctrl_zone: added_zones)
    {
        controllers.insert(ctrl_zone->controller);
    }

    for(RGBControllerInterface* controller : controllers)
    {
        for(unsigned int i = 0; i < controller->GetModeCount(); i++)
        {
            if(controller->GetModeName(i) == "Direct")
            {
                controller->SetActiveMode(i);
            }
        }
    }
}

bool VirtualController::HasZone(ControllerZone* ctrl_zone)
{
    return std::find(added_zones.begin(), added_zones.end(),ctrl_zone) != added_zones.end();
}

void VirtualController::Add(ControllerZone* ctrl_zone)
{
    // Identity-based graph check; names/serials can collide or be user-edited.
    // The registry is held only on the GUI/editor path, never across SubmitImage.
    std::lock_guard<std::mutex> graph_lock(instances_mutex);
    if(WouldCreateCycle(ctrl_zone->controller))
    {
        LogAppend(LL_WARNING, "[Visual Map] Cyclic virtual-map membership rejected");
        return;
    }
    std::lock_guard<std::mutex> lock(added_zones_mutex);

    if(!HasZone(ctrl_zone))
    {
        added_zones.push_back(ctrl_zone);
        graph_targets.insert(ctrl_zone->controller);

        /*-------------------------------------------------*\
        | Make sure to have the correct LED size            |
        \*-------------------------------------------------*/
        if(ctrl_zone->isCustomShape() && ctrl_zone->led_count() !=  ctrl_zone->settings.custom_shape->led_positions.size())
        {
            ctrl_zone->settings.custom_shape->resizeCustomShape(ctrl_zone->led_count());
        }

        /*-------------------------------------------------*\
        | Hide controller if necessary                      |
        \*-------------------------------------------------*/
        if(registered && members_hidden && !ctrl_zone->controller->GetHidden())
        {
            ctrl_zone->controller->SetHidden(true);
        }
    }
}

void VirtualController::Remove(ControllerZone* ctrl_zone)
{
    std::lock_guard<std::mutex> graph_lock(instances_mutex);
    std::lock_guard<std::mutex> lock(added_zones_mutex);

    if(HasZone(ctrl_zone))
    {
        added_zones.erase(std::find(added_zones.begin(), added_zones.end(), ctrl_zone));
        if(std::none_of(added_zones.begin(), added_zones.end(), [=](auto* z){return z->controller == ctrl_zone->controller;}))
            graph_targets.erase(ctrl_zone->controller);
        // No retained raw controller pointers survive membership removal.
        image_routes.clear();
        led_routes.clear();
    }
}

void VirtualController::Clear()
{
    /*-----------------------------------------------------*\
    | Locked so a call in flight on the device thread       |
    | drains before the zones go away. Clearing here on     |
    | DETECTION_STARTED stops the device thread touching    |
    | controllers that are about to be freed.               |
    \*-----------------------------------------------------*/
    std::lock_guard<std::mutex> graph_lock(instances_mutex);
    std::lock_guard<std::mutex> lock(added_zones_mutex);

    added_zones.clear();
    graph_targets.clear();
    image_routes.clear();
    led_routes.clear();
}

std::vector<ControllerZone*> VirtualController::GetZones()
{
    return added_zones;
}

bool VirtualController::IsEmpty()
{
    return added_zones.empty();
}

std::string VirtualController::GetName()
{
    return virtual_controller ? virtual_controller->GetName() : setup.name;
}

unsigned int VirtualController::GetTotalLeds()
{
    unsigned int result = 0;

    for(ControllerZone* ctrl_zone : added_zones)
    {
        result += ctrl_zone->led_count();
    }

    return result;
}

void VirtualController::ApplyImage(const QImage& original)
{
    const auto frame = visual_image::FromImage(original, ++image_sequence);
    if(!frame) return;
    {
        std::lock_guard<std::mutex> lock(added_zones_mutex);
        const auto& map = setup.zones[0].matrix_map;
        for(unsigned y=0; y<map.height; ++y)
        for(unsigned x=0; x<map.width; ++x)
        {
            const unsigned led = map.map[size_t(y)*map.width+x];
            if(led == NA) continue;
            const uint32_t c = room_image::SampleBGRA(*frame, {}, (x+0.5)/map.width, (y+0.5)/map.height);
            virtual_controller->SetColor(led, ToRGBColor(qRed(c),qGreen(c),qBlue(c)));
        }
    }
    // BackgroundApplier is another image producer, not a per-pixel LED generator.
    SubmitImage(0, frame, room_image::Mapping{}, 1000);
}

void VirtualController::ApplyToDevice(const QImage& image)
{
    /*-----------------------------------------------------*\
    | Make sure we update the controller only once by using |
    | a set                                                 |
    \*-----------------------------------------------------*/
    std::set<RGBControllerInterface*> controllers;

    /*-----------------------------------------------------*\
    | Held across the apply so the zones (and the           |
    | controllers they point at) cannot be cleared/freed    |
    | mid-update by a DETECTION_STARTED on another thread.  |
    \*-----------------------------------------------------*/
    {
        std::lock_guard<std::mutex> lock(added_zones_mutex);

        for(ControllerZone* ctrl_zone: added_zones)
        {
            ApplyToZone(ctrl_zone, image);
            controllers.insert(ctrl_zone->controller);
        }

        for(RGBControllerInterface* controller : controllers)
        {
            controller->UpdateLEDs();
        }
    }

    std::function<void(const QImage&)> notify;
    { std::lock_guard<std::mutex> lock(image_mutex); notify = callback; }
    if(notify) notify(image);
}

void VirtualController::ApplyToZone(ControllerZone* ctrl_zone, const QImage& image)
{
    RGBControllerInterface* controller  = ctrl_zone->controller;
    unsigned int            start_idx;

    const auto plan = std::find_if(image_routes.begin(), image_routes.end(), [=](const auto& r) {
        return r.member == ctrl_zone;
    });
    if(plan == image_routes.end()) return;
    start_idx = plan->start;

    const auto routes = led_routes.find(ctrl_zone);

    if(routes == led_routes.end())
    {
        return;
    }

    for(const LedRouting::LedRoute& route : routes->second)
    {
        const QColor color = LedRouting::MixColor(image, route);
        controller->SetColor(start_idx + route.led_index,
                             ToRGBColor(color.red(), color.green(), color.blue()));
    }
}

void VirtualController::UnregisterAll()
{
    /*-----------------------------------------------------*\
    | Unregister all registered virtual controllers and     |
    | unhide any hidden members. This is called during      |
    | Unload() to ensure all controllers are properly       |
    | cleaned up before the plugin code is unloaded.        |
    \*-----------------------------------------------------*/
    std::vector<VirtualController*> snapshot;
    { std::lock_guard<std::mutex> lock(instances_mutex); snapshot = instances; }
    for(VirtualController* vc : snapshot) vc->StopImages();
    for(VirtualController* vc : snapshot)
    {
        if(vc->registered && OpenRGBVisualMapPlugin::api)
        {
            vc->Register(false, false);
        }
    }
    for(VirtualController* vc : snapshot)
    {
        if(vc->virtual_controller && OpenRGBVisualMapPlugin::api)
        {
            OpenRGBVisualMapPlugin::api->DeleteVirtualRGBController(vc->virtual_controller);
            vc->virtual_controller = nullptr;
        }
    }
}

void VirtualController::DeviceUpdateLEDs_func(void* object_ptr)
{
    ((VirtualController*)object_ptr)->DeviceUpdateLEDs();
}

bool VirtualController::WouldCreateCycle(RGBControllerInterface* target) const
{
    std::vector<RGBControllerInterface*> pending{target};
    std::set<RGBControllerInterface*> visited;
    while(!pending.empty())
    {
        auto* next = pending.back(); pending.pop_back();
        if(next == virtual_controller) return true;
        if(!visited.insert(next).second) continue;
        for(const auto* map : instances)
            if(map->virtual_controller == next)
                pending.insert(pending.end(), map->graph_targets.begin(), map->graph_targets.end());
    }
    return false;
}

bool VirtualController::GetImageOutput(unsigned zone, room_image::Output& out) const
{
    if(zone != 0 || !image_attached.load()) return false;
    out = {0, width.load(), height.load(), 60};
    return true;
}

room_image::SubmitResult VirtualController::SubmitImage(unsigned zone,
    std::shared_ptr<const room_image::Frame> frame, const room_image::Mapping& mapping, unsigned lease_ms)
{
    if(zone != 0) return room_image::SubmitResult::Unsupported;
    if(!frame || !frame->Valid() || !mapping.Valid() || lease_ms < 100 || lease_ms > 5000)
        return room_image::SubmitResult::Invalid;
    std::unique_lock<std::mutex> lock(image_mutex, std::try_to_lock);
    if(!lock.owns_lock()) return room_image::SubmitResult::Busy;
    if(!image_running) return room_image::SubmitResult::Unsupported;
    image_frame = std::move(frame);
    image_mapping = mapping;
    image_expiry = std::chrono::steady_clock::now() + std::chrono::milliseconds(lease_ms);
    image_dirty = true;
    lock.unlock(); image_changed.notify_one();
    return room_image::SubmitResult::Accepted;
}

bool VirtualController::GetImagePreview(unsigned zone, std::shared_ptr<const room_image::Frame>& frame,
                                       room_image::Mapping& mapping) const
{
    std::lock_guard<std::mutex> lock(image_mutex);
    if(zone != 0 || !image_running || !image_frame || std::chrono::steady_clock::now() >= image_expiry) return false;
    frame = image_frame; mapping = image_mapping;
    mapping.brightness *= std::clamp(virtual_controller->GetModeBrightness(0)/100.0,0.0,1.0);
    return true;
}

void VirtualController::StopImages()
{
    // Detach waits for any in-progress callback on the core wrapper. Neither
    // side holds image_mutex while waiting for the other side's thread/lock.
    if(image_attached.exchange(false) && image_api)
        image_api->AttachImageInterface(virtual_controller, nullptr);
    { std::lock_guard<std::mutex> lock(image_mutex); image_running=false; image_frame.reset(); }
    image_changed.notify_all();
    if(image_worker.joinable()) image_worker.join();
}

void VirtualController::ImageLoop()
{
    auto next_frame = std::chrono::steady_clock::now();
    auto last_preview = std::chrono::steady_clock::time_point{};
    std::unique_lock<std::mutex> lock(image_mutex);
    while(image_running)
    {
        image_changed.wait(lock, [this]{return !image_running || image_dirty;});
        if(!image_running) break;
        image_changed.wait_until(lock, next_frame, [this]{return !image_running;});
        if(!image_running) break;
        const auto now = std::chrono::steady_clock::now();
        const auto frame = image_frame;
        const auto mapping = image_mapping;
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(image_expiry-now).count();
        const auto notify = callback;
        image_dirty=false;
        if(!frame || remaining <= 0) continue;
        lock.unlock();
        const bool deferred = RouteImage(frame, mapping, unsigned(std::clamp<int64_t>(remaining,100,5000)));
        if(notify && now-last_preview >= std::chrono::milliseconds(67))
        {
            auto preview_mapping = mapping;
            preview_mapping.brightness *= std::clamp(virtual_controller->GetModeBrightness(0)/100.0,0.0,1.0);
            notify(visual_image::Preview(*frame,preview_mapping,QSize(width.load(),height.load())));
            last_preview=now;
        }
        next_frame=now+std::chrono::microseconds(16667);
        lock.lock();
        if(deferred && image_frame == frame && std::chrono::steady_clock::now() < image_expiry)
            image_dirty = true;
    }
}

bool VirtualController::RouteImage(const std::shared_ptr<const room_image::Frame>& frame,
                                   room_image::Mapping mapping, unsigned lease_ms)
{
    mapping.brightness *= std::clamp(virtual_controller->GetModeBrightness(0)/100.0,0.0,1.0);
    std::lock_guard<std::mutex> lock(added_zones_mutex);
    std::set<RGBControllerInterface*> led_controllers;
    const auto now = std::chrono::steady_clock::now();
    bool deferred = false;
    for(auto& route : image_routes)
    {
        if(route.plan.affine_surface)
        {
            auto* sink = dynamic_cast<room_image::RGBControllerImageInterface*>(route.controller);
            room_image::Output output;
            if(sink && sink->GetImageOutput(route.zone,output) && output.zone == route.zone)
            {
                const unsigned fps = output.max_fps ? std::clamp(output.max_fps,1u,240u) : 60;
                if(now-route.last_submit < std::chrono::microseconds(1000000/fps))
                { deferred = true; continue; }
                const auto result = sink->SubmitImage(route.zone,frame,
                    visual_image::Compose(mapping,route.plan.surface),lease_ms);
                if(result != room_image::SubmitResult::Unsupported)
                {
                    route.last_submit=now;
                    continue; // Busy/Invalid never cause a conflicting LED write.
                }
            }
        }
        auto led_mapping=mapping;led_mapping.brightness*=route.plan.brightness;
        for(const auto& sample : route.plan.samples)
        {
            const uint32_t c = room_image::SampleBGRA(*frame,led_mapping,sample.u,sample.v);
            route.controller->SetColor(route.start+sample.led,ToRGBColor(qRed(c),qGreen(c),qBlue(c)));
        }
        if(!route.plan.samples.empty()) led_controllers.insert(route.controller);
    }
    for(auto* controller : led_controllers) controller->UpdateLEDs();
    return deferred;
}
