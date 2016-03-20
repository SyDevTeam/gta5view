#include "PictureDialog.h"
#include "ui_PictureDialog.h"

PictureDialog::PictureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PictureDialog)
{
    ui->setupUi(this);
}

PictureDialog::~PictureDialog()
{
    delete ui;
}
