#include "OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"

#include <QString>
#include <QToolButton>
#include <QLabel>

OpenRGBVisualMapTab::OpenRGBVisualMapTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::OpenRGBVisualMapTab)
{
    ui->setupUi(this);

    ui->virtual_controller_tabs->clear();

    QToolButton *tb = new QToolButton();
    tb->setText("+");

    ui->virtual_controller_tabs->addTab(new QLabel(), QString());
    ui->virtual_controller_tabs->setTabEnabled(0, false);
    ui->virtual_controller_tabs->tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, SIGNAL(clicked()), this, SLOT(AddTab()));

    AddTab();
}

void OpenRGBVisualMapTab::AddTab()
{
    int tab_size = ui->virtual_controller_tabs->count();
    std::string tab_name = "Virtual controller #" + std::to_string(tab_size);

    VirtualControllerTab* tab = new VirtualControllerTab();
    tab->RenameController(tab_name);

    ui->virtual_controller_tabs->insertTab(tab_size - 1, tab , QString::fromUtf8(tab_name.c_str()));
    ui->virtual_controller_tabs->setCurrentIndex(tab_size -1);
}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    delete ui;
}
