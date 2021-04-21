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

    settings = new GridSettings();

    settings->w = ctrl_zone->settings.custom_shape->w;
    settings->h = ctrl_zone->settings.custom_shape->h;
    settings->show_grid = true;
    settings->show_bounds = true;
    settings->live_preview = false;
    settings->grid_size = 1;
    settings->grid_scale_factor = 10;

    ui->grid->Init(settings);

    // hide until correct impl
    ui->rotate_button->hide();

    ui->identify_button->hide();

    connect(ui->grid, &EditorGrid::ItemSelected, [=](int idx){
        ui->identify_button->setVisible(idx >= 0);

        if(ui->auto_identify->isChecked())
        {
            IdentifySelected();
        }
    });

    if(ctrl_zone->settings.custom_shape == nullptr)
    {
        ResetShape();
    }
    else
    {
        ui->grid->CreateLEDItems(ctrl_zone->settings.custom_shape);
    }

    Update();
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

void WidgetEditor::Update()
{
    ui->led_count->setValue(ctrl_zone->led_count());
    ui->w_spinBox->setValue(ctrl_zone->settings.custom_shape->w);
    ui->h_spinBox->setValue(ctrl_zone->settings.custom_shape->h);

    //ui->grid->UpdateItems();
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
    int led_count = ctrl_zone->led_count();

    ctrl_zone->settings.custom_shape = new CustomShape();
    ctrl_zone->settings.custom_shape->w = ctrl_zone->led_count();
    ctrl_zone->settings.custom_shape->h = ctrl_zone->led_count();

    // really needed ?
    ctrl_zone->settings.custom_shape->led_positions = std::vector<QPoint*>();
    // ---------------

    for(int i = 0; i < led_count; i++)
    {
        ctrl_zone->settings.custom_shape->led_positions.push_back(new QPoint(i, 0));
    }

    ui->grid->CreateLEDItems(ctrl_zone->settings.custom_shape);

    Update();
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

        printf("Exec \n");
        QString selected = inp->textValue();
        ControllerZone* selected_ctrl_zone = ctrl_zones_choices[selected];

        ctrl_zone->settings.custom_shape = new CustomShape();
        ctrl_zone->settings.custom_shape->w = selected_ctrl_zone->settings.custom_shape->w;
        ctrl_zone->settings.custom_shape->h = selected_ctrl_zone->settings.custom_shape->h;

        ctrl_zone->settings.custom_shape->led_positions = std::vector<QPoint*>();

        for(QPoint* point :selected_ctrl_zone->settings.custom_shape->led_positions)
        {
            ctrl_zone->settings.custom_shape->led_positions.push_back(new QPoint(point->x(), point->y()));
        }

        ui->grid->CreateLEDItems(ctrl_zone->settings.custom_shape);

        Update();
    }

}

void WidgetEditor::on_cancel_button_clicked()
{
    emit Cancel();
}

void WidgetEditor::on_save_button_clicked()
{
    emit Save();
}

void WidgetEditor::on_w_spinBox_valueChanged(int value)
{
    settings->w = value;
    ctrl_zone->settings.custom_shape->w = value;
    ui->grid->OnSettingsChanged();
}

void WidgetEditor::on_h_spinBox_valueChanged(int value)
{
    settings->h = value;
    ctrl_zone->settings.custom_shape->h = value;
    ui->grid->OnSettingsChanged();
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

    if(led_num >= 0){
        ZoneManager::Get()->IdentifyLed(ctrl_zone, led_num);
    }
}

void WidgetEditor::on_rotate_button_clicked()
{
    //    int new_width  = ctrl_zone->settings.custom_shape->h;
    //    int new_height = ctrl_zone->settings.custom_shape->w;

    //    QTransform t1 = QTransform().rotate(90);
    //    QTransform t2 = QTransform().translate(new_width, 0);

    //    ctrl_zone->settings.custom_shape->w = new_width;
    //    ctrl_zone->settings.custom_shape->h = new_height;

    //    for(unsigned int i = 0; i < ctrl_zone->settings.custom_shape->led_positions.size(); i++)
    //    {
    //        QPoint* point = ctrl_zone->settings.custom_shape->led_positions[i];
    //        QPoint new_pos = t1.map(*point);
    //        point->setX(new_pos.x() + new_width - 1);
    //        point->setY(new_pos.y());
    //    }

    //    Update();
}

void WidgetEditor::on_v_flip_button_clicked()
{
    QTransform t = QTransform().scale(1,-1);

    for(unsigned int i = 0; i < ctrl_zone->settings.custom_shape->led_positions.size(); i++)
    {
        QPoint* point = ctrl_zone->settings.custom_shape->led_positions[i];
        QPoint new_pos = t.map(*point);
        point->setX(new_pos.x());
        point->setY(new_pos.y() +  ctrl_zone->settings.custom_shape->h - 1);
    }

    ui->grid->UpdateItems();
}

void WidgetEditor::on_h_flip_button_clicked()
{
    QTransform t = QTransform().scale(-1, 1);

    for(unsigned int i = 0; i < ctrl_zone->settings.custom_shape->led_positions.size(); i++)
    {
        QPoint* point = ctrl_zone->settings.custom_shape->led_positions[i];
        QPoint new_pos = t.map(*point);
        point->setX(new_pos.x()  +  ctrl_zone->settings.custom_shape->w - 1 );
        point->setY(new_pos.y());
    }

    ui->grid->UpdateItems();
}

