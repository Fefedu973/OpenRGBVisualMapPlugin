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

int WidgetEditor::Show(ControllerZone* ctrl_zone, std::vector<ControllerZone*> other_zones)
{
    WidgetEditor* editor = new WidgetEditor(nullptr, ctrl_zone);
    editor->other_zones = other_zones;

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

    if(ctrl_zone->isCustomShape() && ctrl_zone->settings.custom_shape)
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
    QStringList items;

    std::map<QString, ControllerZone*> ctrl_zones_choices;

    // generate choice list
    int i = 0;

    for(ControllerZone* ctrl_zone_it : other_zones)
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

        AutoResize();

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
    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    if(led_positions.empty())
    {
       return;
    }

    QPoint start_position = led_positions.front()->point;

    QTransform t = QTransform()
            .translate(start_position.x(), start_position.y())
            .rotate(90)
            .translate(-start_position.x(), -start_position.y());

    for(LedPosition* led_position : led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x());
        led_position->setY(new_pos.y());
    }

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_v_flip_button_clicked()
{
    QTransform t = QTransform().scale(1,-1);

    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    if(led_positions.empty())
    {
       return;
    }

    QPoint start_position = led_positions.front()->point;

    for(LedPosition* led_position : led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x());
        led_position->setY(new_pos.y() + start_position.y() - 1);
    }

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_h_flip_button_clicked()
{
    QTransform t = QTransform().scale(-1, 1);

    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    if(led_positions.empty())
    {
       return;
    }

    QPoint start_position = led_positions.front()->point;

    for(LedPosition* led_position : led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x() +  start_position.x() - 1 );
        led_position->setY(new_pos.y());
    }

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_h_line_button_clicked()
{
    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    if(led_positions.empty())
    {
       return;
    }

    QPoint start_position = led_positions.front()->point;

    unsigned int i = 0;

    for(LedPosition* led_position : led_positions)
    {
       led_position->setX(start_position.x() + (i++));
       led_position->setY(start_position.y());
    }

    UpdateWidgetsValues();

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_v_line_button_clicked()
{
    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    if(led_positions.empty())
    {
       return;
    }

    QPoint start_position = led_positions.front()->point;

    unsigned int i = 0;

    for(LedPosition* led_position : led_positions)
    {
       led_position->setX(start_position.x());
       led_position->setY(start_position.y() + (i++));
    }

    UpdateWidgetsValues();

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_grow_button_clicked()
{
    QTransform t = QTransform().scale(2, 2);

    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    for(LedPosition* led_position : led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x());
        led_position->setY(new_pos.y());
    }

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_shrink_button_clicked()
{
    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    QTransform t = QTransform().scale(0.5, 0.5);

    for(LedPosition* led_position : led_positions)
    {
        QPoint new_pos = t.map(led_position->point);
        led_position->setX(new_pos.x());
        led_position->setY(new_pos.y());
    }

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_circle_button_clicked()
{
    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;
    QPoint start_position = led_positions.front()->point;

    double PI = 3.14159265359l;

    unsigned int leds_count = led_positions.size();

    int radius = leds_count / 2;

    for(unsigned int i = 0; i< led_positions.size(); i++)
    {
        float theta = ((PI*2) / leds_count);
        float angle = (theta * i);

        int x = round(radius * cos(angle));
        int y = round(radius * sin(angle));

        led_positions[i]->setX(x);
        led_positions[i]->setY(y);

        led_positions[i]->shift(start_position.x() + radius, start_position.y() + radius);
    }

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_square_button_clicked()
{
    std::vector<LedPosition*> selection = ui->grid->GetSelection();
    std::vector<LedPosition*> led_positions = selection.empty() ? temp_shape->led_positions : selection;

    int side = led_positions.size() / 4 ;

    QPoint start_position = led_positions.front()->point;

    for(int i = 0; i < side; i++)
    {
        led_positions[i]->setX(1 + i);
        led_positions[i]->setY(0);
    }
    for(int i = 0; i < side; i++)
    {
        led_positions[side + i]->setX(side + 1);
        led_positions[side + i]->setY(i + 1);
    }

    for(int i = 0; i < side; i++)
    {
        led_positions[2 * side + i]->setX(side - i);
        led_positions[2 * side + i]->setY(side + 1);
    }

    for(int i = 0; i < side; i++)
    {
        led_positions[3 * side + i]->setX(0);
        led_positions[3 * side + i]->setY(side - i);
    }

    int offset = side * 4;

    int rest = led_positions.size() - side * 4;

    for(int i = 0; i < rest; i++)
    {
        led_positions[offset+i]->setX(i+1);
        led_positions[offset+i]->setY(i+1);
    }

    for(LedPosition* led_position : led_positions)
    {
        led_position->shift(start_position.x(), start_position.y());
    }

    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::on_undo_button_clicked()
{
    Undo();
}

void WidgetEditor::on_auto_resize_button_clicked()
{
    AutoResize();

    ui->grid->UpdateItems();

    SaveState();
}

void WidgetEditor::AutoResize()
{
    // 1st shift everything if theres some leds in negative place / or empty row/cols
    int shift_x = 1024;
    int shift_y = 1024;

    for(LedPosition* led_position : temp_shape->led_positions)
    {
       shift_x = std::min<int>(shift_x, led_position->x());
       shift_y = std::min<int>(shift_y, led_position->y());
    }

    for(LedPosition* led_position : temp_shape->led_positions)
    {
        led_position->shift(-shift_x, -shift_y);
    }

    //2nd fit the size
    unsigned int w = 1;
    unsigned int h = 1;

    for(LedPosition* led_position : temp_shape->led_positions)
    {
       w = std::max<int>(w, led_position->x() + 1 );
       h = std::max<int>(h, led_position->y() + 1 );
    }

    temp_shape->w = w;
    temp_shape->h = h;

    settings->w = w;
    settings->h = h;

    ui->grid->ApplySettings(settings);

    UpdateWidgetsValues();
}

QPoint WidgetEditor::GetCenter(std::vector<LedPosition*> led_positions)
{
    QRect bounds(0,0,1,1);

    for(LedPosition* led_position: led_positions)
    {
        bounds.setX(std::min<unsigned int>(bounds.x(), led_position->x()));
        bounds.setWidth(std::max<unsigned int>(bounds.width(), led_position->x() + 1));

        bounds.setY(std::min<unsigned int>(bounds.y(), led_position->y()));
        bounds.setHeight(std::max<unsigned int>(bounds.height(), led_position->y() + 1));
    }

    return bounds.center();
}


