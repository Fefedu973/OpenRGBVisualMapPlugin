#include "VirtualControllerTab.h"
#include "OpenRGBVisualMapPlugin.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"
#include "WidgetEditor.h"
#include "hsv.h"
#include "VisualMapJsonDefinitions.h"

static void VirtualControllerChangeCallback(void * this_ptr)
{
    VirtualControllerTab * _this = (VirtualControllerTab *)this_ptr;

    QMetaObject::invokeMethod(_this, "OnBackgroundApplied", Qt::QueuedConnection);
}

VirtualControllerTab::VirtualControllerTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::VirtualControllerTab),
    virtual_controller(new VirtualController())
{
    ui->setupUi(this);

    // default settings for main grid
    settings = new GridSettings();
    settings->w = 64;
    settings->h = 64;
    settings->show_bounds = true;
    settings->show_grid = true;
    settings->live_preview = true;
    settings->grid_size = 1;
    settings->grid_scale_factor = 1;

    ui->grid->Init(settings);
    ui->gridOptions->Init(settings);

    virtual_controller->UpdateSize(settings->w, settings->h);

    InitZoneList();

    ui->itemOptions->hide();
    ui->backgroundApplier->SetSize(settings->w, settings->h);

    connect(this, SIGNAL(ApplyBackground(QImage*)), this, SLOT(OnBackgroundApplied(QImage*)));
    connect(ui->itemOptions, SIGNAL(ItemOptionsChanged()), this, SLOT(OnItemOptionsChanged()));
    connect(ui->backgroundApplier, SIGNAL(BackgroundApplied(QImage*)), this, SLOT(OnBackgroundApplied(QImage*)));
    connect(ui->zoneList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnZoneSelectionChanged()));

    connect(ui->grid, &Grid::ItemSelected, [=](int idx){
        ui->zoneList->selectRow(idx);
    });

    connect(ui->grid, &Grid::ItemMoved, [=](int){
        ui->itemOptions->Update();
    });

    connect(ui->gridOptions, &GridOptions::SettingsChanged, [=](){
        ui->grid->OnSettingsChanged();
        ui->backgroundApplier->SetSize(settings->w,settings->h);
        virtual_controller->UpdateSize(settings->w, settings->h);
    });

    virtual_controller->SetCallBack([=](QImage* image){
        emit ApplyBackground(image);
    });
}

void VirtualControllerTab::RenameController(std::string value)
{
    virtual_controller->name = value;
}

void VirtualControllerTab::DecorateButton(QPushButton* button, QIcon icon)
{
    button->setIcon(icon);
}

void VirtualControllerTab::resizeEvent(QResizeEvent*)
{
    ui->grid->update();
}

VirtualControllerTab::~VirtualControllerTab()
{
    delete ui;
}

void VirtualControllerTab::InitZoneList()
{
    std::vector<ControllerZone*> retained_zones = ZoneManager::Get()->GetAvailableZones();

    // Hide headers
    ui->zoneList->horizontalHeader()->hide();
    ui->zoneList->verticalHeader()->hide();

    // Set size
    ui->zoneList->setRowCount(retained_zones.size());
    ui->zoneList->setColumnCount(2);
    ui->zoneList->setColumnWidth(1, 20);

    // Set selection options
    ui->zoneList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->zoneList->setFocusPolicy(Qt::NoFocus);
    ui->zoneList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->zoneList->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Set stretch modes
    ui->zoneList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->zoneList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->zoneList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Fill the table
    for(unsigned int i = 0; i < retained_zones.size(); i++)
    {
        // Cell 1 : device name + zone name
        std::string display_name = retained_zones[i]->controller->name + "\n" + retained_zones[i]->controller->zones[retained_zones[i]->zone_idx].name;
        ui->zoneList->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(display_name)));

        // Cell 2 : add/remove button
        QWidget* widget = new QWidget();
        QPushButton* button = new QPushButton();
        DecorateButton(button, add_icon);
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->addWidget(button);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        widget->setLayout(layout);
        ui->zoneList->setCellWidget(i, 1, widget);

        connect(button, &QPushButton::clicked, [=]() {
            if(std::find(added_zones.begin(), added_zones.end(),retained_zones[i]) == added_zones.end())
            {
                added_zones.push_back(retained_zones[i]);
                DecorateButton(button, remove_icon);
            }
            else
            {
                added_zones.erase(std::find(added_zones.begin(), added_zones.end(),retained_zones[i]));
                DecorateButton(button, add_icon);
            }

            ui->grid->ResetItems(added_zones);
        });
    }

}

void VirtualControllerTab::OnZoneSelectionChanged()
{    
    int selected_idx = ui->zoneList->selectionModel()->currentIndex().row();
    ui->itemOptions->SetControllerZone(ZoneManager::Get()->GetZone(selected_idx));
    ui->itemOptions->show();
    ui->grid->SetSelected(selected_idx);
}


void VirtualControllerTab::OnItemOptionsChanged()
{
    ui->grid->UpdateItems();
}

void VirtualControllerTab::on_register_controller_stateChanged(int value)
{
    if(value)
    {
        OpenRGBVisualMapPlugin::RMPointer->RegisterRGBController(virtual_controller);
    }
    else
    {
        OpenRGBVisualMapPlugin::RMPointer->UnregisterRGBController(virtual_controller);
    }
}

void VirtualControllerTab::on_resetButton_clicked()
{   
    for(ControllerZone* ctrl_zone: added_zones)
    {
        ctrl_zone->settings = ControllerZoneSettings::defaults();
    }

    added_zones.clear();

    ui->grid->ResetItems(added_zones);

    UpdateZoneButtons();

    ui->itemOptions->Update();
}

void VirtualControllerTab::on_saveButton_clicked()
{
    json j;
    j["ctrl_zones"] = added_zones;
    j["grid_settings"] = settings;
    VisualMapSettingsManager::SaveSettings(j);
}

void VirtualControllerTab::on_loadButton_clicked()
{
    json j = VisualMapSettingsManager::LoadSettings();

    std::vector<ControllerZone*> available_zones = ZoneManager::Get()->GetAvailableZones();

    added_zones.clear();

    auto ctrl_zones = j["ctrl_zones"];

    for (auto it = ctrl_zones.begin(); it != ctrl_zones.end(); ++it)
    {
        auto entry = it.value();
        auto controller = entry["controller"];
        auto settings = entry["settings"];

        for(unsigned int i= 0; i < available_zones.size(); i++)
        {
            ControllerZone* ctrl_zone = available_zones[i];
            if(
                    ctrl_zone->controller->name     == controller["name"]     &&
                    ctrl_zone->controller->location == controller["location"] &&
                    ctrl_zone->controller->serial   == controller["serial"]   &&
                    ctrl_zone->controller->vendor   == controller["vendor"]   &&
                    ctrl_zone->zone_idx == entry["zone_idx"]
                    )
            {
                ctrl_zone->settings = settings;
                added_zones.push_back(available_zones[i]);
            }
        }

    }

    UpdateZoneButtons();

    j.at("grid_settings").get_to(settings);

    ui->gridOptions->SetSettings(settings);

    ui->grid->ResetItems(added_zones);

    virtual_controller->UpdateSize(settings->w, settings->h);
}

void VirtualControllerTab::UpdateZoneButtons()
{
    std::vector<ControllerZone*> available_zones = ZoneManager::Get()->GetAvailableZones();

    for(unsigned int i = 0; i < available_zones.size(); i++)
    {
        QList<QPushButton *> buttons = ui->zoneList->cellWidget(i, 1)->findChildren<QPushButton *>();

        if(buttons.size() == 1)
        {
            bool zone_added = std::find(added_zones.begin(), added_zones.end(), available_zones[i]) != added_zones.end();
            DecorateButton(buttons[0], zone_added ? remove_icon : add_icon);
        }

    }
}

void VirtualControllerTab::OnBackgroundApplied(QImage* image)
{
    if(!image)
    {
        return;
    }

    if(settings->live_preview)
    {
        ui->grid->UpdatePreview(image);        
    }

    ZoneManager::Get()->ApplyImage(added_zones, image);

    delete image;
}

