#include "OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"

#include <QString>
#include <QToolButton>
#include <QLabel>
#include <QInputDialog>

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

    ui->virtual_controller_tabs->setTabsClosable(true);
    ui->virtual_controller_tabs->setStyleSheet("QTabBar::close-button{image:url(:close.png);}");
    ui->virtual_controller_tabs->tabBar()->setStyleSheet("QTabBar::tab:hover {text-decoration: underline;}");

    connect(tb, SIGNAL(clicked()), this, SLOT(AddTab()));

    connect(ui->virtual_controller_tabs, &QTabWidget::tabBarDoubleClicked, [=](int tab_index){
        // real tab size (do not count the empty tab with the "+" button
        int tab_size = ui->virtual_controller_tabs->count() - 1;
        if(tab_index < tab_size)
        {
            VirtualControllerTab* vct = (VirtualControllerTab*) ui->virtual_controller_tabs->widget(tab_index);

            QString new_name = QInputDialog::getText(
                        nullptr, "Rename controller", "Set the new name",
                        QLineEdit::Normal, QString::fromUtf8(vct->GetControllerName().c_str())).trimmed();

            if(!new_name.isEmpty())
            {
                ui->virtual_controller_tabs->setTabText(tab_index, new_name);
                vct->RenameController(new_name.toStdString());
            }

        }
    });

    connect(ui->virtual_controller_tabs, &QTabWidget::tabCloseRequested, [=](int tab_idx){
        QWidget* tab = ui->virtual_controller_tabs->widget(tab_idx);
        ui->virtual_controller_tabs->removeTab(tab_idx);
        delete tab;
    });

    AddTab();
}

void OpenRGBVisualMapTab::AddTab()
{
    int tab_size = ui->virtual_controller_tabs->count();
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
