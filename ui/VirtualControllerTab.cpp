#include "VirtualControllerTab.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"
#include "WidgetEditor.h"
#include "hsv.h"
#include "VisualMapJsonDefinitions.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <set>

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

    ui->grid->Init(settings);
    ui->gridOptions->Init(settings);
    ui->grid->ApplySettings(settings);

    virtual_controller->UpdateSize(settings->w, settings->h);

    InitZoneList();

    ui->itemOptions->hide();
    ui->backgroundApplier->SetSize(settings->w, settings->h);

    connect(this, SIGNAL(ApplyBackground(QImage)), this, SLOT(OnBackgroundApplied(QImage)));
    connect(ui->itemOptions, SIGNAL(ItemOptionsChanged()), this, SLOT(OnItemOptionsChanged()));
    connect(ui->backgroundApplier, SIGNAL(BackgroundApplied(QImage)), this, SLOT(OnBackgroundApplied(QImage)));
    connect(ui->zoneList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnZoneSelectionChanged()));
    connect(ui->grid, SIGNAL(SelectionChanged()), this, SLOT(OnGridSelectionChanged()));
    connect(ui->zoneList, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnZoneDoubleClick(int, int)));
    connect(ui->gridOptions, SIGNAL(SettingsChanged()), this, SLOT(OnSettingsChanged()));

    connect(ui->itemOptions, &ItemOptions::ShapeEditRequest, [=](ControllerZone* ctrl_zone){
        if(ctrl_zone)
        {
            int result = WidgetEditor::Show(ctrl_zone, retained_zones);

            if(result)
            {
                OnItemOptionsChanged();
            }
        }
    });

    connect(ui->grid, &Grid::Changed, [=](){
        ui->itemOptions->Update();
    });

    virtual_controller->SetCallBack([=](QImage image){
        emit ApplyBackground(image);
    });

    UpdateVirtualControllerDetails();
}

VirtualControllerTab::~VirtualControllerTab()
{
    delete virtual_controller;
    delete ui;
}

void VirtualControllerTab::OnSettingsChanged()
{
    ui->grid->ApplySettings(settings);
    ui->backgroundApplier->SetSize(settings->w, settings->h);
    virtual_controller->UpdateSize(settings->w, settings->h);
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

void VirtualControllerTab::UpdateVirtualControllerDetails()
{
    ui->virtual_controller_details_label->setText(
                QString::fromStdString("Total leds: " + std::to_string(virtual_controller->GetTotalLeds())));
}

void VirtualControllerTab::InitZoneList()
{
    retained_zones = ZoneManager::Get()->GetAvailableZones();

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
    ui->zoneList->setSelectionMode(QAbstractItemView::MultiSelection);
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
            if(!virtual_controller->HasZone(retained_zones[i]))
            {
                virtual_controller->Add(retained_zones[i]);
                ui->zoneList->selectRow(i);
                DecorateButton(button, remove_icon);
            }
            else
            {
                virtual_controller->Remove(retained_zones[i]);

                if(selected_ctrl_zone == retained_zones[i])
                {
                    ui->grid->ClearSelection();
                    ui->zoneList->clearSelection();
                    ui->itemOptions->hide();
                }

                DecorateButton(button, add_icon);
            }


            UpdateVirtualControllerDetails();

            ui->grid->ResetItems(virtual_controller->GetZones());
        });
    }

}

void VirtualControllerTab::OnZoneSelectionChanged()
{
    QModelIndexList selected_indexes = ui->zoneList->selectionModel()->selectedRows();

    std::vector<ControllerZone*> selection;

    for(int i = 0; i < selected_indexes.size(); i++)
    {
        int index = selected_indexes.at(i).row();
        selection.push_back(retained_zones[index]);
    }

    if(selection.size() == 1)
    {
        ui->itemOptions->SetControllerZone(selection.front());
        ui->itemOptions->show();
    }
    else
    {
        ui->itemOptions->SetControllerZone(nullptr);
        ui->itemOptions->hide();
    }

    ui->grid->SetSelection(selection);
}

void VirtualControllerTab::OnGridSelectionChanged()
{
    std::vector<ControllerZoneItem*> selected_items = ui->grid->GetSelection();

    ui->zoneList->selectionModel()->blockSignals(true);

    ui->zoneList->clearSelection();

    for(unsigned int i = 0; i < retained_zones.size(); i++)
    {
        for(ControllerZoneItem* item: selected_items)
        {
            if(item->GetControllerZone() == retained_zones[i])
            {
                ui->zoneList->selectRow(i);
                break;
            }
        }
    }

    if(selected_items.size() == 1)
    {
        ui->itemOptions->SetControllerZone(selected_items[0]->GetControllerZone());
        ui->itemOptions->show();
    }
    else
    {
        ui->itemOptions->SetControllerZone(nullptr);
        ui->itemOptions->hide();
    }


    ui->zoneList->selectionModel()->blockSignals(false);

    ui->zoneList->update();
}

void VirtualControllerTab::OnZoneDoubleClick(int row, int)
{
    ControllerZone* ctrl_zone = retained_zones[row];

    std::string old_name = ctrl_zone->controller->zones[ctrl_zone->zone_idx].name;

    QString new_name = QInputDialog::getText(
                nullptr, "Rename zone", "Set the new name",
                QLineEdit::Normal, QString::fromUtf8(old_name.c_str())).trimmed();

    if(!new_name.isEmpty())
    {
        ctrl_zone->custom_zone_name = new_name.toStdString();
        ui->zoneList->item(row, 0)->setText(new_name);
    }
}

void VirtualControllerTab::OnItemOptionsChanged()
{
    ui->grid->UpdateItems();
}

void VirtualControllerTab::on_register_controller_stateChanged(int value)
{
    virtual_controller->Register(value, settings->unregister_members);
}

void VirtualControllerTab::on_clearButton_clicked()
{   
    for(ControllerZone* ctrl_zone: virtual_controller->GetZones())
    {
        ctrl_zone->settings = ControllerZoneSettings::defaults();
    }

    virtual_controller->Clear();

    ui->grid->ResetItems(virtual_controller->GetZones());

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

        j["ctrl_zones"] = virtual_controller->GetZones();
        j["grid_settings"] = settings;

        VisualMapSettingsManager::SaveMap(filename.toStdString(), j);
    }
}

void VirtualControllerTab::on_loadButton_clicked()
{
    QStringList file_list;

    std::vector<std::string> filenames = VisualMapSettingsManager::GetMapNames();

    for(std::string filename : filenames)
    {
        file_list << QString::fromUtf8(filename.c_str());
    }

    QInputDialog *inp = new QInputDialog(this);

    inp->setOptions(QInputDialog::UseListViewForComboBoxItems);
    inp->setComboBoxItems(file_list);
    inp->setWindowTitle("Choose file");

    QPoint position = ui->optionsLayout->contentsRect().topLeft();
    inp->move(position.x(), position.y());

    if(!inp->exec()){
        return;
    }

    QString filename = inp->textValue();

    LoadFile(filename.toStdString());
}

void VirtualControllerTab::LoadFile(std::string filename)
{
    json j = VisualMapSettingsManager::LoadMap(filename);

    virtual_controller->Clear();

    auto ctrl_zones = j["ctrl_zones"];

    bool has_failures = false;

    for (auto it = ctrl_zones.begin(); it != ctrl_zones.end(); ++it)
    {
        auto entry = it.value();
        auto controller = entry["controller"];
        auto settings = entry["settings"];

        for(unsigned int i= 0; i < retained_zones.size(); i++)
        {
            ControllerZone* ctrl_zone = retained_zones[i];

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

                    virtual_controller->Add(retained_zones[i]);

                    ui->zoneList->item(i,0)->setText(QString::fromUtf8(ctrl_zone->display_name().c_str()));

                } catch(const std::exception& e)
                {
                    has_failures = true;
                }
            }
        }
    }

    QPoint button_pos = ui->loadButton->cursor().pos();

    if(has_failures)
    {
        QMessageBox msgBox;
        msgBox.setText("Some of the components could not be loaded, the format is probably out of date.");
        msgBox.setWindowTitle("Sorry");
        msgBox.move(button_pos.x(), button_pos.y());
        msgBox.exec();
    }

    RenameController(filename);

    UpdateZoneButtons();

    j.at("grid_settings").get_to(settings);

    ui->gridOptions->SetSettings(settings);

    ui->grid->ResetItems(virtual_controller->GetZones());

    virtual_controller->UpdateSize(settings->w, settings->h);

    if(settings->auto_register)
    {
        // will auto trigger registering
        ui->register_controller->setChecked(true);
    }

    UpdateVirtualControllerDetails();
}

void VirtualControllerTab::UpdateZoneButtons()
{
    for(unsigned int i = 0; i < retained_zones.size(); i++)
    {
        QList<QPushButton *> buttons = ui->zoneList->cellWidget(i, 1)->findChildren<QPushButton *>();

        if(buttons.size() == 1)
        {
            DecorateButton(buttons[0], virtual_controller->HasZone(retained_zones[i]) ? remove_icon : add_icon);
        }

    }
}

void VirtualControllerTab::OnBackgroundApplied(QImage image)
{
    if(settings->live_preview)
    {
        ui->grid->UpdatePreview(image);
    }

    virtual_controller->ApplyImage(image);
}

void VirtualControllerTab::Unregister()
{
    virtual_controller->Register(false, false);
}

void VirtualControllerTab::Clear()
{
    ui->zoneList->clear();
    ui->grid->Clear();
    virtual_controller->Clear();
}

void VirtualControllerTab::DeviceListChanged()
{
    InitZoneList();
}
