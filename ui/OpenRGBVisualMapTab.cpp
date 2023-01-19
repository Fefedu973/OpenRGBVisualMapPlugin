#include "OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"
#include "VisualMapSettingsManager.h"
#include "PluginInfo.h"
#include "TabHeader.h"

#include <QString>
#include <QToolButton>
#include <QLabel>
#include <QInputDialog>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QDialog>

OpenRGBVisualMapTab::OpenRGBVisualMapTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::OpenRGBVisualMapTab)
{
    ui->setupUi(this);

    // remove intial dummy tabs
    ui->virtual_controller_tabs->clear();

    // define tab style + settings
    ui->virtual_controller_tabs->setTabsClosable(true);
    ui->virtual_controller_tabs->setStyleSheet("QTabBar::close-button{image:url(:images/close.png);}");
    ui->virtual_controller_tabs->tabBar()->setStyleSheet("QTabBar::tab:hover {text-decoration: underline;}");

    QMenu* main_menu = new QMenu(this);

    QLabel* no_map = new QLabel("You have no visual map.\n You can add one by clicking the VMap button.");
    no_map->setAlignment(Qt::AlignCenter);

    // First tab: add button
    QPushButton* main_menu_button = new QPushButton();
    main_menu_button->setText("VMap");
    ui->virtual_controller_tabs->addTab(no_map, QString(""));
    ui->virtual_controller_tabs->tabBar()->setTabButton(0, QTabBar::RightSide, main_menu_button);
    ui->virtual_controller_tabs->setTabEnabled(0, false);
    main_menu_button->setMenu(main_menu);

    QAction* new_map = new QAction("New map", this);
    connect(new_map, &QAction::triggered, this, &OpenRGBVisualMapTab::AddTabSlot);
    main_menu->addAction(new_map);


    QAction* open_vmap_folder = new QAction("Open VMaps folder", this);
    connect(open_vmap_folder, &QAction::triggered, this, &OpenRGBVisualMapTab::OpenVmapsFolder);
    main_menu->addAction(open_vmap_folder);

    QAction* about = new QAction("About", this);
    connect(about, &QAction::triggered, this, &OpenRGBVisualMapTab::AboutSlot);
    main_menu->addAction(about);

    if(!SearchAndAutoLoad())
    {
         AddTab();
    }
}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    delete ui;
}

void OpenRGBVisualMapTab::UnregisterAll()
{
    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->Unregister();
    }
}

void OpenRGBVisualMapTab::Backup()
{
    printf("[OpenRGBVisualMapPlugin] Backup\n");

    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->BackupZones();
    }

    printf("[OpenRGBVisualMapPlugin] Backup done\n");
}

void OpenRGBVisualMapTab::Clear()
{    
    printf("[OpenRGBVisualMapPlugin] Clear\n");

    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->Clear();
    }

    printf("[OpenRGBVisualMapPlugin] Clear done\n");
}

void OpenRGBVisualMapTab::Recreate()
{
    printf("[OpenRGBVisualMapPlugin] Recreate\n");

    for(VirtualControllerTab* controller_tab: controller_tabs)
    {
        controller_tab->Recreate();
    }

    printf("[OpenRGBVisualMapPlugin] Recreate done\n");
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

    std::vector<std::string> filenames = VisualMapSettingsManager::GetMapNames();

    for(std::string filename : filenames)
    {
        try
        {
            json j = VisualMapSettingsManager::LoadMap(filename);

            bool auto_load = j["grid_settings"]["auto_load"];

            if(auto_load)
            {
                printf("[OpenRGBVisualMapPlugin] Auto load: loading file %s\n", filename.c_str());
                VirtualControllerTab* tab = AddTab();
                tab->LoadFile(filename);
                has_loaded = true;
            }
        }
        catch(const std::exception& e)
        {
            printf("[OpenRGBVisualMapPlugin] Not able to load file %s: \n%s\n", filename.c_str(), e.what());
        }
    }

    return has_loaded;
}

void OpenRGBVisualMapTab::AddTabSlot()
{
    AddTab();
}

void OpenRGBVisualMapTab::AboutSlot()
{
    QDialog* dialog = new QDialog();
    dialog->setWindowTitle("Visual Map");
    dialog->setMinimumSize(300,320);
    dialog->setModal(true);

    QVBoxLayout* dialog_layout = new QVBoxLayout(dialog);

    PluginInfo* plugin_info = new PluginInfo(dialog);

    dialog_layout->addWidget(plugin_info);

    dialog->exec();
}

void OpenRGBVisualMapTab::OpenVmapsFolder()
{
    filesystem::path config_dir = VisualMapSettingsManager::MapsFolder();
    QUrl url = QUrl::fromLocalFile(QString::fromStdString(config_dir.string()));

    printf("[OpenRGBEffectsPlugin] Opening %s\n", url.path().toStdString().c_str());

    QDesktopServices::openUrl(url);
}
