#include "OpenRGBVisualMapTab.h"
#include "OpenRGBVisualMapPlugin.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"
#include "WidgetEditor.h"
#include "VisualMapJsonDefinitions.h"
#include "hsv.h"
#include "VirtualControllerProvider.h"
#include "EventEmitter.h"

OpenRGBVisualMapTab::OpenRGBVisualMapTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::OpenRGBVisualMapTab)
{
    ui->setupUi(this);

    // default settings for main grid
    settings = new GridSettings();
    settings->w = 64;
    settings->h = 64;
    settings->show_bounds = true;
    settings->show_grid = true;
    settings->grid_size = 8;
    settings->grid_scale_factor = 1;

    ui->grid->Init(settings);
    ui->gridOptions->Init(settings);

    VirtualControllerProvider::Get()->UpdateSize(settings->w, settings->h);

    InitZoneList();

    ui->itemOptions->hide();
    ui->backgroundApplier->SetSize(settings->w, settings->h);

    connect(ui->itemOptions, SIGNAL(ItemOptionsChanged()), this, SLOT(OnItemOptionsChanged()));
    connect(ui->backgroundApplier, SIGNAL(BackgroundApplied(QImage*)), this, SLOT(OnBackgroundApplied(QImage*)));
    connect(ui->zoneList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnZoneSelectionChanged()));

    connect(ui->grid, &Grid::ItemSelected, [=](int idx){
        ui->zoneList->selectRow(idx);
    });

    connect(ui->grid, &Grid::ItemMoved, [=](int){
        OnBackgroundApplied(ui->backgroundApplier->GetImage());
        ui->itemOptions->Update();
    });

    connect(ui->gridOptions, &GridOptions::SettingsChanged, [=](){
        ui->grid->OnSettingsChanged();
        ui->backgroundApplier->SetSize(settings->w,settings->h);
        VirtualControllerProvider::Get()->UpdateSize(settings->w, settings->h);
    });

    connect(EventEmitter::Get(), SIGNAL(ImageApplied(QImage*)),
            this, SLOT(OnBackgroundApplied(QImage*)),Qt::QueuedConnection);

}

void OpenRGBVisualMapTab::DecorateButton(QPushButton* button, QIcon icon)
{
    button->setIcon(icon);
}

void OpenRGBVisualMapTab::resizeEvent(QResizeEvent*)
{
    ui->grid->update();
}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    delete ui;
}

void OpenRGBVisualMapTab::InitZoneList()
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
            if(!ZoneManager::Get()->HasZone(i))
            {
                ZoneManager::Get()->AddZone(i);
                DecorateButton(button, remove_icon);
            }
            else
            {
                ZoneManager::Get()->RemoveZone(i);
                DecorateButton(button, add_icon);
            }

            ui->grid->ResetItems();
        });
    }

}

void OpenRGBVisualMapTab::OnZoneSelectionChanged()
{    
    int selected_idx = ui->zoneList->selectionModel()->currentIndex().row();
    ui->itemOptions->SetControllerZone(selected_idx);
    ui->itemOptions->show();
    ui->grid->SetSelected(selected_idx);
}

void OpenRGBVisualMapTab::OnItemOptionsChanged()
{
    ui->grid->UpdateItems();
}

void OpenRGBVisualMapTab::on_resetButton_clicked()
{
    ZoneManager::Get()->ClearZones();

    ui->grid->ResetItems();

    UpdateZoneButtons();

    std::vector<ControllerZone*> ctrl_zones = ZoneManager::Get()->GetAvailableZones();

    for(ControllerZone* ctrl_zone:ctrl_zones)
    {
        ctrl_zone->settings = ControllerZoneSettings::defaults();
    }

    ui->itemOptions->Update();
}

void OpenRGBVisualMapTab::on_saveButton_clicked()
{
    std::vector<ControllerZone*> ctrl_zones = ZoneManager::Get()->GetAddedZones();
    json j;
    j["ctrl_zones"] = ctrl_zones;
    j["grid_settings"] = settings;
    VisualMapSettingsManager::SaveSettings(j);
}

void OpenRGBVisualMapTab::on_loadButton_clicked()
{
    json j = VisualMapSettingsManager::LoadSettings();

    std::vector<ControllerZone*> available_zones = ZoneManager::Get()->GetAvailableZones();

    ZoneManager::Get()->ClearZones();

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
                ZoneManager::Get()->AddZone(i);
            }
        }

    }

    UpdateZoneButtons();

    j.at("grid_settings").get_to(settings);

    ui->gridOptions->SetSettings(settings);

    ui->grid->ResetItems();

    VirtualControllerProvider::Get()->UpdateSize(settings->w, settings->h);
}

void OpenRGBVisualMapTab::UpdateZoneButtons()
{
    std::vector<ControllerZone*> available_zones = ZoneManager::Get()->GetAvailableZones();

    for(unsigned int i = 0; i < available_zones.size(); i++)
    {
        QList<QPushButton *> buttons = ui->zoneList->cellWidget(i, 1)->findChildren<QPushButton *>();

        if(buttons.size() == 1)
        {
            DecorateButton(buttons[0], ZoneManager::Get()->HasZone(i) ? remove_icon : add_icon);
        }

    }
}

void OpenRGBVisualMapTab::OnBackgroundApplied(QImage* image)
{
    if(!image)
    {
        return;
    }

    ui->grid->UpdatePreview(image);
    ZoneManager::Get()->ApplyImage(image);

    delete image;
}

