#include "OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"
#include "VisualMapSettingsManager.h"
#include "PluginInfo.h"
#include "TabHeader.h"
#include "OpenRGBVisualMapPlugin.h"

#include <QString>
#include <QToolButton>
#include <QLabel>
#include <QInputDialog>
#include <QTimer>

OpenRGBVisualMapTab::OpenRGBVisualMapTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::OpenRGBVisualMapTab)
{
    ui->setupUi(this);

    // remove intial dummy tabs
    ui->virtual_controller_tabs->clear();

    // define tab style + settings
    ui->virtual_controller_tabs->setTabsClosable(true);
    ui->virtual_controller_tabs->setStyleSheet("QTabBar::close-button{image:url(:close.png);}");
    ui->virtual_controller_tabs->tabBar()->setStyleSheet("QTabBar::tab:hover {text-decoration: underline;}");

    // First tab: add button
    QToolButton *new_map_button = new QToolButton();
    new_map_button->setText("New map");
    ui->virtual_controller_tabs->addTab(new PluginInfo(), QString(""));
    ui->virtual_controller_tabs->tabBar()->setTabButton(0, QTabBar::RightSide, new_map_button);
    ui->virtual_controller_tabs->setTabEnabled(0, false);


    // 2md tab: plugin info
    QToolButton *dummy_button = new QToolButton();
    dummy_button->setText("");
    ui->virtual_controller_tabs->addTab(new PluginInfo(), QString("Plugin info"));
    ui->virtual_controller_tabs->tabBar()->setTabButton(1, QTabBar::RightSide, dummy_button);
    dummy_button->setFixedWidth(0);
    dummy_button->setFixedHeight(0);
    dummy_button->hide();

    connect(new_map_button, SIGNAL(clicked()), this, SLOT(AddTabSlot()));

    if(!SearchAndAutoLoad())
    {
         AddTab();
    }

}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    delete ui;
}

void OpenRGBVisualMapTab::Clear()
{
    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->Clear();
    }
}

void OpenRGBVisualMapTab::DeviceListChanged()
{
    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->DeviceListChanged();
    }
}

VirtualControllerTab* OpenRGBVisualMapTab::AddTab()
{
    int tab_size = ui->virtual_controller_tabs->count();

    // insert at the end
    int tab_position = tab_size;

    std::string tab_name = "Untitled";

    VirtualControllerTab* tab = new VirtualControllerTab();
    TabHeader* tab_header = new TabHeader();
    tab_header->Rename(QString::fromUtf8(tab_name.c_str()));

    tab->RenameController(tab_name);

    ui->virtual_controller_tabs->insertTab(tab_position, tab , "");
    ui->virtual_controller_tabs->tabBar()->setTabButton(tab_position, QTabBar::RightSide, tab_header);

    ui->virtual_controller_tabs->setCurrentIndex(tab_position);

    connect(tab, &VirtualControllerTab::ControllerRenamed, [=](std::string new_name){
        tab_header->Rename(QString::fromUtf8(new_name.c_str()));
    });

    connect(tab_header, &TabHeader::RenameRequest, [=](QString new_name){
        tab->RenameController(new_name.toStdString());
    });

    connect(tab_header, &TabHeader::CloseRequest, [=](){
        int tab_idx = ui->virtual_controller_tabs->indexOf(tab);

        ui->virtual_controller_tabs->removeTab(tab_idx);

        controller_tabs.erase(std::find(controller_tabs.begin(), controller_tabs.end(), tab));

        delete tab;
        delete tab_header;       
    });

    ui->virtual_controller_tabs->update();

    controller_tabs.push_back(tab);

    return tab;
}


bool OpenRGBVisualMapTab::SearchAndAutoLoad()
{
    bool has_loaded = false;

    std::vector<std::string> filenames = VisualMapSettingsManager::GetFileNames();
    for(std::string filename : filenames)
    {
        try
        {
            json j = VisualMapSettingsManager::LoadSettings(filename);

            bool auto_load = j["grid_settings"]["auto_load"];

            if(auto_load)
            {
                printf("Auto load: loading file %s\n", filename.c_str());
                VirtualControllerTab* tab = AddTab();
                tab->LoadFile(filename);
                has_loaded = true;
            }
        }
        catch(const std::exception& e)
        {
            printf("Not able to load file %s: \n%s\n", filename.c_str(), e.what());
        }
    }

    return has_loaded;
}

void OpenRGBVisualMapTab::AddTabSlot()
{
    AddTab();
}
