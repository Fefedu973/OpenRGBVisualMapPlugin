#include "WidgetEditor.h"
#include "ui_WidgetEditor.h"

#include "OpenRGBVisualMapPlugin.h"
#include "ZoneManager.h"

#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QFile>
#include <QPoint>
#include <QInputDialog>
#include <QTransform>
#include <QRect>

WidgetEditor::WidgetEditor(QWidget *parent, ControllerZone* ctrl_zone):
    QWidget(parent),
    ui(new Ui::WidgetEditor),
    ctrl_zone(ctrl_zone)
{
    ui->setupUi(this);

    ResetShape();

    settings = new GridSettings();

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;
    settings->show_grid = true;
    settings->show_bounds = true;
    settings->live_preview = false;
    settings->grid_size = 1;
    settings->grid_scale_factor = 10;

    ui->grid->ApplySettings(settings);

    ui->identify_button->hide();

    connect(ui->grid, &EditorGrid::ItemSelected, [=](int idx){
        ui->identify_button->setVisible(idx >= 0);

        if(ui->auto_identify->isChecked())
        {
            IdentifySelected();
        }
    });

    ui->grid->CreateLEDItems(temp_shape);

    UpdateWidgetsValues();
}

WidgetEditor::~WidgetEditor()
{
    delete ui;
}

int WidgetEditor::Show(ControllerZone* ctrl_zone)
{
    WidgetEditor* editor = new WidgetEditor(nullptr, ctrl_zone);

    QDialog* dialog = new QDialog();

    if (OpenRGBVisualMapPlugin::DarkTheme)
    {
        QPalette pal;
        pal.setColor(QPalette::WindowText, Qt::white);
        dialog->setPalette(pal);
        QFile dark_theme(":/windows_dark.qss");
        dark_theme.open(QFile::ReadOnly);
        dialog->setStyleSheet(dark_theme.readAll());
        dark_theme.close();
    }

    std::string title = "Widget editor: " + ctrl_zone->display_name();

    dialog->setWindowTitle(QString::fromUtf8(title.c_str()));
    dialog->setMinimumSize(814,489);
    dialog->setModal(true);


    QVBoxLayout* dialog_layout = new QVBoxLayout(dialog);

    dialog_layout->addWidget(editor);
    dialog->setLayout(dialog_layout);

    editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    connect(editor, &WidgetEditor::Save, [=](){
        dialog->accept();
    });

    connect(editor, &WidgetEditor::Cancel, [=](){
        dialog->reject();
    });

    int result = dialog->exec();

    // hmmmm delete anyway ?
    if (result)
    {
        delete dialog;
    }

    return result;
}

void WidgetEditor::UpdateWidgetsValues()
{
    ui->led_count->setValue(ctrl_zone->led_count());
    ui->w_spinBox->setValue(temp_shape->w);
    ui->h_spinBox->setValue(temp_shape->h);
}

void WidgetEditor::on_identify_button_clicked()
{
    IdentifySelected();
}

void WidgetEditor::on_reset_button_clicked()
{
    ResetShape();
}

void WidgetEditor::ResetShape()
{
    temp_shape = new CustomShape();

    // custom shape already exists, copy it to temp shape
    if(ctrl_zone->settings.custom_shape)
    {
        temp_shape->w = ctrl_zone->settings.custom_shape->w;
        temp_shape->h = ctrl_zone->settings.custom_shape->h;

        for(LedPosition* led_position : ctrl_zone->settings.custom_shape->led_positions)
        {
            LedPosition* temp_led_position = new LedPosition();
            temp_led_position->led_num = led_position->led_num;
            temp_led_position->setX(led_position->x());
            temp_led_position->setY(led_position->y());

            temp_shape->led_positions.push_back(temp_led_position);
        }
    }
    // custom shape does not exist, generate one (horizontal line)
    else
    {
        int led_count = ctrl_zone->led_count();

        temp_shape->w = ctrl_zone->led_count();
        temp_shape->h = 1;
        temp_shape->led_positions.resize(led_count);

        for(int i = 0; i < led_count; i++)
        {
            LedPosition* temp_led_position = new LedPosition();
            temp_led_position->led_num = i;
            temp_led_position->setX(i);
            temp_led_position->setY(0);

            temp_shape->led_positions.push_back(temp_led_position);
        }
    }

}

void WidgetEditor::on_copy_shape_button_clicked()
{
    std::vector<ControllerZone*> ctrl_zones = ZoneManager::Get()->GetAvailableZones();

    QStringList items;

    std::map<QString, ControllerZone*> ctrl_zones_choices;

    int i = 0;
    for(ControllerZone* ctrl_zone_it : ctrl_zones)
    {
        // ignore current ctrl_zone
        if(ctrl_zone == ctrl_zone_it)
        {
            continue;
        }

        if(ctrl_zone->led_count() != ctrl_zone_it->led_count())
        {
            continue;
        }

        if(!ctrl_zone_it->isCustomShape())
        {
            continue;
        }

        std::string item_text = std::to_string(i+1) + ". " +ctrl_zone_it->display_name() + "(" + std::to_string(ctrl_zone_it->led_count()) +")";
        QString choice = QString::fromUtf8(item_text.c_str());

        items << choice;

        ctrl_zones_choices[choice] = ctrl_zone_it;
        i++;
    }

    QPoint button_pos = ui->copy_shape_button->cursor().pos();

    if(items.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("No other eligible shape found.\nMake sure you have a similar device zone (number of leds has to be the same).");
        msgBox.setWindowTitle("Oooops");
        msgBox.move(button_pos.x(), button_pos.y());
        msgBox.exec();
        return;
    }

    QInputDialog *inp = new QInputDialog(this);

    inp->setOptions(QInputDialog::UseListViewForComboBoxItems);
    inp->setComboBoxItems(items);
    inp->setWindowTitle("Choose shape");
    inp->move(button_pos.x(), button_pos.y());

    if(inp->exec()){        
        QString selected = inp->textValue();
        ControllerZone* selected_ctrl_zone = ctrl_zones_choices[selected];

        temp_shape = new CustomShape();
        temp_shape->w = selected_ctrl_zone->settings.custom_shape->w;
        temp_shape->h = selected_ctrl_zone->settings.custom_shape->h;

        temp_shape->led_positions.resize(selected_ctrl_zone->settings.custom_shape->led_positions.size());

        for(LedPosition* led_position: selected_ctrl_zone->settings.custom_shape->led_positions)
        {
           LedPosition* temp_led_position = new LedPosition();
           temp_led_position->led_num = led_position->led_num;
           temp_led_position->setX(led_position->x());
           temp_led_position->setY(led_position->y());

           temp_shape->led_positions.push_back(temp_led_position);
        }

        ui->grid->CreateLEDItems(temp_shape);

        UpdateWidgetsValues();
    }

}

void WidgetEditor::on_cancel_button_clicked()
{
    emit Cancel();
}

void WidgetEditor::on_save_button_clicked()
{
    ctrl_zone->settings.custom_shape = temp_shape;
    emit Save();
}

void WidgetEditor::on_w_spinBox_valueChanged(int value)
{
    settings->w = value;
    temp_shape->w = value;
    ui->grid->ApplySettings(settings);
}

void WidgetEditor::on_h_spinBox_valueChanged(int value)
{
    settings->h = value;
    temp_shape->h = value;
    ui->grid->ApplySettings(settings);
}

void WidgetEditor::on_auto_identify_stateChanged(int state)
{
    if(state)
    {
        IdentifySelected();
    }
}

void WidgetEditor::IdentifySelected()
{
    int led_num = ui->grid->GetSelected();

    if(led_num >= 0)
    {
        ZoneManager::Get()->IdentifyLed(ctrl_zone, led_num);
    }
}

void WidgetEditor::on_rotate_button_clicked()
{
    // Resize first (swap w and h)
    int new_width  = temp_shape->h;
    int new_height = temp_shape->w;

    temp_shape->w = new_width;
    temp_shape->h = new_height;

    UpdateWidgetsValues();

    // Apply clockwise rotation then
    QTransform t = QTransform().rotate(90);

    for(LedPosition* led_position : temp_shape->led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x() + new_width - 1);
        led_position->setY(new_pos.y());
    }

    // Fix the glitch with UpdateItems (some leds disappear, need to zoom out/in after update)
    // Lets recreate the items instead
    //ui->grid->UpdateItems();
    ui->grid->CreateLEDItems(temp_shape);
}

void WidgetEditor::on_v_flip_button_clicked()
{
    QTransform t = QTransform().scale(1,-1);

    for(LedPosition* led_position : temp_shape->led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x());
        led_position->setY(new_pos.y() + temp_shape->h - 1);
    }

    ui->grid->UpdateItems();
}

void WidgetEditor::on_h_flip_button_clicked()
{
    QTransform t = QTransform().scale(-1, 1);

    for(LedPosition* led_position : temp_shape->led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x() + temp_shape->w - 1 );
        led_position->setY(new_pos.y());
    }

    ui->grid->UpdateItems();
}

