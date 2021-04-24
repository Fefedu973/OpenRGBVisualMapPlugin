#include "OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"
#include "PluginInfo.h"

#include <QString>
#include <QToolButton>
#include <QLabel>
#include <QInputDialog>

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

    // First tab: plugin info
    QToolButton *dummy_button = new QToolButton();
    dummy_button->setText("");
    ui->virtual_controller_tabs->addTab(new PluginInfo(), QString("Plugin info"));
    ui->virtual_controller_tabs->tabBar()->setTabButton(0, QTabBar::RightSide, dummy_button);
    dummy_button->setFixedWidth(0);
    dummy_button->setFixedHeight(0);
    dummy_button->hide();

    // Second tab : Add button
    QToolButton *tb = new QToolButton();
    tb->setText("+");
    ui->virtual_controller_tabs->addTab(new QLabel(), QString("New map"));
    ui->virtual_controller_tabs->setTabEnabled(1, false);
    ui->virtual_controller_tabs->tabBar()->setTabButton(1, QTabBar::RightSide, tb);

    connect(tb, SIGNAL(clicked()), this, SLOT(AddTab()));

    connect(ui->virtual_controller_tabs, &QTabWidget::tabCloseRequested, [=](int tab_idx){
        QWidget* tab = ui->virtual_controller_tabs->widget(tab_idx);
        ui->virtual_controller_tabs->removeTab(tab_idx);
        delete tab;

        // dont let the last tab beeing able to be the current
        int current = ui->virtual_controller_tabs->currentIndex();
        int tab_count = ui->virtual_controller_tabs->count();

        if(current == tab_count -1)
        {
            ui->virtual_controller_tabs->setCurrentIndex(tab_count - 2);
        }

    });

    connect(ui->virtual_controller_tabs, &QTabWidget::tabBarClicked, [=](int tab_idx){

        int current = ui->virtual_controller_tabs->currentIndex();
        int tab_count = ui->virtual_controller_tabs->count();

        // dont rename 1st and last tabs
        if(tab_idx == 0 || tab_idx == tab_count - 1)
        {
            return;
        }

        if(current == tab_idx)
        {
            VirtualControllerTab* vct = (VirtualControllerTab*) ui->virtual_controller_tabs->widget(tab_idx);

            QString new_name = QInputDialog::getText(
                        nullptr, "Rename controller", "Set the new name",
                        QLineEdit::Normal, QString::fromUtf8(vct->GetControllerName().c_str())).trimmed();

            if(!new_name.isEmpty())
            {
                ui->virtual_controller_tabs->setTabText(tab_idx, new_name);
                vct->RenameController(new_name.toStdString());
            }
        }

    });

    AddTab();
}

void OpenRGBVisualMapTab::AddTab()
{
    int tab_size = ui->virtual_controller_tabs->count();

    // insert just before the add button
    int tab_position = tab_size - 1;

    std::string tab_name = "New map";

    VirtualControllerTab* tab = new VirtualControllerTab();

    tab->RenameController(tab_name);

    ui->virtual_controller_tabs->insertTab(tab_position, tab , QString::fromUtf8(tab_name.c_str()));
    ui->virtual_controller_tabs->setCurrentIndex(tab_position);

    connect(tab, &VirtualControllerTab::ControllerRenamed, [=](std::string name){
        ui->virtual_controller_tabs->setTabText(tab_position, QString::fromUtf8(name.c_str()));
    });

}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    delete ui;
}
