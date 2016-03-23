#include "SavegameDialog.h"
#include "ui_SavegameDialog.h"

#include <QMessageBox>

SavegameDialog::SavegameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SavegameDialog)
{
    ui->setupUi(this);
    savegameLabStr = ui->labSavegameText->text();
}

SavegameDialog::~SavegameDialog()
{
    delete ui;
}

void SavegameDialog::setSavegameData(SavegameData *savegame, bool readOk)
{
    // Showing error if reading error
    if (!readOk)
    {
        QMessageBox::warning(this,tr("Savegame Viewer"),tr("Failed at %1").arg(savegame->getLastStep()));
        return;
    }

    ui->labSavegameText->setText(savegameLabStr.arg(savegame->getSavegameStr()));
}

void SavegameDialog::on_cmdClose_clicked()
{
    this->close();
}
