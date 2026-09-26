/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <QCoreApplication>
#include <QElapsedTimer>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <thread>
#include "FakeAPI.h"
#include "ImageRouting.h"
#include "VirtualController.h"
#include "OpenRGBVisualMapPlugin.h"

OpenRGBPluginAPIInterface* OpenRGBVisualMapPlugin::api = nullptr;
#define CHECK(x) do { if(!(x)) throw std::runtime_error(#x); } while(0)
bool close(double a,double b){return std::abs(a-b)<1e-7;}
template<class F> void Await(F check)
{
    for(unsigned i=0;i<1000;++i) {if(check()) return; std::this_thread::sleep_for(std::chrono::milliseconds(1));}
    CHECK(check());
}
class FakeDevice : public RGBController, public room_image::RGBControllerImageInterface
{
public:
    FakeDevice(unsigned w,unsigned h)
    {
        name="Synthetic output";
        leds.resize(w*h);
        zone z; z.name="Arbitrary zone"; z.type=ZONE_TYPE_MATRIX;
        z.leds_count=z.leds_min=z.leds_max=w*h;
        z.matrix_map.width=w; z.matrix_map.height=h; z.matrix_map.map.resize(w*h);
        std::iota(z.matrix_map.map.begin(),z.matrix_map.map.end(),0u); zones.push_back(z);
        mode m; m.name="Direct"; modes.push_back(m); SetupColors();
    }
    ~FakeDevice(){Shutdown();}
    bool native=true;
    std::atomic<room_image::SubmitResult> result{room_image::SubmitResult::Accepted};
    std::atomic<unsigned> submits{0}, led_updates{0};
    std::mutex seen_mutex;
    std::shared_ptr<const room_image::Frame> seen;
    room_image::Mapping seen_mapping;
    bool GetImageOutput(unsigned z, room_image::Output& o) const override
    { if(!native || z!=0) return false; o={0,800,600,60}; return true; }
    room_image::SubmitResult SubmitImage(unsigned, std::shared_ptr<const room_image::Frame> f,
        const room_image::Mapping& m,unsigned) override
    {std::lock_guard<std::mutex> lock(seen_mutex); seen=std::move(f);seen_mapping=m;++submits;return result.load();}
    void DeviceUpdateLEDs() override {++led_updates;}
};
struct Member
{
    ControllerZone zone{};
    explicit Member(RGBControllerInterface* c)
    {
        zone.set_controller(c); zone.zone_idx=0; zone.is_segment=false; zone.segment_idx=0;
        zone.settings=ControllerZoneSettings::defaults(); zone.settings.shape=CUSTOM;
        auto* s=zone.settings.custom_shape=new CustomShape();
        s->w=c->GetZoneMatrixMapWidth(0);s->h=c->GetZoneMatrixMapHeight(0);
        const auto* m=c->GetZoneMatrixMapData(0);
        for(unsigned y=0;y<s->h;++y) for(unsigned x=0;x<s->w;++x)
            if(m[size_t(y)*s->w+x] != NA) s->led_positions.push_back(new LedPosition{m[size_t(y)*s->w+x],QPoint(x,y)});
    }
    ~Member(){delete zone.settings.custom_shape;}
};

void Geometry()
{
    CHECK(visual_image::CompatibilitySize(64,64)==QSize(64,64));
    CHECK(visual_image::CompatibilitySize(800,600)==QSize(127,95));
    for(unsigned w : {1u,64u,127u,128u,800u,1024u}) for(unsigned h : {1u,128u,600u,1024u})
    {auto s=visual_image::CompatibilitySize(w,h);CHECK(s.width()*s.height()*4+8<=65535);}
    FakeDevice device(3,2); Member member(&device);
    member.zone.settings.x=10;member.zone.settings.y=20;member.zone.settings.scale=4;
    auto plan=visual_image::BuildPlan(&member.zone,100,100);
    CHECK(plan.affine_surface && plan.samples.size()==6);
    CHECK(close(plan.surface.origin_x,.1)&&close(plan.surface.origin_y,.2));
    CHECK(close(plan.surface.u_x,.12)&&close(plan.surface.v_y,.08));
    // The custom editor stores a 90 degree rotation in grid coordinates.
    for(auto* p:member.zone.settings.custom_shape->led_positions) p->point=QPoint(1-p->point.y(),p->point.x());
    member.zone.settings.custom_shape->w=2;member.zone.settings.custom_shape->h=3;
    plan=visual_image::BuildPlan(&member.zone,100,100);
    CHECK(plan.affine_surface && close(plan.surface.u_y,.12)&&close(plan.surface.v_x,-.08));
    auto outer=room_image::Mapping::Rectangle(.1,.2,.5,.4,37,true,false);
    outer.brightness=.7;
    const auto combined=visual_image::Compose(outer,plan.surface);
    double x,y,a,b,c,d;plan.surface.Point(.3,.8,x,y);outer.Point(x,y,a,b);combined.Point(.3,.8,c,d);
    CHECK(close(a,c)&&close(b,d)&&close(combined.brightness,.7));
    // Keep a valid dense shape but break affinity: per-LED fallback is required.
    std::swap(member.zone.settings.custom_shape->led_positions[0]->point,member.zone.settings.custom_shape->led_positions[1]->point);
    CHECK(!visual_image::BuildPlan(&member.zone,100,100).affine_surface);
    member.zone.settings.custom_shape->h=4;
    CHECK(!visual_image::BuildPlan(&member.zone,100,100).affine_surface); // partial mask
    member.zone.settings.shape=HORIZONTAL_LINE;member.zone.settings.reverse=true;member.zone.settings.led_spacing=3;
    plan=visual_image::BuildPlan(&member.zone,100,100);
    CHECK(!plan.affine_surface && plan.samples.front().u>plan.samples.back().u);
    member.zone.settings.scale=std::numeric_limits<double>::quiet_NaN();
    CHECK(visual_image::BuildPlan(&member.zone,100,100).samples.empty());
}

void LegacySampling()
{
    FakeDevice device(2,2);Member member(&device);
    member.zone.settings.x=1.25;member.zone.settings.y=.5;member.zone.settings.scale=1.5;
    const auto old_routes=LedRouting::BuildRoutes(&member.zone,QPointF(1.25,.5),QSize(8,8));
    const auto same_routes=LedRouting::BuildRoutes(&member.zone,QPointF(1.25,.5),QSize(8,8),QSizeF(8,8));
    CHECK(old_routes.size()==same_routes.size());
    QImage image(8,8,QImage::Format_RGB32);
    for(int y=0;y<8;++y)for(int x=0;x<8;++x) image.setPixel(x,y,qRgb(x*30,y*30,30));
    for(size_t i=0;i<old_routes.size();++i)
        CHECK(LedRouting::MixColor(image,old_routes[i])==LedRouting::MixColor(image,same_routes[i]));
    const auto scaled=LedRouting::BuildRoutes(&member.zone,QPointF(1.25,.5),QSize(4,4),QSizeF(8,8));
    CHECK(scaled.size()==4);
}

void Pipeline()
{
    FakeAPI api;OpenRGBVisualMapPlugin::api=&api;
    FakeDevice native(2,2), ordinary(2,2);ordinary.native=false;
    api.physical={&native,&ordinary};
    Member a(&native),b(&ordinary);
    a.zone.settings.scale=100;b.zone.settings.x=400;b.zone.settings.scale=100;
    auto source=QImage(800,600,QImage::Format_RGB32);
    for(int y=0;y<600;++y) for(int x=0;x<800;++x) source.setPixel(x,y,qRgb(x%256,y%256,40));
    auto frame=visual_image::FromImage(source,7);CHECK(frame&&frame->Valid());
    source.fill(Qt::black);CHECK((*frame->pixels)[0]==40); // owned, independent copy
    {
        VirtualController map;
        auto* wrapper=api.created.back();
        map.Add(&a.zone);map.Add(&b.zone);map.UpdateSize(800,600);
        CHECK(wrapper->GetLEDCount()<=127u*127u);
        auto* sink=dynamic_cast<room_image::RGBControllerImageInterface*>(wrapper);
        room_image::Output output;CHECK(sink&&sink->GetImageOutput(0,output));
        CHECK(output.width==800&&output.height==600);
        CHECK(!sink->GetImageOutput(1,output));
        CHECK(sink->SubmitImage(0,{}, {},100)==room_image::SubmitResult::Invalid);
        CHECK(sink->SubmitImage(0,frame,{},99)==room_image::SubmitResult::Invalid);
        CHECK(sink->SubmitImage(1,frame,{},100)==room_image::SubmitResult::Unsupported);
        CHECK(sink->SubmitImage(0,frame,{},150)==room_image::SubmitResult::Accepted);
        Await([&]{return native.submits>0&&ordinary.led_updates>0;});
        CHECK(native.led_updates==0);
        {std::lock_guard<std::mutex> lock(native.seen_mutex);
         CHECK(native.seen==frame);CHECK(close(native.seen_mapping.u_x,.25));
         CHECK(room_image::SampleBGRA(*native.seen,native.seen_mapping,.25,.5)
             !=room_image::SampleBGRA(*native.seen,native.seen_mapping,.75,.5));}
        const uint32_t expected=room_image::SampleBGRA(*frame,{},450.0/800,50.0/600);
        CHECK(ordinary.GetColor(0)==ToRGBColor(qRed(expected),qGreen(expected),qBlue(expected)));
        std::shared_ptr<const room_image::Frame> preview;room_image::Mapping mapping;
        CHECK(sink->GetImagePreview(0,preview,mapping)&&preview==frame);
        auto thumb=visual_image::Preview(*preview,mapping,QSize(800,600));CHECK(thumb.size()==QSize(266,200));
        const unsigned physical_updates=ordinary.led_updates;
        map.DeviceUpdateLEDs();CHECK(ordinary.led_updates==physical_updates); // active lease suppresses legacy
        std::this_thread::sleep_for(std::chrono::milliseconds(170));
        CHECK(!sink->GetImagePreview(0,preview,mapping));
        map.DeviceUpdateLEDs();Await([&]{return ordinary.led_updates>physical_updates;});
        const unsigned before=native.led_updates;
        native.result=room_image::SubmitResult::Busy;
        CHECK(sink->SubmitImage(0,frame,{},150)==room_image::SubmitResult::Accepted);
        Await([&]{return native.submits>=2;});CHECK(native.led_updates==before);
        native.result=room_image::SubmitResult::Invalid;
        const unsigned submitted=native.submits;
        sink->SubmitImage(0,frame,{},150);
        Await([&]{return native.submits>submitted;});CHECK(native.led_updates==before);
        native.result=room_image::SubmitResult::Unsupported;
        sink->SubmitImage(0,frame,{},150);
        Await([&]{return native.led_updates>before;});
        native.result=room_image::SubmitResult::Accepted;
        // The most recent frame is owned after producer replacement.
        for(unsigned i=0;i<100;++i){auto f=std::make_shared<room_image::Frame>(*frame);f->sequence=100+i;sink->SubmitImage(0,f,{},500);}
        auto final_frame=std::make_shared<room_image::Frame>(*frame);final_frame->sequence=200;
        Await([&]{return sink->SubmitImage(0,final_frame,{},500)==room_image::SubmitResult::Accepted;});
        Await([&]{std::lock_guard<std::mutex> lock(native.seen_mutex);return native.seen->sequence==200;});
        map.Clear();const unsigned seen=native.submits;
        sink->SubmitImage(0,frame,{},150);std::this_thread::sleep_for(std::chrono::milliseconds(30));CHECK(native.submits==seen);
        // A -> B is valid, B -> A and A -> A are rejected by wrapper identity.
        VirtualController second;auto* wrapper2=api.created.back();
        ControllerZone child{};child.set_controller(wrapper2);child.settings=ControllerZoneSettings::defaults();child.zone_idx=0;
        map.Add(&child);CHECK(map.GetZones().size()==1);
        ControllerZone parent{};parent.set_controller(wrapper);parent.settings=ControllerZoneSettings::defaults();parent.zone_idx=0;
        second.Add(&parent);CHECK(second.GetZones().empty());
        map.Add(&parent);CHECK(map.GetZones().size()==1);
        map.Clear();
    }
    CHECK(api.attachments==2&&api.detachments==2&&api.removed==2&&api.created.empty());
    OpenRGBVisualMapPlugin::api=nullptr;
}

void LegacyHost()
{
    FakeAPI api;api.image_version=0;OpenRGBVisualMapPlugin::api=&api;
    {
        VirtualController map;map.UpdateSize(128,128);
        auto* controller=api.created.back();
        CHECK(controller->GetZoneMatrixMapWidth(0)==128&&controller->GetZoneMatrixMapHeight(0)==128);
        room_image::Output output;CHECK(!map.GetImageOutput(0,output));
    }
    CHECK(api.attachments==0&&api.created.empty());
    OpenRGBVisualMapPlugin::api=nullptr;
}

int main(int argc,char** argv)
{
    QCoreApplication app(argc,argv);
    try{Geometry();LegacySampling();Pipeline();LegacyHost();std::cout<<"PASS geometry, legacy sampling, actual wrapper/mailbox/lease/cycle/lifetime pipeline, legacy host\n";return 0;}
    catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}
}
