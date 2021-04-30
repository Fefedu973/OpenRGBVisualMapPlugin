#include "TabHeader.h"
#include "ui_TabHeader.h"

TabHeader::TabHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabHeader)
{
    ui->setupUi(this);

    connect(ui->editor, &EditableLabel::Focussed, [=](bool has_focus){
        if(has_focus)
        {
            emit SelectRequest();
        }
    });
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

