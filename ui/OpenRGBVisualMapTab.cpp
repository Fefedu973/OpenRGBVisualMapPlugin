/*---------------------------------------------------------*\
| OpenRGBVisualMapTab.cpp                                   |
|                                                           |
|   OpenRGB Visual Map tab                                  |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QAction>
#include <QDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QString>
#include <QTimer>
#include <QToolButton>
#include <QScopedValueRollback>
#include "MapPersistence.h"

#include "OpenRGBVisualMapPlugin.h"
#include "OpenRGBVisualMapTab.h"
#include "TabHeader.h"
#include "VirtualControllerTab.h"
#include "VisualMapSettingsManager.h"

OpenRGBVisualMapTab::OpenRGBVisualMapTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::OpenRGBVisualMapTab)
{
    ui->setupUi(this);

    /*-----------------------------------------------------*\
    | Remove intial dummy tabs                              |
    \*-----------------------------------------------------*/
    ui->virtual_controller_tabs->clear();

    /*-----------------------------------------------------*\
    | Define tab style + settings                           |
    \*-----------------------------------------------------*/
    ui->virtual_controller_tabs->setTabsClosable(true);
    ui->virtual_controller_tabs->setStyleSheet("QTabBar::close-button{image:url(:images/close.png);}");
    ui->virtual_controller_tabs->tabBar()->setStyleSheet("QTabBar::tab:hover {text-decoration: underline;}");

    QMenu* main_menu = new QMenu(this);

    QLabel* no_map = new QLabel("You have no visual map.\n You can add one by clicking the VMap button.");
    no_map->setAlignment(Qt::AlignCenter);

    /*-----------------------------------------------------*\
    | First tab: add button                                 |
    \*-----------------------------------------------------*/
    QPushButton* main_menu_button = new QPushButton();
    main_menu_button->setText("VMap");
    ui->virtual_controller_tabs->addTab(no_map, QString(""));
    ui->virtual_controller_tabs->tabBar()->setTabButton(0, QTabBar::RightSide, main_menu_button);
    ui->virtual_controller_tabs->setTabEnabled(0, false);
    main_menu_button->setMenu(main_menu);

    QAction* new_map = new QAction("New map", this);
    connect(new_map, &QAction::triggered, this, &OpenRGBVisualMapTab::AddTabSlot);
    main_menu->addAction(new_map);

    if(!SearchAndAutoLoad())
    {
         AddTab();
    }
    const auto workspace = VisualMapSettingsManager::LoadWorkspace();
    if(workspace.is_object() && workspace.contains("version") && workspace["version"] == 1 && workspace.contains("active_map"))
        LoadProfile(workspace);
}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    FlushMaps();
    delete ui;
}

void OpenRGBVisualMapTab::HideAll()
{
    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->Hide();
    }
}

void OpenRGBVisualMapTab::Clear()
{    
    LOG_INFO("[OpenRGBVisualMapPlugin] Clear\n");

    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->Clear();
    }

    LOG_INFO("[OpenRGBVisualMapPlugin] Clear done\n");
}

void OpenRGBVisualMapTab::Recreate()
{
    if(switching) return;
    LOG_INFO("[OpenRGBVisualMapPlugin] Recreate\n");

    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->Recreate();
    }

    LOG_INFO("[OpenRGBVisualMapPlugin] Recreate done\n");
}

void OpenRGBVisualMapTab::PauseForDetection()
{
    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->PauseForDetection();
    }
}

VirtualControllerTab* OpenRGBVisualMapTab::AddTab()
{
    int tab_size = ui->virtual_controller_tabs->count();

    /*-----------------------------------------------------*\
    | Insert at the end                                     |
    \*-----------------------------------------------------*/
    int tab_position = tab_size;

    std::string tab_name = "Untitled";

    VirtualControllerTab* tab = new VirtualControllerTab();
    TabHeader* tab_header = new TabHeader();
    tab_header->Rename(QString::fromUtf8(tab_name.c_str()));

    tab->RenameController(tab_name);

    ui->virtual_controller_tabs->insertTab(tab_position, tab , "");
    ui->virtual_controller_tabs->tabBar()->setTabButton(tab_position, QTabBar::RightSide, tab_header);

    ui->virtual_controller_tabs->setCurrentIndex(tab_position);

    connect(tab, &VirtualControllerTab::ControllerRenamed, [=](std::string new_name)
    {
        tab_header->Rename(QString::fromUtf8(new_name.c_str()));
    });
    connect(tab, &VirtualControllerTab::ActivationRequested, this,
            [this](VirtualControllerTab* selected, bool enabled){ Activate(selected, enabled); });

    connect(tab_header, &TabHeader::RenameRequest, [=](QString new_name)
    {
        tab->RenameController(new_name.toStdString());
    });

    connect(tab_header, &TabHeader::CloseRequest, [=]()
    {
        if(!tab->FlushSave()) return;
        if(tab->IsActive()) Activate(tab, false);
        int tab_idx = ui->virtual_controller_tabs->indexOf(tab);

        ui->virtual_controller_tabs->removeTab(tab_idx);

        controller_tabs.erase(std::find(controller_tabs.begin(), controller_tabs.end(), tab));

        delete tab;
        delete tab_header;       
    });

    ui->virtual_controller_tabs->update();

    controller_tabs.push_back(tab);

    return tab;
}

bool OpenRGBVisualMapTab::SearchAndAutoLoad()
{
    std::vector<VirtualControllerTab*> loaded_tabs;

    std::vector<std::string> filenames = VisualMapSettingsManager::GetMapNames();

    for(std::string filename : filenames)
    {
        try
        {
            json j = VisualMapSettingsManager::LoadMap(filename);

            bool auto_load = j["grid_settings"]["auto_load"];

            if(auto_load)
            {
                LOG_INFO("[OpenRGBVisualMapPlugin] Auto load: loading file %s\n", filename.c_str());
                VirtualControllerTab* tab = AddTab();
                tab->LoadFile(filename);
                loaded_tabs.push_back(tab);
            }
        }
        catch(const std::exception& e)
        {
            LOG_ERROR("[OpenRGBVisualMapPlugin] Not able to load file %s: \n%s\n", filename.c_str(), e.what());
        }
    }

    /*-----------------------------------------------------*\
    | Register only once every map has been loaded, as each |
    | registration rebuilds the controller zone list the    |
    | remaining maps still need to match against            |
    \*-----------------------------------------------------*/
    // Preserve the previous session's explicit selection before legacy auto-
    // register flags can overwrite it. A saved null means intentionally off.
    const auto workspace = VisualMapSettingsManager::LoadWorkspace();
    const bool remembered = workspace.is_object() && workspace.contains("version") && workspace["version"] == 1
                         && workspace.contains("active_map");
    for(VirtualControllerTab* tab : loaded_tabs)
    {
        if(!remembered) tab->ApplyAutoRegister();
    }

    return !loaded_tabs.empty();
}

void OpenRGBVisualMapTab::AddTabSlot()
{
    AddTab();
}

void OpenRGBVisualMapTab::FlushMaps()
{
    for(auto* tab : controller_tabs) tab->FlushSave();
}

json OpenRGBVisualMapTab::Selection() const
{
    json selected = nullptr;
    for(auto* tab : controller_tabs)
        if(tab->IsActive() && !tab->CanonicalFile().empty()) selected = tab->CanonicalFile();
    return {{"version",1},{"active_map",selected}};
}

void OpenRGBVisualMapTab::Activate(VirtualControllerTab* selected, bool enabled, bool persist)
{
    const QScopedValueRollback<bool> guard(switching, true);
    // Drain every previous producer before enabling the selected map.
    for(auto* tab : controller_tabs) tab->SuspendOutput();
    for(auto* tab : controller_tabs) tab->SetActive(false);
    if(enabled && selected) selected->SetActive(true);
    if(enabled && selected) ui->virtual_controller_tabs->setCurrentWidget(selected);
    if(persist && !VisualMapSettingsManager::SaveWorkspace(Selection()))
        LOG_ERROR("[OpenRGBVisualMapPlugin] Active map selection could not be saved");
}

VirtualControllerTab* OpenRGBVisualMapTab::FindOrLoad(const std::string& filename)
{
    if(!visual_persistence::ValidName(filename)) throw std::invalid_argument("Invalid map reference");
    for(auto* tab : controller_tabs)
        if(tab->CanonicalFile() == filename)
        { tab->LoadFile(filename); return tab; }
    // Validate before adding a tab, so a missing file cannot create a phantom.
    const auto data = VisualMapSettingsManager::LoadMap(filename);
    if(!data.is_object() || !data.contains("ctrl_zones") || !data.contains("grid_settings"))
        throw std::invalid_argument("Missing map reference: " + filename);
    auto* tab = AddTab();
    tab->LoadFile(filename);
    return tab;
}

void OpenRGBVisualMapTab::BeginProfileLoad()
{
    const auto selection = Selection();
    previous_map = selection["active_map"].is_string() ? selection["active_map"].get<std::string>() : "";
    profile_loading = true; profile_applied = false;
    FlushMaps();
    for(auto* tab : controller_tabs) tab->SuspendOutput();
}

void OpenRGBVisualMapTab::LoadProfile(const json& data)
{
    if(data.is_null()) return; // Old plugin profiles stored null.
    profile_applied = true;
    Activate(nullptr, false, false);
    try
    {
        if(!data.is_object() || data.value("version",0) != 1 || !data.contains("active_map")
           || (!data["active_map"].is_string() && !data["active_map"].is_null()))
            throw std::invalid_argument("Unsupported Visual Map profile");
        VirtualControllerTab* target = data["active_map"].is_string()
            ? FindOrLoad(data["active_map"].get<std::string>()) : nullptr;
        Activate(target, target != nullptr);
    }
    catch(const std::exception& e)
    { LOG_ERROR("[OpenRGBVisualMapPlugin] Map profile not activated: %s",e.what()); }
}

json OpenRGBVisualMapTab::SaveProfile()
{
    FlushMaps();
    return Selection();
}

void OpenRGBVisualMapTab::FinishProfileLoad()
{
    if(!profile_loading) return;
    profile_loading = false;
    if(!profile_applied)
    {
        // Profiles created before map support retain the current live map.
        VirtualControllerTab* previous = nullptr;
        for(auto* tab : controller_tabs)
            if(tab->CanonicalFile() == previous_map && tab->IsActive()) previous = tab;
        Activate(previous, previous != nullptr, false);
    }
}

