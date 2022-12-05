#include "TabHeader.h"
#include "ui_TabHeader.h"
#include "OpenRGBPluginsFont.h"

TabHeader::TabHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabHeader)
{
    ui->setupUi(this);

    ui->close->setFont(OpenRGBPluginsFont::GetFont());
    ui->close->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::close));
}

void TabHeader::Rename(QString name)
{
    ui->editor->setText(name);
}

void TabHeader::on_editor_editingFinished()
{
    emit RenameRequest(ui->editor->text());
}

void TabHeader::on_close_clicked()
{
    emit CloseRequest();
}

TabHeader::~TabHeader()
{
    delete ui;
}
