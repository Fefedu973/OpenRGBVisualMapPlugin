#include "OpenRGBVisualMapTab.h"
#include "OpenRGBVisualMapPlugin.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"
#include "VisualMapJsonDefinitions.h"
#include "hsv.h"

OpenRGBVisualMapTab::OpenRGBVisualMapTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::OpenRGBVisualMapTab)
{
    ui->setupUi(this);
    ui->itemOptions->hide();


    InitZoneList();

    ui->gradient->SetSize(ui->grid->GetWidth(), ui->grid->GetHeight());

    connect(ui->itemOptions, SIGNAL(ItemOptionsChanged()), this, SLOT(OnItemOptionsChanged()));
    connect(ui->gradient, SIGNAL(GradientApplied(QImage*)), this, SLOT(OnGradientApplied(QImage*)));
    connect(ui->zoneList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnZoneSelectionChanged()));

    connect(ui->grid, &Grid::ItemSelected, [=](int idx){
        ui->zoneList->selectRow(idx);
    });

    connect(ui->grid, &Grid::ItemMoved, [=](int){
        ui->itemOptions->Update();
    });

    connect(ui->gridOptions, &GridOptions::OptionsChanged, [=](GridSettings settings){
        ui->grid->SetSettings(settings);
        ui->gradient->SetSize(settings.w,settings.h);
    });
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


void OpenRGBVisualMapTab::on_saveButton_clicked()
{
    std::vector<ControllerZone*> ctrl_zones = ZoneManager::Get()->GetAddedZones();
    json settings;
    settings["ctrl_zones"] = ctrl_zones;
    settings["grid_settings"] = ui->gridOptions->GetSettings();
    VisualMapSettingsManager::SaveSettings(settings);
}

void OpenRGBVisualMapTab::on_loadButton_clicked()
{
    json settings = VisualMapSettingsManager::LoadSettings();

    std::vector<ControllerZone*> available_zones = ZoneManager::Get()->GetAvailableZones();

    ZoneManager::Get()->ClearZones();

    auto ctrl_zones = settings["ctrl_zones"];

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

        for(unsigned int i = 0; i < available_zones.size(); i++)
        {
            QList<QPushButton *> buttons = ui->zoneList->cellWidget(i, 1)->findChildren<QPushButton *>();

            if(buttons.size() == 1)
            {
                DecorateButton(buttons[0], ZoneManager::Get()->HasZone(i) ? remove_icon : add_icon);
            }

        }
    }

    ui->gridOptions->SetSettings(settings["grid_settings"]);

    ui->grid->ResetItems();
}


void OpenRGBVisualMapTab::OnGradientApplied(QImage* image)
{
    ui->grid->UpdatePreview(image);

    std::vector<ControllerZone*> ctrl_zones = ZoneManager::Get()->GetAddedZones();

    for(unsigned int i = 0; i < ctrl_zones.size(); i++)
    {
        UpdateControllerZone(ctrl_zones[i], image);
    }
}


void OpenRGBVisualMapTab::UpdateControllerZone(ControllerZone* ctrl_zone, QImage* image)
{
    RGBController* controller = ctrl_zone->controller;
    zone z = controller->zones[ctrl_zone->zone_idx];
    ControllerZoneSettings settings = ctrl_zone->settings;
    int leds_count = z.leds_count;
    int start_idx = z.start_idx;

    switch (ctrl_zone->settings.shape) {
        case ControllerZoneSettings::HORIZONTAL_LINE:
            for(int i = 0; i < leds_count; i++)
            {
                int idx = settings.reverse ? leds_count - 1 - i : i;
                QColor color = image->pixelColor(idx * settings.led_spacing + settings.x, settings.y);
                controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
            }
            break;

        case ControllerZoneSettings::VERTICAL_LINE:
            for(int i = 0; i < leds_count; i++)
            {
                int idx = settings.reverse ? leds_count - 1 - i : i;

                QColor color = image->pixelColor(settings.x, idx * settings.led_spacing + settings.y);
                controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
            }
            break;

        case ControllerZoneSettings::CIRCLE:
            for(int i = 0; i < leds_count; i++)
            {

            }
            break;

        default:break;
    }

    controller->UpdateLEDs();
}
