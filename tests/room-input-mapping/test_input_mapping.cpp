// SPDX-License-Identifier: GPL-2.0-or-later
#include <QApplication>
#include <atomic>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>
#include "FakeAPI.h"
#include "VirtualController.h"
#include "OpenRGBVisualMapPlugin.h"

OpenRGBPluginAPIInterface* OpenRGBVisualMapPlugin::api=nullptr;
static unsigned checks=0;
#define CHECK(x) do {++checks;if(!(x))throw std::runtime_error(#x);}while(0)
static bool near(double a,double b){return std::abs(a-b)<1e-9;}

class Keyboard : public RGBController
{
public:
    unsigned name_reads=0;
    explicit Keyboard(unsigned count=6)
    {
        name="Synthetic keyboard";serial="test-serial";location="test-location";type=DEVICE_TYPE_KEYBOARD;
        leds.resize(count);for(unsigned i=0;i<count;++i)leds[i].name="Key "+std::to_string(i);
        zone z;z.name="Keys";z.type=ZONE_TYPE_LINEAR;z.leds_count=z.leds_min=z.leds_max=count;
        if(count==6)for(unsigned i=0;i<2;++i){segment s;s.name="Segment";s.type=ZONE_TYPE_LINEAR;s.start_idx=i*3;s.leds_count=3;z.segments.push_back(s);}
        zones.push_back(z);mode m;m.name="Direct";m.brightness=100;m.brightness_max=100;modes.push_back(m);SetupColors();
    }
    ~Keyboard(){Shutdown();}
    void DeviceUpdateLEDs() override{}
    std::string GetLEDName(unsigned i) override {++name_reads;return RGBController::GetLEDName(i);}
    void MakeOther(){type=DEVICE_TYPE_LEDSTRIP;}
};
struct Member
{
    ControllerZone zone{};
    Member(Keyboard& keyboard,bool segment=false,unsigned index=0)
    {
        zone.set_controller(&keyboard);zone.zone_idx=0;zone.is_segment=segment;zone.segment_idx=index;
        zone.settings=ControllerZoneSettings::defaults();zone.settings.shape=CUSTOM;
        zone.settings.custom_shape=CustomShape::HorizontalLine(zone.led_count());
    }
    ~Member(){delete zone.settings.custom_shape;}
};

void GeometryAndLifecycle()
{
    FakeAPI api;OpenRGBVisualMapPlugin::api=&api;
    Keyboard keyboard,other;other.MakeOther();api.physical={&keyboard,&other};
    Member first(keyboard,true,0),second(keyboard,true,1),lamp(other);
    first.zone.settings.x=10;first.zone.settings.y=20;
    second.zone.settings.x=30;second.zone.settings.y=40;
    auto& settings=second.zone.settings;settings.scale_x=2;settings.scale_y=3;settings.rotation=90;settings.flip_x=true;
    // Fractional cell and repeated physical position remain separate global LEDs.
    auto& positions=settings.custom_shape->led_positions;
    positions[0]->point=QPointF(.25,.125);positions[1]->point=positions[0]->point;
    {
        VirtualController map;map.Add(&first.zone);map.Add(&second.zone);map.Add(&lamp.zone);map.UpdateSize(320,200);
        std::vector<room_input::InputPoint> points;CHECK(map.GetInputPoints(0,points));CHECK(points.size()==6);
        auto* wrapper=dynamic_cast<room_input::RGBControllerInputMappingInterface*>(api.created.back());
        CHECK(wrapper);std::vector<room_input::InputPoint> forwarded;
        CHECK(wrapper->GetInputPoints(0,forwarded)&&forwarded.size()==points.size());
        CHECK(forwarded[3].global_led==points[3].global_led&&near(forwarded[3].u,points[3].u));
        CHECK(points[0].global_led==0&&points[3].global_led==3&&points[5].global_led==5);
        CHECK(points[0].location=="test-location"&&points[0].serial=="test-serial"&&points[0].name=="Synthetic keyboard");
        CHECK(points[3].keyname=="Key 3");CHECK(near(points[0].u,10.5/320)&&near(points[0].v,20.5/200));
        // Independent affine calculation: (0.75,.625) about center(1.5,.5),
        // x-flip / scale(2,3), rotate90, translate center(3,1.5) + (30,40).
        CHECK(near(points[3].u,32.625/320)&&near(points[3].v,43.0/200));
        CHECK(near(points[3].u,points[4].u)&&near(points[3].v,points[4].v));
        auto generation=points[0].generation;CHECK(generation>0);
        for(const auto& p:points)CHECK(p.generation==generation);
        const auto reads=keyboard.name_reads;CHECK(map.GetInputPoints(0,points));CHECK(keyboard.name_reads==reads);
        CHECK(!map.GetInputPoints(1,points)&&points.empty());
        map.SetRoutingEnabled(false);CHECK(!map.GetInputPoints(0,points)&&points.empty());
        CHECK(!wrapper->GetInputPoints(0,forwarded)&&forwarded.empty());
        map.SetRoutingEnabled(true);CHECK(map.GetInputPoints(0,points)&&points[0].generation>generation);
        generation=points[0].generation;settings.x=31;map.UpdateVirtualZone();
        CHECK(map.GetInputPoints(0,points)&&points[0].generation>generation);CHECK(near(points[3].u,33.625/320));
        // Cache copies remain valid after a member is removed; new queries do not.
        const auto retained=points;map.Remove(&second.zone);CHECK(!map.GetInputPoints(0,points)&&points.empty());
        CHECK(retained[3].keyname=="Key 3"&&near(retained[3].u,33.625/320));
        map.UpdateVirtualZone();CHECK(map.GetInputPoints(0,points)&&points.size()==3);
        std::atomic<bool> running{true},valid{true};
        std::thread reader([&]{std::vector<room_input::InputPoint> copy;while(running){if(map.GetInputPoints(0,copy))for(const auto& p:copy)if(p.generation!=copy.front().generation)valid=false;}});
        for(unsigned i=0;i<30;++i){first.zone.settings.x=i;map.UpdateVirtualZone();}
        running=false;reader.join();CHECK(valid);
        map.Clear();CHECK(!map.GetInputPoints(0,points)&&points.empty());
    }
    OpenRGBVisualMapPlugin::api=nullptr;
}

void BoundedAndLegacy()
{
    FakeAPI api;OpenRGBVisualMapPlugin::api=&api;
    Keyboard keyboard(unsigned(room_input::MaxInputPoints+1));Member huge(keyboard);
    // Tiny fractional cells avoid an expensive giant compatibility grid while
    // exercising the production inverse point admission boundary.
    huge.zone.settings.scale=.001;
    {VirtualController map;map.Add(&huge.zone);map.UpdateSize(32,20);
     std::vector<room_input::InputPoint> points;CHECK(!map.GetInputPoints(0,points)&&points.empty());}
    api.image_version=0;
    {VirtualController map;std::vector<room_input::InputPoint> points;CHECK(!map.GetInputPoints(0,points));}
    OpenRGBVisualMapPlugin::api=nullptr;
}

int main(int argc,char** argv)
{
    QApplication app(argc,argv);
    try{GeometryAndLifecycle();BoundedAndLegacy();std::cout<<"PASS input mapping: "<<checks<<" assertions; exact affine/segments/duplicates, cache, disable/rebind/removal, concurrent snapshots, bounded admission\n";return 0;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
