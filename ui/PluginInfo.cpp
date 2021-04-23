#include "PluginInfo.h"
#include "ui_PluginInfo.h"

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
}

PluginInfo::~PluginInfo()
{
    delete ui;
}
