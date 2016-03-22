/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
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

#include "PictureDialog.h"
#include "ProfileDatabase.h"
#include "ui_PictureDialog.h"

#include <QJsonDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>

PictureDialog::PictureDialog(ProfileDatabase *profileDB, QWidget *parent) :
    QDialog(parent), profileDB(profileDB),
    ui(new Ui::PictureDialog)
{
    ui->setupUi(this);
    windowTitleStr = this->windowTitle();
    jsonDrawString = ui->labJSON->text();
    ui->cmdExport->setEnabled(0);
    ui->cmdExport->setDefault(0);
    ui->cmdClose->setDefault(1);
    ui->cmdClose->setFocus();
}

PictureDialog::~PictureDialog()
{
    delete ui;
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk)
{
    // Showing error if reading error
    if (!readOk)
    {
        QMessageBox::warning(this,tr("Snapmatic Picture Viewer"),tr("Failed at %1").arg(picture->getLastStep())); return;
    }

    if (picture->isPicOk())
    {
        ui->labPicture->setPixmap(picture->getPixmap());
        ui->cmdExport->setEnabled(true);
    }
    if (picture->isJsonOk())
    {
        QString locX = QString::number(picture->getLocationX());
        QString locY = QString::number(picture->getLocationY());
        QString locZ = QString::number(picture->getLocationZ());
        QString crewID = QString::number(picture->getCrewNumber());
        QStringList plyrsList = picture->getPlayers();

        QString plyrsStr;
        if (plyrsList.length() >= 1)
        {
            foreach (const QString &player, plyrsList)
            {
                plyrsStr.append(", ");
                plyrsStr.append(profileDB->getPlayerName(player.toInt()));
            }
            plyrsStr.remove(0,2);
        }
        else
        {
            plyrsStr = tr("No player");
        }

        if (crewID == "") { crewID = tr("No crew"); }

        this->setWindowTitle(windowTitleStr.arg(picture->getPictureStr()));
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID));
    }
    else
    {
        ui->labJSON->setText(jsonDrawString.arg("0.0", "0.0", "0.0", tr("No player"), tr("No crew")));
        QMessageBox::warning(this,tr("Snapmatic Picture Viewer"),tr("Failed at %1").arg(picture->getLastStep()));
    }
}

void PictureDialog::on_cmdClose_clicked()
{
    this->close();
}

void PictureDialog::on_cmdExport_clicked()
{

}
