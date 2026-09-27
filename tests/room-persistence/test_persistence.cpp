/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <QApplication>
#include <QPluginLoader>
#include <QTemporaryDir>
#include <QDir>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QThread>
#include <QElapsedTimer>
#include <QTest>
#include <atomic>
#include <fstream>
#include <iostream>
#include "FakeAPI.h"
#include "ProfileManager.h"
#include "ResourceManagerCallback.h"
#include "MapPersistence.h"
using json=nlohmann::json;
static unsigned assertions=0;
#define CHECK(x) do { ++assertions; if(!(x)) throw std::runtime_error(#x); } while(0)
class Output : public RGBController
{
public:
    std::atomic<unsigned> updates{0};
    Output()
    {
        name="Persistence fake strip";vendor="Room tests";serial="synthetic";location="test-only";
        leds.resize(2);zone z;z.name="Strip";z.type=ZONE_TYPE_LINEAR;z.leds_count=z.leds_min=z.leds_max=2;zones.push_back(z);
        mode m;m.name="Direct";m.color_mode=MODE_COLORS_PER_LED;modes.push_back(m);SetupColors();
    }
    ~Output(){Shutdown();}
    void DeviceUpdateLEDs() override {++updates;}
};
class Host : public FakeAPI
{
public:
    filesystem::path path;
    explicit Host(const QString& p):path(p.toStdString()){}
    filesystem::path GetConfigurationDirectory() override{return path;}
    json GetSettings(std::string) override{return json::object();}
};
static void Wait(unsigned ms)
{
    QElapsedTimer timer;timer.start();
    while(timer.elapsed()<ms){QApplication::processEvents();QThread::msleep(2);}
}
static json Read(const filesystem::path& p){std::ifstream f(p);json j;f>>j;return j;}
static void Write(const filesystem::path& p,const json& j){std::ofstream f(p);f<<j.dump(2);CHECK(f.good());}
static json Map(bool autoload,double x)
{
    json identity={{"name","Persistence fake strip"},{"vendor","Room tests"},{"serial","synthetic"},{"location","test-only"}};
    json settings={{"shape",0},{"x",x},{"y",15.25},{"scale",1},{"led_spacing",1},{"reverse",false},{"custom_shape",nullptr}};
    json member={{"controller",identity},{"zone_idx",0},{"settings",settings},{"source_note","preserve imported provenance"}};
    json missing=member;missing["controller"]["serial"]="unplugged";
    return {{"ctrl_zones",json::array({member,missing})},
      {"grid_settings",{{"w",320},{"h",200},{"show_grid",false},{"show_bounds",true},{"grid_size",1},
        {"snap_to_grid",false},{"auto_load",autoload},{"auto_register",autoload},{"hide_members",false}}}};
}
static room_image::RGBControllerImageInterface* Image(Host& host,const std::string& name)
{
    for(auto* c:host.created)if(c->GetName()==name)return dynamic_cast<room_image::RGBControllerImageInterface*>(c);
    return nullptr;
}
static std::shared_ptr<const room_image::Frame> Frame()
{
    auto pixels=std::make_shared<std::vector<uint8_t>>(320*200*4,255);
    return std::make_shared<const room_image::Frame>(room_image::Frame{320,200,1280,1,pixels});
}
static QWidget* ActiveTab(OpenRGBPluginInterface* p)
{
    auto* tabs=p->GetWidget()->findChild<QTabWidget*>("virtual_controller_tabs");CHECK(tabs);return tabs->currentWidget();
}
static void EditPosition(OpenRGBPluginInterface* p,double x,double y)
{
    auto* tab=ActiveTab(p);auto* grid=tab->findChild<QGraphicsView*>("grid");CHECK(grid&&grid->scene());
    bool selected=false;
    for(auto* item:grid->scene()->items())if(item->flags()&QGraphicsItem::ItemIsSelectable)
    { QTest::mouseClick(grid->viewport(),Qt::LeftButton,Qt::NoModifier,grid->mapFromScene(item->mapToScene(item->boundingRect().center())));selected=item->isSelected();break; }
    CHECK(selected);QApplication::processEvents();
    auto* xs=tab->findChild<QDoubleSpinBox*>("x_spinBox");auto* ys=tab->findChild<QDoubleSpinBox*>("y_spinBox");CHECK(xs&&ys);
    xs->setValue(x);ys->setValue(y);
}
static void Load(OpenRGBPluginInterface* p,const json& data)
{
    p->OnProfileAboutToLoad();p->OnProfileLoad(data);
    p->ProfileManagerUpdated(PROFILEMANAGER_UPDATE_REASON_ACTIVE_PROFILE_CHANGED);Wait(20);
}
int main(int argc,char** argv)
{
    qputenv("QT_QPA_PLATFORM","offscreen");QApplication app(argc,argv);
    try
    {
        CHECK(argc==2);QTemporaryDir temp;CHECK(temp.isValid());
        const auto folder=temp.path()+"/plugins/settings/virtual-controllers";CHECK(QDir().mkpath(folder));
        const filesystem::path maps=folder.toStdString();
        Write(maps/"Full Scale.json",Map(true,10));Write(maps/"Music - Tri Band.json",Map(false,50));
        CHECK(!visual_persistence::ValidName("../outside.json"));CHECK(!visual_persistence::ValidName("x\\outside.json"));
        CHECK(!visual_persistence::ValidName(std::string("x\0y",3)));CHECK(visual_persistence::ValidName("Music - Tri Band.json"));
        Output output;Host host(temp.path());host.physical.push_back(&output);
        QPluginLoader loader(QString::fromLocal8Bit(argv[1]));loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
        auto* object=loader.instance();if(!object)throw std::runtime_error(loader.errorString().toStdString());
        auto* plugin=qobject_cast<OpenRGBPluginInterface*>(object);CHECK(plugin);plugin->Load(&host);
        plugin->GetWidget()->resize(1100,700);plugin->GetWidget()->show();Wait(30);
        CHECK(plugin->OnProfileSave()==json({{"version",1},{"active_map","Full Scale.json"}}));
        EditPosition(plugin,274.97,105.87);Wait(650);
        auto saved=Read(maps/"Full Scale.json");CHECK(saved["ctrl_zones"].size()==2);
        std::cerr << "Saved editor coordinates: " << saved["ctrl_zones"][0]["settings"]["x"] << ", " << saved["ctrl_zones"][0]["settings"]["y"] << '\n';
        CHECK(saved["ctrl_zones"][0]["settings"]["x"]==274.97);CHECK(saved["ctrl_zones"][0]["settings"]["y"]==105.87);
        CHECK(saved["ctrl_zones"][0]["source_note"]=="preserve imported provenance");
        EditPosition(plugin,274.98,105.88); // Recreate before the debounce fires.
        plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);Wait(20);
        plugin->OnProfileSave();saved=Read(maps/"Full Scale.json");
        CHECK(saved["ctrl_zones"][0]["settings"]["x"]==274.98);
        CHECK(saved["ctrl_zones"][0]["settings"]["y"]==105.88);CHECK(saved["ctrl_zones"].size()==2);
        CHECK(saved["ctrl_zones"][0]["source_note"]=="preserve imported provenance");
        const auto frame=Frame();auto* full=Image(host,"Full Scale.json");CHECK(full);
        CHECK(full->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Accepted);Wait(40);
        CHECK(output.updates>0);
        Load(plugin,{{"version",1},{"active_map","Music - Tri Band.json"}});
        auto* music=Image(host,"Music - Tri Band.json");CHECK(music);
        CHECK(full->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Busy);
        CHECK(music->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Accepted);Wait(40);
        CHECK(plugin->OnProfileSave()==json({{"version",1},{"active_map","Music - Tri Band.json"}}));
        auto latest=Read(maps/"Full Scale.json");latest["ctrl_zones"][0]["settings"]["x"]=123.456789;Write(maps/"Full Scale.json",latest);
        Load(plugin,{{"version",1},{"active_map","Full Scale.json"}});
        EditPosition(plugin,123.46,109.2); // X widget already rounds to 123.46; only Y changes.
        plugin->OnProfileSave();
        CHECK(Read(maps/"Full Scale.json")["ctrl_zones"][0]["settings"]["x"]==123.456789);
        CHECK(music->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Busy);
        Load(plugin,nullptr);CHECK(plugin->OnProfileSave()["active_map"]=="Full Scale.json");
        CHECK(full->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Accepted);
        // No plugin payload: core completion must resume the same current map.
        plugin->OnProfileAboutToLoad();plugin->ProfileManagerUpdated(PROFILEMANAGER_UPDATE_REASON_ACTIVE_PROFILE_CHANGED);
        CHECK(full->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Accepted);
        Load(plugin,{{"version",1},{"active_map",nullptr}});const auto stopped=output.updates.load();Wait(80);
        for(auto* c:host.created)c->UpdateLEDs();Wait(40);
        CHECK(full->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Busy);
        CHECK(music->SubmitImage(0,frame,{},5000)==room_image::SubmitResult::Busy);CHECK(output.updates==stopped);
        Load(plugin,{{"version",1},{"active_map","Music - Tri Band.json"}});
        plugin->Unload();CHECK(loader.unload());CHECK(host.created.empty());
        // Saved selection overrides Full Scale's legacy auto_register flag.
        object=loader.instance();CHECK(object);plugin=qobject_cast<OpenRGBPluginInterface*>(object);plugin->Load(&host);Wait(20);
        CHECK(plugin->OnProfileSave()["active_map"]=="Music - Tri Band.json");
        CHECK(Image(host,"Full Scale.json")->SubmitImage(0,frame,{},1000)==room_image::SubmitResult::Busy);
        CHECK(Image(host,"Music - Tri Band.json")->SubmitImage(0,frame,{},1000)==room_image::SubmitResult::Accepted);
        plugin->Unload();CHECK(loader.unload());CHECK(host.created.empty());
        std::cout<<"PASS "<<assertions<<" assertions: real DLL/editor autosave, float positions, missing members, recreate, canonical references, exclusive routing, legacy profiles, clean reload\n";
        return 0;
    }
    catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
}
