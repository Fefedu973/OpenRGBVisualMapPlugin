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

    return dialog->exec();
}

WidgetEditor::WidgetEditor(QWidget *parent, ControllerZone* ctrl_zone):
    QWidget(parent),
    ui(new Ui::WidgetEditor),
    ctrl_zone(ctrl_zone)
{
    ui->setupUi(this);

    InitShape();

    settings = new GridSettings();

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;
    settings->show_grid = true;
    settings->show_bounds = true;
    settings->live_preview = false;
    settings->grid_size = 1;

    ui->grid->ApplySettings(settings);

    ui->identify_button->hide();

    connect(ui->grid, &EditorGrid::SelectionChanged, [=](){
        ui->identify_button->setVisible(!ui->grid->GetSelection().empty());

        if(ui->auto_identify->isChecked())
        {
            IdentifySelected();
        }
    });

    connect(ui->grid, &EditorGrid::Changed, [=](){
        SaveState();
    });

    ui->grid->CreateLEDItems(temp_shape);

    ui->undo_button->setEnabled(false);

    UpdateWidgetsValues();
}

WidgetEditor::~WidgetEditor()
{
    delete settings;
    delete ui;
}

void WidgetEditor::InitShape()
{
    // if custom shape already exists, copy it to temp shape
    // else, generate one (horizontal line)

    if(ctrl_zone->settings.custom_shape)
    {
        temp_shape = ctrl_zone->settings.custom_shape->clone();
    }
    else
    {
        temp_shape = CustomShape::HorizontalLine(ctrl_zone->led_count());
    }

    states.push_back(temp_shape->clone());
}

bool WidgetEditor::StateChanged()
{
    return temp_shape->differs(states.back());
}

void WidgetEditor::SaveState()
{
    if(StateChanged())
    {
        states.push_back(temp_shape->clone());
        ui->undo_button->setEnabled(true);
    }
}

void WidgetEditor::Undo()
{
    if(states.size() > 1)
    {
        RestoreState(states[states.size() -2]);

        if(states.size() > 1)
        {
            states.pop_back();
        }

        ui->undo_button->setEnabled(states.size() > 1);
    }
}

void WidgetEditor::RestoreState(CustomShape* shape)
{    
    temp_shape = shape->clone();

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    UpdateWidgetsValues();

    ui->grid->ApplySettings(settings);
    ui->grid->CreateLEDItems(temp_shape);
}

void WidgetEditor::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Z && event->modifiers() == Qt::CTRL)
    {
        Undo();
    }

    QWidget::keyPressEvent(event);
}

void WidgetEditor::UpdateWidgetsValues()
{   
    ui->led_count->blockSignals(true);
    ui->w_spinBox->blockSignals(true);
    ui->h_spinBox->blockSignals(true);

    ui->led_count->setValue(ctrl_zone->led_count());
    ui->w_spinBox->setValue(temp_shape->w);
    ui->h_spinBox->setValue(temp_shape->h);

    ui->led_count->blockSignals(false);
    ui->w_spinBox->blockSignals(false);
    ui->h_spinBox->blockSignals(false);
}

void WidgetEditor::IdentifySelected()
{
    std::vector<LedPosition*> selection = ui->grid->GetSelection();

    std::vector<unsigned int> selected_led_nums;

    for(LedPosition* led_position: selection)
    {
        selected_led_nums.push_back(led_position->led_num);
    }

    if(!selected_led_nums.empty())
    {
        ZoneManager::Get()->IdentifyLeds(ctrl_zone, selected_led_nums);
    }
}

void WidgetEditor::on_identify_button_clicked()
{
    IdentifySelected();
}

void WidgetEditor::on_reset_button_clicked()
{
    temp_shape = states[0]->clone();

    states.clear();

    states.push_back(temp_shape->clone());

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    UpdateWidgetsValues();

    ui->grid->ApplySettings(settings);
    ui->grid->CreateLEDItems(temp_shape);

    ui->undo_button->setEnabled(false);
}

void WidgetEditor::on_copy_shape_button_clicked()
{
    std::vector<ControllerZone*> ctrl_zones = ZoneManager::Get()->GetAvailableZones();

    QStringList items;

    std::map<QString, ControllerZone*> ctrl_zones_choices;

    // generate choice list
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

        temp_shape = selected_ctrl_zone->settings.custom_shape->clone();

        ui->grid->CreateLEDItems(temp_shape);

        SaveState();

        UpdateWidgetsValues();
    }

}

void WidgetEditor::on_cancel_button_clicked()
{
    emit Cancel();
}

void WidgetEditor::on_save_button_clicked()
{
    ctrl_zone->settings.custom_shape = temp_shape->clone();
    emit Save();
}

void WidgetEditor::on_w_spinBox_valueChanged(int value)
{
    settings->w = value;
    temp_shape->w = value;

    SaveState();

    ui->grid->ApplySettings(settings);
}

void WidgetEditor::on_h_spinBox_valueChanged(int value)
{
    settings->h = value;
    temp_shape->h = value;

    SaveState();

    ui->grid->ApplySettings(settings);    
}

void WidgetEditor::on_auto_identify_stateChanged(int state)
{
    if(state)
    {
        IdentifySelected();
    }
}

void WidgetEditor::on_rotate_button_clicked()
{
    // Resize (swap w and h)
    int new_width  = temp_shape->h;
    int new_height = temp_shape->w;

    temp_shape->w = new_width;
    temp_shape->h = new_height;

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    ui->grid->ApplySettings(settings);

    UpdateWidgetsValues();

    QTransform t = QTransform().rotate(90);

    for(LedPosition* led_position : temp_shape->led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x() + new_width - 1);
        led_position->setY(new_pos.y());
    }

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
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

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
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

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
}

void WidgetEditor::on_h_line_button_clicked()
{
    temp_shape->w = 0;
    temp_shape->h = 1;

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    ui->grid->ApplySettings(settings);

    for(LedPosition* led_position: temp_shape->led_positions)
    {
       led_position->setX(temp_shape->w++);
       led_position->setY(0);
    }

    UpdateWidgetsValues();

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
}

void WidgetEditor::on_v_line_button_clicked()
{
    temp_shape->w = 1;
    temp_shape->h = 0;

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    ui->grid->ApplySettings(settings);

    for(LedPosition* led_position: temp_shape->led_positions)
    {
       led_position->setX(0);
       led_position->setY(temp_shape->h++);
    }

    UpdateWidgetsValues();

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
}

void WidgetEditor::on_grow_button_clicked()
{
    temp_shape->w *= 2;
    temp_shape->h *= 2;

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    ui->grid->ApplySettings(settings);

    UpdateWidgetsValues();

    QTransform t = QTransform().scale(2, 2);

    for(LedPosition* led_position : temp_shape->led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x());
        led_position->setY(new_pos.y());
    }

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
}

void WidgetEditor::on_shrink_button_clicked()
{
    temp_shape->w *= 0.5;
    temp_shape->h *= 0.5;

    temp_shape->w = std::max<int>(1,temp_shape->w);
    temp_shape->h = std::max<int>(1,temp_shape->h);

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    ui->grid->ApplySettings(settings);
    UpdateWidgetsValues();

    QTransform t = QTransform().scale(0.5, 0.5);

    for(LedPosition* led_position : temp_shape->led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x());
        led_position->setY(new_pos.y());
    }

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
}

void WidgetEditor::on_circle_button_clicked()
{
    double PI = 3.14159265359l;

    unsigned int leds_count = temp_shape->led_positions.size();

    int radius = leds_count / 2;

    temp_shape->w = 2 * radius + 1;
    temp_shape->h = 2 * radius + 1;

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    ui->grid->ApplySettings(settings);

    UpdateWidgetsValues();

    for(unsigned int i  = 0 ; i < leds_count; i++)
    {
        float theta = ((PI*2) / leds_count);
        float angle = (theta * i);

        int x = round(radius + radius * cos(angle));
        int y = round(radius + radius * sin(angle));

        temp_shape->led_positions[i]->setX(x);
        temp_shape->led_positions[i]->setY(y);
    }

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
}

void WidgetEditor::on_square_button_clicked()
{
    int side = temp_shape->led_positions.size() / 4 ;

    temp_shape->w = side + 2;
    temp_shape->h = side + 2;

    settings->w = temp_shape->w;
    settings->h = temp_shape->h;

    ui->grid->ApplySettings(settings);

    UpdateWidgetsValues();

    for(int i = 0; i < side; i++)
    {
        temp_shape->led_positions[i]->setX(1 + i);
        temp_shape->led_positions[i]->setY(0);
    }
    for(int i = 0; i < side; i++)
    {
        temp_shape->led_positions[side + i]->setX(side + 1);
        temp_shape->led_positions[side + i]->setY(i + 1);
    }

    for(int i = 0; i < side; i++)
    {
        temp_shape->led_positions[2 * side + i]->setX(side - i);
        temp_shape->led_positions[2 * side + i]->setY(side + 1);
    }

    for(int i = 0; i < side; i++)
    {
        temp_shape->led_positions[3 * side + i]->setX(0);
        temp_shape->led_positions[3 * side + i]->setY(side - i);
    }

    int offset = side * 4;

    int rest = temp_shape->led_positions.size() - side * 4;

    for(int i = 0; i < rest; i++)
    {
        temp_shape->led_positions[offset+i]->setX(i+1);
        temp_shape->led_positions[offset+i]->setY(i+1);
    }

    ui->grid->CreateLEDItems(temp_shape);

    SaveState();
}

void WidgetEditor::on_undo_button_clicked()
{
    Undo();
}






