#include "VirtualControllerTab.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"
#include "WidgetEditor.h"
#include "VisualMapJsonDefinitions.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <set>
#include <QMenu>
#include <QWidgetAction>
#include <QVBoxLayout>

VirtualControllerTab::VirtualControllerTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::VirtualControllerTab),
    virtual_controller(new VirtualController())
{
    ui->setupUi(this);

    /*-------------------------------------------------*\
    | Default settings for main grid                    |
    \*-------------------------------------------------*/
    settings                = new GridSettings();
    settings->w             = 64;
    settings->h             = 64;
    settings->show_bounds   = true;
    settings->show_grid     = true;
    settings->grid_size     = 1;

    /*-------------------------------------------------*\
    | Init the grid                                     |
    \*-------------------------------------------------*/
    ui->grid->Init();
    ui->gridOptions->Init(settings);
    ui->grid->ApplySettings(settings);

    virtual_controller->UpdateSize(settings->w, settings->h);

    /*-------------------------------------------------*\
    | Init the grid                                     |
    \*-------------------------------------------------*/
    InitZoneList();

    /*-------------------------------------------------*\
    | Init other ui parts                               |
    \*-------------------------------------------------*/
    ui->itemFrame->hide();
    ui->backgroundApplier->SetSize(settings->w, settings->h);
    ui->backgroundFrame->hide();

    /*-------------------------------------------------*\
    | Listen for virtual controller updates, redraw     |
    \*-------------------------------------------------*/
    connect(this, &VirtualControllerTab::VirtualControllerPostUpdateSignal, this, &VirtualControllerTab::VirtualControllerPostUpdateSlot);

    virtual_controller->SetPostUpdateCallBack([&](const QImage& image){
        emit VirtualControllerPostUpdateSignal(image);
    });

    /*-------------------------------------------------*\
    | Init other ui parts                               |
    \*-------------------------------------------------*/
    UpdateVirtualControllerDetails();

    /*-------------------------------------------------*\
    | Init Menu                                         |
    \*-------------------------------------------------*/
    CreateMainMenu();
}

VirtualControllerTab::~VirtualControllerTab()
{
    delete virtual_controller;
    delete ui;
}

void VirtualControllerTab::CreateMainMenu()
{
    // todo move this to own method
    QMenu* main_menu = new QMenu(ui->main_menu);
    ui->main_menu->setMenu(main_menu);

    main_menu->setMaximumWidth(ui->main_menu->maximumWidth());

    register_controller = new QAction("Register controller", this);
    register_controller->setCheckable(true);
    connect(register_controller, &QAction::triggered, this, &VirtualControllerTab::RegisterAction);
    main_menu->addAction(register_controller);

    add_background = new QAction("Add background", this);
    add_background->setCheckable(true);
    connect(add_background, &QAction::triggered, this, &VirtualControllerTab::AddBackgroundAction);
    main_menu->addAction(add_background);

    QAction* save_vmap = new QAction("Save", this);
    connect(save_vmap, &QAction::triggered, this, &VirtualControllerTab::SaveVmapAction);
    main_menu->addAction(save_vmap);

    QAction* load_vmap = new QAction("Load", this);
    connect(load_vmap, &QAction::triggered, this, &VirtualControllerTab::LoadVmapAction);
    main_menu->addAction(load_vmap);

    QAction* clear = new QAction("Clear", this);
    connect(clear, &QAction::triggered, this, &VirtualControllerTab::ClearVmapAction);
    main_menu->addAction(clear);
}

void VirtualControllerTab::RenameController(std::string value)
{
    virtual_controller->name = value;
    emit ControllerRenamed(value);
}

std::string VirtualControllerTab::GetControllerName()
{
    return virtual_controller->name;
}

void VirtualControllerTab::resizeEvent(QResizeEvent*)
{
    ui->grid->update();
}

void VirtualControllerTab::UpdateVirtualControllerDetails()
{
    virtual_controller->UpdateVirtualZone();
    ui->virtual_controller_details_label->setText(QString::fromStdString("Total leds: " + std::to_string(virtual_controller->GetTotalLeds())));
}

void VirtualControllerTab::InitZoneList()
{
    retained_zones = ZoneManager::Get()->GetAvailableZones();
    ui->device_list->Init(retained_zones);
}

void VirtualControllerTab::LoadFile(std::string filename)
{
    json j = VisualMapSettingsManager::LoadMap(filename);

    RenameController(filename);

    LoadJson(j);
}

void VirtualControllerTab::LoadJson(json j)
{    
    virtual_controller->Clear();

    auto ctrl_zones = j["ctrl_zones"];
    std::vector<ControllerZone*> all_zones = retained_zones;

    bool has_failures = false;

    for (auto it = ctrl_zones.begin(); it != ctrl_zones.end(); ++it)
    {
        auto entry = it.value();
        auto controller = entry["controller"];
        auto settings = entry["settings"];

        std::vector<ControllerZone*> candidates;

        for(ControllerZone* ctrl_zone : all_zones)
        {
            /*-------------------------------------------------*\
            | Don't compare location for HID devices,           |
            | because it constantly changes                     |
            \*-------------------------------------------------*/
            bool hid_location = std::string(controller["location"]).find("HID: ") == 0;

            if(
                ctrl_zone->controller->name == controller["name"] &&
                ctrl_zone->controller->vendor == controller["vendor"] &&
                ctrl_zone->controller->serial == controller["serial"] &&
                (ctrl_zone->controller->location == controller["location"] || hid_location) &&
                ctrl_zone->zone_idx == entry["zone_idx"])
            {
                if(entry.contains("custom_zone_name"))
                {
                    ctrl_zone->custom_zone_name = entry["custom_zone_name"];
                }

                ctrl_zone->settings = settings;

                candidates.push_back(ctrl_zone);
            }
        }

        ControllerZone* zone = nullptr;
        if (candidates.size() > 1)
        {
            /*-------------------------------------------------*\
            | If we found more than onne zone matching the      |
            | saved one then fall back to comparing by location |
            | even for HID devices.                             |
            | This is a workaround to prevent us from loading   |
            | the wrong device when multiple HID devices have   |
            | the same name, vendor, serial, and zone_idx.      |
            | It won't work 100% of the time, but it will work  |
            | in some situations                                |
            \*-------------------------------------------------*/
            for (ControllerZone* z : candidates)
            {
                if (z->controller->location == controller["location"])
                {
                    zone = z;
                    break;
                }
            }
        }
        else if (candidates.size() == 1)
        {
            zone = candidates[0];
        }

        try
        {
            if (zone)
            {
                virtual_controller->Add(zone);
                ui->device_list->UpdateControllerState(zone);
                /*-------------------------------------------------*\
                | Remove this zone from the list of all zones so    |
                | that we don't modify its custom name or settings  |
                | if there's another collision.                     |
                \*-------------------------------------------------*/
                all_zones.erase(std::find(all_zones.begin(), all_zones.end(), zone));
            }
        }
        catch (const std::exception&)
        {
            has_failures = true;
        }
    }

    QPoint button_pos = ui->main_menu->cursor().pos();

    if(has_failures)
    {
        QMessageBox msgBox;
        msgBox.setText("Some of the components could not be loaded, the format is probably out of date.");
        msgBox.setWindowTitle("Sorry");
        msgBox.move(button_pos.x(), button_pos.y());
        msgBox.exec();
    }

    j.at("grid_settings").get_to(settings);

    ui->gridOptions->SetSettings(settings);

    ui->grid->ResetItems(virtual_controller->GetZones());

    virtual_controller->UpdateSize(settings->w, settings->h);

    if(settings->auto_register)
    {
        /*-------------------------------------------------*\
        | This will auto trigger registering                |
        \*-------------------------------------------------*/
        register_controller->setChecked(true);
        RegisterAction();
    }

    UpdateVirtualControllerDetails();
}

void VirtualControllerTab::on_backgroundApplier_BackgroundUpdated(const QImage& image)
{
    virtual_controller->ApplyImage(image);
}

void VirtualControllerTab::VirtualControllerPostUpdateSlot(const QImage& image)
{
    ui->grid->UpdatePreview(image);
}

void VirtualControllerTab::Unregister()
{
    virtual_controller->Register(false, false);
}

void VirtualControllerTab::Recreate()
{
    InitZoneList();

    ReassignZones();

    if(register_controller->isChecked())
    {
        virtual_controller->Register(true, settings->unregister_members);
    }
}

void VirtualControllerTab::ReassignZones()
{
    LoadJson(saved_zones);
}

void VirtualControllerTab::BackupZones()
{
    saved_zones["ctrl_zones"]       = virtual_controller->GetZones();
    saved_zones["grid_settings"]    = settings;
}

void VirtualControllerTab::Clear()
{
    ui->device_list->Clear();
    ui->grid->Clear();
    retained_zones.clear();
    selected_ctrl_zone = nullptr;
}

void VirtualControllerTab::UpdateItemOptions(std::vector<ControllerZone*> selected_controller_zones)
{
    if(selected_controller_zones.size() == 1)
    {
        ui->itemOptions->SetControllerZone(selected_controller_zones[0]);
        ui->itemFrame->show();
    }
    else
    {
        ui->itemOptions->SetControllerZone(nullptr);
        ui->itemFrame->hide();
    }
}

/*-------------------------------------------------*\
| ui element signals                                |
\*-------------------------------------------------*/
void VirtualControllerTab::on_device_list_DeviceAdded(ControllerZone* controller_zone)
{
    virtual_controller->Add(controller_zone);
    UpdateVirtualControllerDetails();
    ui->grid->ResetItems(virtual_controller->GetZones());
}


void VirtualControllerTab::on_device_list_DeviceRemoved(ControllerZone* controller_zone)
{
    virtual_controller->Remove(controller_zone);
    UpdateVirtualControllerDetails();
    ui->grid->ResetItems(virtual_controller->GetZones());
}

void VirtualControllerTab::on_device_list_SelectionChanged(std::vector<ControllerZone*> selected_controller_zones)
{
    ui->grid->SetSelection(selected_controller_zones);
    UpdateItemOptions(selected_controller_zones);
}

void VirtualControllerTab::on_grid_SelectionChanged(std::vector<ControllerZone*> selected_controller_zones)
{
    ui->device_list->SetSelection(selected_controller_zones);
    UpdateItemOptions(selected_controller_zones);
}

void VirtualControllerTab::on_itemOptions_ItemOptionsChanged()
{
    virtual_controller->UpdateVirtualZone();
    ui->grid->UpdateItems();
}

void VirtualControllerTab::on_grid_Changed()
{
    virtual_controller->UpdateVirtualZone();
    ui->itemOptions->Update();
}

void VirtualControllerTab::on_itemOptions_ShapeEditRequest(ControllerZone* controller_zone)
{
    if(controller_zone)
    {
        int result = WidgetEditor::Show(controller_zone, retained_zones);

        if(result)
        {
            on_itemOptions_ItemOptionsChanged();
        }
    }
}

void VirtualControllerTab::on_gridOptions_SettingsChanged()
{
    ui->grid->ApplySettings(settings);
    ui->backgroundApplier->SetSize(settings->w, settings->h);
    virtual_controller->UpdateSize(settings->w, settings->h);
}

void VirtualControllerTab::on_gridOptions_AutoResizeRequest()
{
    /*-------------------------------------------------*\
    | Do nothing if the controller is empty             |
    \*-------------------------------------------------*/
    if(virtual_controller->IsEmpty())
    {
        return;
    }

    /*-------------------------------------------------*\
    | Calculate bounds                                  |
    \*-------------------------------------------------*/
    int min_x = INT_MAX;
    int min_y = INT_MAX;
    int max_x = INT_MIN;
    int max_y = INT_MIN;

    for (ControllerZone* controller_zone: virtual_controller->GetZones())
    {
        min_x = std::min<int>(min_x, controller_zone->settings.x);
        min_y = std::min<int>(min_y, controller_zone->settings.y);

        max_x = std::max<int>(max_x, controller_zone->settings.x + controller_zone->width());
        max_y = std::max<int>(max_y, controller_zone->settings.y + controller_zone->height());
    }

    /*-------------------------------------------------*\
    | Shift all controllers                             |
    \*-------------------------------------------------*/
    for (ControllerZone* controller_zone: virtual_controller->GetZones())
    {
        controller_zone->settings.x -= min_x;
        controller_zone->settings.y -= min_y;
    }

    /*-------------------------------------------------*\
    | Resize the map                                    |
    \*-------------------------------------------------*/
    settings->w = max_x - min_x;
    settings->h = max_y - min_y;

    /*-------------------------------------------------*\
    | Update GUI elements                               |
    \*-------------------------------------------------*/
    ui->grid->ApplySettings(settings);
    ui->grid->UpdateItems();
    ui->gridOptions->SetSettings(settings);
}

/*-------------------------------------------------*\
| Main menu actions                                 |
\*-------------------------------------------------*/
void VirtualControllerTab::RegisterAction()
{
    virtual_controller->Register(register_controller->isChecked(), settings->unregister_members);
}

void VirtualControllerTab::AddBackgroundAction()
{
    ui->backgroundFrame->setVisible(add_background->isChecked());
}

void VirtualControllerTab::ClearVmapAction()
{
    for(ControllerZone* ctrl_zone: virtual_controller->GetZones())
    {
        ctrl_zone->settings = ControllerZoneSettings::defaults();
    }

    virtual_controller->Clear();

    ui->grid->ResetItems(virtual_controller->GetZones());

    ui->itemOptions->Update();
}

void VirtualControllerTab::SaveVmapAction()
{
    QString filename = QInputDialog::getText(
                           nullptr, "Save virtual controller", "Choose a filename",
                           QLineEdit::Normal, QString::fromUtf8(GetControllerName().c_str())).trimmed();

    if(!filename.isEmpty())
    {
        RenameController(filename.toStdString());

        json j;

        j["ctrl_zones"]     = virtual_controller->GetZones();
        j["grid_settings"]  = settings;

        VisualMapSettingsManager::SaveMap(filename.toStdString(), j);
    }
}

void VirtualControllerTab::LoadVmapAction()
{
    QStringList file_list;

    std::vector<std::string> filenames = VisualMapSettingsManager::GetMapNames();

    for(const std::string& filename : filenames)
    {
        file_list << QString::fromUtf8(filename.c_str());
    }

    QInputDialog *inp = new QInputDialog(this);

    inp->setOptions(QInputDialog::UseListViewForComboBoxItems);
    inp->setComboBoxItems(file_list);
    inp->setWindowTitle("Choose file");

    if(!inp->exec())
    {
        return;
    }

    QString filename = inp->textValue();

    LoadFile(filename.toStdString());
}
