/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016-2018 Syping
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "SavegameDialog.h"
#include "ui_SavegameDialog.h"
#include "SavegameCopy.h"
#include "AppEnv.h"
#include <QMessageBox>
#include <QDebug>

SavegameDialog::SavegameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SavegameDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    // Setup User Interface
    ui->setupUi(this);
    ui->cmdClose->setFocus();
    savegameLabStr = ui->labSavegameText->text();

    // Set Icon for Close Button
    if (QIcon::hasThemeIcon("dialog-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("gtk-close"));
    }

    // Set Icon for Export Button
    if (QIcon::hasThemeIcon("document-export"))
    {
        ui->cmdCopy->setIcon(QIcon::fromTheme("document-export"));
    }
    else if (QIcon::hasThemeIcon("document-save"))
    {
        ui->cmdCopy->setIcon(QIcon::fromTheme("document-save"));
    }

    refreshWindowSize();
}

SavegameDialog::~SavegameDialog()
{
    delete ui;
}

void SavegameDialog::refreshWindowSize()
{
    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    int dpiWindowWidth = 400 * screenRatio;
    int dpiWindowHeight = 105 * screenRatio;
    if (dpiWindowHeight < heightForWidth(dpiWindowWidth))
    {
        dpiWindowHeight = heightForWidth(dpiWindowWidth);
    }
    resize(dpiWindowWidth, dpiWindowHeight);
}

void SavegameDialog::setSavegameData(SavegameData *savegame, QString savegamePath, bool readOk)
{
    // Showing error if reading error
    if (!readOk)
    {
        QMessageBox::warning(this,tr("Savegame Viewer"),tr("Failed at %1").arg(savegame->getLastStep()));
        return;
    }
    sgdPath = savegamePath;
    ui->labSavegameText->setText(savegameLabStr.arg(savegame->getSavegameStr()));
    refreshWindowSize();
}

void SavegameDialog::on_cmdClose_clicked()
{
    this->close();
}

void SavegameDialog::on_cmdCopy_clicked()
{
    SavegameCopy::copySavegame(this, sgdPath);
}
