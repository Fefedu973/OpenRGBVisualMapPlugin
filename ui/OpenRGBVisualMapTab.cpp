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

    connect(tab, &VirtualControllerTab::ControllerRenamed, [=](std::string name){
        ui->virtual_controller_tabs->setTabText(tab_size - 1, QString::fromUtf8(name.c_str()));
    });    

    QPushButton * close_button = new QPushButton ("close");

    connect(close_button, &QPushButton::clicked, [=](){
        // todo debug this
        ui->virtual_controller_tabs->removeTab(tab_size - 1);
        delete tab;
    });

    ui->virtual_controller_tabs->tabBar()->setTabButton(tab_size - 1, QTabBar::LeftSide, close_button);

}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    delete ui;
}
