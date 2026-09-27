/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <QApplication>
#include <QGraphicsScene>
#include <QPainter>
#include <QElapsedTimer>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <fstream>
#include <set>
#include <numeric>
#include <stdexcept>
#include <thread>
#include "FakeAPI.h"
#include "ImageRouting.h"
#include "VirtualController.h"
#include "OpenRGBVisualMapPlugin.h"
#include "VisualMapJsonDefinitions.h"
#include "ControllerZoneItem.h"
#include "ZoneIdentity.h"

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
    explicit Member(RGBControllerInterface* c, unsigned zone_index=0)
    {
        zone.set_controller(c); zone.zone_idx=zone_index; zone.is_segment=false; zone.segment_idx=0;
        zone.settings=ControllerZoneSettings::defaults(); zone.settings.shape=CUSTOM;
        auto* s=zone.settings.custom_shape=new CustomShape();
        s->w=c->GetZoneMatrixMapWidth(zone_index);s->h=c->GetZoneMatrixMapHeight(zone_index);
        const auto* m=c->GetZoneMatrixMapData(zone_index);
        for(unsigned y=0;y<s->h;++y) for(unsigned x=0;x<s->w;++x)
            if(m[size_t(y)*unsigned(s->w)+x] != NA) s->led_positions.push_back(new LedPosition{m[size_t(y)*unsigned(s->w)+x],QPointF(x,y)});
    }
    ~Member(){delete zone.settings.custom_shape;}
};

// Intercepts only the optional color-frame interface. A direct SetColor call
// remains observable, so refusal tests cannot accidentally pass via fallback.
class ColorBatchDevice : public FakeDevice
{
public:
    explicit ColorBatchDevice(unsigned count,bool two_zones=false,bool six_segments=false):FakeDevice(count,1)
    {
        native=false;
        if(two_zones)
        {
            name="Two-zone synthetic output";
            zones.push_back(zones.front());leds.resize(2*count);SetupColors();
        }
        if(six_segments)
        {
            name="Six-segment synthetic output";
            for(unsigned i=0;i<6;++i)
            {
                segment s;s.name="Segment "+std::to_string(i);s.type=ZONE_TYPE_LINEAR;s.start_idx=2*i;s.leds_count=2;
                zones[0].segments.push_back(s);
            }
        }
    }
    std::atomic<uint64_t> topology{41};
    std::atomic<unsigned> batches{0}, direct_colors{0};
    std::atomic<room_color::SubmitResult> batch_result{room_color::SubmitResult::Accepted};
    std::mutex batch_mutex;
    std::shared_ptr<const room_color::ColorFrame> last_batch;
    unsigned last_lease=0;
    uint64_t GetColorTopology() const override {return topology.load();}
    room_color::SubmitResult SubmitColorFrame(std::shared_ptr<const room_color::ColorFrame> f,unsigned lease) override
    {
        const auto result=f->topology==topology.load()?batch_result.load():room_color::SubmitResult::Stale;
        {std::lock_guard<std::mutex> lock(batch_mutex);last_batch=std::move(f);last_lease=lease;}
        ++batches;return result;
    }
    void SetColor(unsigned index,RGBColor color) override {++direct_colors;RGBController::SetColor(index,color);}
};

void ColorBatches()
{
    FakeAPI api;OpenRGBVisualMapPlugin::api=&api;
    ColorBatchDevice multi_zone(1,true), multi_segment(12,false,true);
    api.physical={&multi_zone,&multi_segment};
    Member first(&multi_zone,0),second(&multi_zone,1),overlap(&multi_segment);
    first.zone.settings.x=1;second.zone.settings.x=17;
    std::vector<std::unique_ptr<Member>> segments;
    for(unsigned i=0;i<6;++i)
    {
        auto m=std::make_unique<Member>(&multi_segment);
        m->zone.is_segment=true;m->zone.segment_idx=i;m->zone.settings.x=2*i;
        delete m->zone.settings.custom_shape;m->zone.settings.custom_shape=CustomShape::HorizontalLine(2);
        segments.push_back(std::move(m));
    }
    // The overlapping whole-zone member is distinct from its segments. Its
    // repeated LED indexes must retain map order instead of being deduplicated.
    overlap.zone.settings.x=5;
    QImage image(32,4,QImage::Format_RGB32);
    for(int y=0;y<4;++y)for(int x=0;x<32;++x)image.setPixel(x,y,qRgb(x*7,y*20,30));
    const auto frame=visual_image::FromImage(image,800);
    {
        VirtualController map;
        map.Add(&first.zone);map.Add(&second.zone);
        for(auto& m:segments)map.Add(&m->zone);
        map.Add(&overlap.zone);map.UpdateSize(32,4);
        auto* sink=dynamic_cast<room_image::RGBControllerImageInterface*>(api.created.back());CHECK(sink);
        auto submit=[&]{Await([&]{return sink->SubmitImage(0,frame,{},777)==room_image::SubmitResult::Accepted;});};
        submit();Await([&]{return multi_zone.batches==1&&multi_segment.batches==1;});
        {
            std::lock_guard<std::mutex> z(multi_zone.batch_mutex),s(multi_segment.batch_mutex);
            const auto& zf=*multi_zone.last_batch;const auto& sf=*multi_segment.last_batch;
            CHECK(zf.values.size()==2&&zf.values[0].index==0&&zf.values[1].index==1);
            CHECK(zf.values[0].color!=zf.values[1].color);
            CHECK(sf.values.size()==24&&sf.topology==41);
            for(unsigned i=0;i<24;++i)CHECK(sf.values[i].index==i%12);
            CHECK(sf.values[0].color!=sf.values[12].color);
            CHECK(multi_segment.last_lease>=100&&multi_segment.last_lease<=777);
        }
        CHECK(multi_zone.direct_colors==0&&multi_segment.direct_colors==0);
        // A resized destination must see the cached token, not a freshly read
        // token that would falsely legitimize an old route's indexes.
        multi_segment.topology=42;submit();Await([&]{return multi_segment.batches==2;});
        {std::lock_guard<std::mutex> lock(multi_segment.batch_mutex);CHECK(multi_segment.last_batch->topology==41);}
        CHECK(multi_segment.direct_colors==0&&multi_segment.led_updates==0);
        map.UpdateSize(32,4);submit();Await([&]{return multi_segment.batches==3;});
        {std::lock_guard<std::mutex> lock(multi_segment.batch_mutex);CHECK(multi_segment.last_batch->topology==42);}
        for(auto refused:{room_color::SubmitResult::Busy,room_color::SubmitResult::Invalid,room_color::SubmitResult::Stale})
        {
            multi_segment.batch_result=refused;const auto before=multi_segment.batches.load();submit();
            Await([&]{return multi_segment.batches>before;});
            CHECK(multi_segment.direct_colors==0&&multi_segment.led_updates==0);
        }
        multi_segment.batch_result=room_color::SubmitResult::Unsupported;
        submit();Await([&]{return multi_segment.led_updates>0;});
        CHECK(multi_segment.direct_colors==24);CHECK(multi_zone.direct_colors==0);
    }
    OpenRGBVisualMapPlugin::api=nullptr;
}

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

void AffineGeometryAndJson()
{
    FakeDevice device(3,2);Member member(&device);auto& s=member.zone.settings;
    s.x=10;s.y=20;s.scale_x=2;s.scale_y=3;s.rotation=90;s.flip_x=true;s.brightness=.4;
    const auto plan=visual_image::BuildPlan(&member.zone,100,100);
    CHECK(plan.affine_surface&&plan.samples.size()==6);
    CHECK(close(plan.samples[0].u,.145)&&close(plan.samples[0].v,.25));
    CHECK(close(plan.surface.origin_x,.16)&&close(plan.surface.origin_y,.26));
    CHECK(close(plan.surface.u_x,0)&&close(plan.surface.u_y,-.06));
    CHECK(close(plan.surface.v_x,-.06)&&close(plan.surface.v_y,0)&&close(plan.surface.brightness,.4));
    const auto cells=LedRouting::BuildCells(&member.zone);
    CHECK(close(cells[0].local_rect.width(),3)&&close(cells[0].local_rect.height(),2));
    CHECK(close(cells[0].local_rect.center().x(),4.5)&&close(cells[0].local_rect.center().y(),5));
    s.rotation=37;s.flip_y=true;
    const auto rotated=LedRouting::BuildRoutes(&member.zone,QPointF(s.x,s.y),QSize(100,100));
    CHECK(rotated.size()==6);
    for(const auto& route:rotated){double total=0;for(const auto& p:route.overlaps)total+=p.weight;CHECK(close(total,1));}
    {
        FakeDevice one(1,1);Member diamond(&one);diamond.zone.settings.scale=2;diamond.zone.settings.rotation=45;
        const auto routes=LedRouting::BuildRoutes(&diamond.zone,QPointF(2,2),QSize(8,8));CHECK(routes.size()==1);
        for(const auto& p:routes[0].overlaps)CHECK(p.pixel!=QPoint(1,1)); // Inside bounding box, outside actual diamond.
        QImage corners(8,8,QImage::Format_RGB32);corners.fill(Qt::black);corners.setPixel(1,1,qRgb(255,0,0));
        CHECK(LedRouting::MixColor(corners,routes[0]).red()==0);
    }
    s.custom_shape->w=16.25;s.custom_shape->h=6.5;
    s.custom_shape->led_positions[0]->point=QPointF(.125,1.875);
    s.custom_shape->led_positions[1]->point=QPointF(.125,1.875); // Independent physical LEDs can overlap.
    json serialized=s;ControllerZoneSettings loaded=ControllerZoneSettings::defaults();serialized.get_to(loaded);
    CHECK(loaded.scale_x==2&&loaded.scale_y==3&&loaded.rotation==37&&loaded.flip_x&&loaded.flip_y);
    CHECK(loaded.custom_shape->w==16.25&&loaded.custom_shape->h==6.5);
    CHECK(loaded.custom_shape->led_positions[0]->point==QPointF(.125,1.875));
    CHECK(loaded.custom_shape->led_positions[1]->point==QPointF(.125,1.875));
    CHECK(loaded.custom_shape->led_positions[0]->led_num!=loaded.custom_shape->led_positions[1]->led_num);
    const auto dup=visual_image::BuildPlan(&member.zone,100,100);
    CHECK(dup.samples.size()==6&&close(dup.samples[0].u,dup.samples[1].u)&&close(dup.samples[0].v,dup.samples[1].v));
    CHECK(!dup.affine_surface);delete loaded.custom_shape;
    serialized.erase("affine");serialized.erase("point_origin");serialized.erase("brightness");
    serialized.get_to(loaded);CHECK(loaded.scale_x==1&&loaded.scale_y==1&&loaded.rotation==0&&!loaded.flip_x&&!loaded.flip_y&&!loaded.point_is_center&&loaded.brightness==1);delete loaded.custom_shape;
    serialized["affine"]={{"scale_x",0}};bool failed=false;try{serialized.get_to(loaded);}catch(...){failed=true;}CHECK(failed);
    s.point_is_center=true;
    const auto exact=visual_image::BuildPlan(&member.zone,100,100);
    const auto expected=LedRouting::LocalTransform(&member.zone).map(QPointF(.125,1.875))+QPointF(s.x,s.y);
    CHECK(close(exact.samples[0].u,expected.x()/100)&&close(exact.samples[0].v,expected.y()/100));
    // Render the production scene item, using the same rotated polygons as routing.
    GridSettings grid{100,100,false,true,1,false,false,false,false};
    QGraphicsScene scene;scene.setSceneRect(0,0,100,100);
    auto* item=new ControllerZoneItem(&member.zone,&grid);scene.addItem(item);
    QImage source(100,100,QImage::Format_RGB32);source.fill(qRgb(200,100,50));item->UpdatePreview(source);
    QImage painted(100,100,QImage::Format_ARGB32);painted.fill(Qt::transparent);
    {QPainter painter(&painted);scene.render(&painter);}
    unsigned visible=0;for(int y=0;y<100;++y)for(int x=0;x<100;++x)if(qAlpha(painted.pixel(x,y)))++visible;
    CHECK(visible>0);CHECK(item->sceneBoundingRect().contains(item->mapToScene(LedRouting::BuildCells(&member.zone)[0].local_rect.center())));
}

void AffineNativePipeline()
{
    FakeAPI api;OpenRGBVisualMapPlugin::api=&api;FakeDevice device(3,2);api.physical={&device};Member member(&device);
    auto& s=member.zone.settings;s.x=10;s.y=20;s.scale_x=2;s.scale_y=3;s.rotation=37;s.flip_x=true;s.brightness=.5;
    QImage image(100,100,QImage::Format_RGB32);image.fill(qRgb(160,80,40));
    const auto frame=visual_image::FromImage(image,700);auto outer=room_image::Mapping::Rectangle(.1,.2,.7,.5,13,false,true);outer.brightness=.4;
    const auto expected=visual_image::Compose(outer,visual_image::BuildPlan(&member.zone,100,100).surface);
    {
        VirtualController map;map.Add(&member.zone);map.UpdateSize(100,100);
        auto* sink=dynamic_cast<room_image::RGBControllerImageInterface*>(api.created.back());CHECK(sink);
        CHECK(sink->SubmitImage(0,frame,outer,500)==room_image::SubmitResult::Accepted);
        Await([&]{return device.submits>0;});CHECK(device.led_updates==0);
        std::lock_guard<std::mutex> lock(device.seen_mutex);
        CHECK(device.seen==frame&&close(device.seen_mapping.brightness,.2));
        for(const auto p:{QPointF(0,0),QPointF(.3,.7),QPointF(1,1)}) {
            double ax,ay,bx,by;expected.Point(p.x(),p.y(),ax,ay);device.seen_mapping.Point(p.x(),p.y(),bx,by);CHECK(close(ax,bx)&&close(ay,by));
        }
    }
    OpenRGBVisualMapPlugin::api=nullptr;
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
        const unsigned native_before_fallback=native.led_updates;
        map.DeviceUpdateLEDs();Await([&]{return ordinary.led_updates>physical_updates && native.led_updates>native_before_fallback;});
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

void SegmentIdentity()
{
    FakeDevice device(4,1);
    ControllerZone first{},second{},whole{};
    for(auto* z:{&first,&second,&whole}) {z->set_controller(&device);z->settings=ControllerZoneSettings::defaults();}
    first.is_segment=second.is_segment=true;first.segment_idx=0;second.segment_idx=1;
    CHECK(first.compare(&first));CHECK(!first.compare(&second));CHECK(!first.compare(&whole));
    json a=&first,b=&second,c=&whole;
    CHECK(a["is_segment"]==true&&a["segment_idx"]==0&&b["segment_idx"]==1);
    CHECK(visual_identity::Matches(a,0,true,0));CHECK(!visual_identity::Matches(a,0,true,1));
    CHECK(!visual_identity::SameZone(a,b));CHECK(visual_identity::SameZone(a,a));
    CHECK(!visual_identity::SameZone(a,c));
    json legacy=c;legacy.erase("is_segment");legacy.erase("segment_idx");
    CHECK(visual_identity::SameZone(legacy,c));CHECK(visual_identity::Matches(legacy,0,false,99));
    CHECK(!visual_identity::Matches(legacy,0,true,0));
    json invalid=a;invalid.erase("segment_idx");CHECK(!visual_identity::SameZone(invalid,a));
    invalid=a;invalid["segment_idx"]=-1;CHECK(!visual_identity::Matches(invalid,0,true,0));
    invalid=a;invalid["segment_idx"]=UINT64_MAX;CHECK(!visual_identity::Matches(invalid,0,true,0));
    invalid=a;invalid["is_segment"]=1;CHECK(!visual_identity::Matches(invalid,0,true,0));
    // Exercise the exact add/remove identity predicate used by the editor.
    std::vector<json> active;
    for(const auto& entry:{a,b,a})
        if(std::none_of(active.begin(),active.end(),[&](const json& saved){return visual_identity::SameZone(entry,saved);})) active.push_back(entry);
    CHECK(active.size()==2);
    active.erase(std::remove_if(active.begin(),active.end(),[&](const json& saved){return visual_identity::SameZone(a,saved);}),active.end());
    CHECK(active.size()==1&&active[0]["segment_idx"]==1);
}

void TestRoutingPerformance();
int main(int argc,char** argv)
{
    QApplication app(argc,argv);
    try{
        Geometry();LegacySampling();AffineGeometryAndJson();AffineNativePipeline();Pipeline();LegacyHost();SegmentIdentity();ColorBatches();TestRoutingPerformance();
        if(argc==3&&std::string(argv[1])=="--validate-map")
        {
            std::ifstream file(argv[2]);CHECK(file.good());json map;file>>map;
            std::set<std::string> identities;unsigned points=0;
            for(const auto& entry:map.at("ctrl_zones"))
            {
                visual_identity::Zone identity;CHECK(visual_identity::Read(entry,identity));
                const std::string key=entry.at("controller").dump()+":"+std::to_string(identity.index)+":"+std::to_string(identity.segment)+":"+std::to_string(identity.segment_index);
                CHECK(identities.insert(key).second);
                ControllerZoneSettings settings=entry.at("settings").get<ControllerZoneSettings>();
                if(settings.custom_shape){points+=unsigned(settings.custom_shape->led_positions.size());delete settings.custom_shape;}
            }
            std::cout<<"Map parser: "<<identities.size()<<" distinct members, "<<points<<" preserved LED points; no device binding\n";
        }
        std::cout<<"PASS geometry, affine JSON/fractional/duplicate/rotation/flips/polygon rendering, native affine pipeline, legacy sampling, actual wrapper/mailbox/lease/cycle/lifetime pipeline, legacy host, segment identity/JSON compatibility, grouped color frames across two zones and six segments, cached topology/refusals/legacy fallback\n";return 0;
    }
    catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}
}
