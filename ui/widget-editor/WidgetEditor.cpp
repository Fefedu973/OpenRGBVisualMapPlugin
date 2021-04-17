#include "WidgetEditor.h"
#include "ui_WidgetEditor.h"

#include "OpenRGBVisualMapPlugin.h"
#include "ZoneManager.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QFile>
#include <QPoint>

WidgetEditor::WidgetEditor(QWidget *parent, ControllerZone* ctrl_zone):
    QWidget(parent),
    ui(new Ui::WidgetEditor),
    ctrl_zone(ctrl_zone)
{
    int led_count = ctrl_zone->led_count();

    if(ctrl_zone->settings.custom_shape == nullptr)
    {
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
    }

    ui->setupUi(this);

    settings = new GridSettings();

    settings->w = ctrl_zone->settings.custom_shape->w;
    settings->h = ctrl_zone->settings.custom_shape->h;
    settings->show_grid = true;
    settings->show_bounds = true;
    settings->grid_size = 1;
    settings->grid_scale_factor = 10;

    ui->grid->Init(settings);

    ui->identify_button->hide();

    connect(ui->grid, &EditorGrid::ItemSelected, [=](int idx){
        ui->identify_button->setVisible(idx >= 0);

        if(ui->auto_identify->isChecked())
        {
            IdentifySelected();
        }
    });

    ui->grid->CreateLEDItems(ctrl_zone->settings.custom_shape);

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

    dialog->setWindowTitle("Widget editor");
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
}

void WidgetEditor::on_identify_button_clicked()
{
    IdentifySelected();
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
