#include "VirtualControllerTab.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"
#include "WidgetEditor.h"
#include "hsv.h"
#include "VisualMapJsonDefinitions.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>

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

    connect(this, SIGNAL(ApplyBackground(QImage)), this, SLOT(OnBackgroundApplied(QImage)));
    connect(ui->itemOptions, SIGNAL(ItemOptionsChanged()), this, SLOT(OnItemOptionsChanged()));
    connect(ui->backgroundApplier, SIGNAL(BackgroundApplied(QImage)), this, SLOT(OnBackgroundApplied(QImage)));
    connect(ui->zoneList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnZoneSelectionChanged()));

    connect(ui->grid, &Grid::ItemSelected, [=](ControllerZone* ctrl_zone){
        std::vector<ControllerZone*> ctrl_zones = ZoneManager::Get()->GetAvailableZones();
        for(unsigned int i = 0; i < ctrl_zones.size(); i++)
        {
            if(ctrl_zones[i] == ctrl_zone)
            {
                ui->zoneList->selectRow(i);
                break;
            }
        }
    });

    connect(ui->grid, &Grid::ItemMoved, [=](ControllerZone*){
        ui->itemOptions->Update();
    });

    connect(ui->gridOptions, &GridOptions::SettingsChanged, [=](){
        ui->grid->ApplySettings(settings);
        ui->backgroundApplier->SetSize(settings->w, settings->h);
        virtual_controller->UpdateSize(settings->w, settings->h);
    });

    connect(ui->zoneList, &QTableWidget::cellDoubleClicked,[=](int r, int){

        ControllerZone* ctrl_zone = ZoneManager::Get()->GetZone(r);

        std::string old_name = ctrl_zone->controller->zones[ctrl_zone->zone_idx].name;

        QString new_name = QInputDialog::getText(
                    nullptr, "Rename zone", "Set the new name",
                    QLineEdit::Normal, QString::fromUtf8(old_name.c_str())).trimmed();

        if(!new_name.isEmpty())
        {
            ctrl_zone->custom_zone_name = new_name.toStdString();
            ui->zoneList->item(r,0)->setText(new_name);
        }

    });


    virtual_controller->SetCallBack([=](QImage image){
        emit ApplyBackground(image);
    });
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
    delete virtual_controller;
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
        // Cell 1 : ControllerZone display name
        ui->zoneList->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(retained_zones[i]->display_name())));

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
                ui->zoneList->selectRow(i);
                DecorateButton(button, remove_icon);
            }
            else
            {
                added_zones.erase(std::find(added_zones.begin(), added_zones.end(),retained_zones[i]));

                if(selected_ctrl_zone == retained_zones[i])
                {
                     ui->grid->ClearSelection();
                     ui->zoneList->clearSelection();
                     ui->itemOptions->hide();
                }

                DecorateButton(button, add_icon);
            }

            ui->grid->ResetItems(added_zones);
        });
    }

}

void VirtualControllerTab::OnZoneSelectionChanged()
{    
    int selected_idx = ui->zoneList->selectionModel()->currentIndex().row();
    selected_ctrl_zone = ZoneManager::Get()->GetZone(selected_idx);
    ui->itemOptions->SetControllerZone(selected_ctrl_zone);
    ui->itemOptions->show();
    ui->grid->SetSelected(selected_ctrl_zone);
}

void VirtualControllerTab::OnItemOptionsChanged()
{
    ui->grid->UpdateItems();
}

void VirtualControllerTab::on_register_controller_stateChanged(int value)
{
    virtual_controller->Register(value);
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
    QString filename = QInputDialog::getText(
                nullptr, "Save virtual controller", "Choose a filename",
                QLineEdit::Normal, QString::fromUtf8(GetControllerName().c_str())).trimmed();

    if(!filename.isEmpty())
    {
        RenameController(filename.toStdString());
        json j;
        j["ctrl_zones"] = added_zones;
        j["grid_settings"] = settings;
        VisualMapSettingsManager::SaveSettings(filename.toStdString(), j);
    }
}

void VirtualControllerTab::on_loadButton_clicked()
{    
    QPoint button_pos = ui->loadButton->cursor().pos();

    QStringList file_list;

    std::vector<std::string> filenames = VisualMapSettingsManager::GetFileNames();

    for(std::string filename : filenames)
    {
        file_list << QString::fromUtf8(filename.c_str());
    }

    QInputDialog *inp = new QInputDialog(this);

    inp->setOptions(QInputDialog::UseListViewForComboBoxItems);
    inp->setComboBoxItems(file_list);
    inp->setWindowTitle("Choose file");
    inp->move(button_pos.x(), button_pos.y());

    if(!inp->exec()){
        return;
    }

    QString filename = inp->textValue();

    json j = VisualMapSettingsManager::LoadSettings(filename.toStdString());

    std::vector<ControllerZone*> available_zones = ZoneManager::Get()->GetAvailableZones();

    added_zones.clear();

    auto ctrl_zones = j["ctrl_zones"];

    bool has_failures = false;

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
                try
                {
                    if(entry.contains("custom_zone_name"))
                    {
                        ctrl_zone->custom_zone_name = entry["custom_zone_name"];
                    }

                    ctrl_zone->settings = settings;

                    added_zones.push_back(available_zones[i]);

                    ui->zoneList->item(i,0)->setText(QString::fromUtf8(ctrl_zone->display_name().c_str()));

                } catch(const std::exception& e)
                {
                    has_failures = true;
                }
            }
        }
    }

    if(has_failures)
    {
        QMessageBox msgBox;
        msgBox.setText("Some of the components could not be loaded, the format is probably out of date.");
        msgBox.setWindowTitle("Sorry");
        msgBox.move(button_pos.x(), button_pos.y());
        msgBox.exec();
    }

    RenameController(filename.toStdString());

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

void VirtualControllerTab::OnBackgroundApplied(QImage image)
{
    if(settings->live_preview)
    {
        ui->grid->UpdatePreview(image);        
    }

    ZoneManager::Get()->ApplyImage(added_zones, image);
}

