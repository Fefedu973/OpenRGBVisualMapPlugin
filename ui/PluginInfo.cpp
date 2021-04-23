#include "PluginInfo.h"
#include "OpenRGBVisualMapPlugin.h"
#include "ui_PluginInfo.h"

#include <QDesktopServices>

PluginInfo::PluginInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PluginInfo)
{
    ui->setupUi(this);

    ui->builddate_string->setText(BUILDDATE_STRING);
    ui->version_string->setText(VERSION_STRING);
    ui->git_commit_id->setText(GIT_COMMIT_ID);
    ui->git_commit_date->setText(GIT_COMMIT_DATE);
    ui->git_branch->setText(GIT_BRANCH);
    //QDesktopServices::openUrl()

}

PluginInfo::~PluginInfo()
{
    delete ui;
}

void PluginInfo::on_open_plugin_folder_clicked()
{
    std::string config_dir = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + "/plugins";
    QUrl url(QString::fromStdString(config_dir));
    QDesktopServices::openUrl(url);
}

void PluginInfo::on_download_latest_clicked()
{
    QDesktopServices::openUrl(QString::fromStdString(LATEST_BUILD_URL));
}
