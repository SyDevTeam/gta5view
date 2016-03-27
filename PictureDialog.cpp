/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping
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
#include <QDebug>
#include <QList>
#include <QUrl>
#include <QDir>

#if QT5_MODE
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

PictureDialog::PictureDialog(ProfileDatabase *profileDB, QWidget *parent) :
    QDialog(parent), profileDB(profileDB),
    ui(new Ui::PictureDialog)
{
    ui->setupUi(this);
    windowTitleStr = this->windowTitle();
    jsonDrawString = ui->labJSON->text();
    ui->cmdExport->setEnabled(0);
    plyrsList = QStringList();
    crewID = "";
    locX = "";
    locY = "";
    locZ = "";
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
        QMessageBox::warning(this, tr("Snapmatic Picture Viewer"), tr("Failed at %1").arg(picture->getLastStep()));
        return;
    }

    if (picture->isPicOk())
    {
        ui->labPicture->setPixmap(QPixmap::fromImage(picture->getPicture(), Qt::AutoColor));
        ui->cmdExport->setEnabled(true);
    }
    if (picture->isJsonOk())
    {
        locX = QString::number(picture->getLocationX());
        locY = QString::number(picture->getLocationY());
        locZ = QString::number(picture->getLocationZ());
        crewID = QString::number(picture->getCrewNumber());
        plyrsList = picture->getPlayers();

        QString plyrsStr;
        if (plyrsList.length() >= 1)
        {
            foreach (const QString &player, plyrsList)
            {
                QString playerName = profileDB->getPlayerName(player.toInt());
                plyrsStr.append(", <a href=\"http://socialclub.rockstargames.com/member/");
                plyrsStr.append(playerName);
                plyrsStr.append("/");
                plyrsStr.append(player);
                plyrsStr.append("\">");
                plyrsStr.append(playerName);
                plyrsStr.append("</a>");
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

    this->setMinimumSize(this->geometry().size());
    this->setMaximumSize(this->geometry().size());
}

void PictureDialog::on_playerNameUpdated()
{
    if (plyrsList.count() >= 1)
    {
        QString plyrsStr;
        foreach (const QString &player, plyrsList)
        {
            QString playerName = profileDB->getPlayerName(player.toInt());
            plyrsStr.append(", <a href=\"http://socialclub.rockstargames.com/member/");
            if (playerName != player)
            {
                plyrsStr.append(playerName);
            }
            else
            {
                plyrsStr.append("id");
            }
            plyrsStr.append("/");
            plyrsStr.append(player);
            plyrsStr.append("\">");
            plyrsStr.append(playerName);
            plyrsStr.append("</a>");
        }
        plyrsStr.remove(0,2);
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID));
    }
}

void PictureDialog::on_cmdClose_clicked()
{
    this->close();
}

void PictureDialog::on_cmdExport_clicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDefaultSuffix("suffix");
    fileDialog.setNameFilter(tr("JPEG picture (*.jpg);;Portable Network Graphics (*.png)"));
    fileDialog.setWindowTitle(tr("Export picture"));
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);

    QList<QUrl> sidebarUrls = fileDialog.sidebarUrls();
    QDir dir;

    // Get Documents + Desktop Location
#ifdef QT5_MODE
    QString documentsLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString desktopLocation = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
#else
    QString documentsLocation = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString desktopLocation = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
#endif

    // Add Desktop Location to Sidebar
    dir.setPath(desktopLocation);
    if (dir.exists())
    {
        sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
    }

    // Add Documents + GTA V Location to Sidebar
    dir.setPath(documentsLocation);
    if (dir.exists())
    {
        sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
        if (dir.cd("Rockstar Games/GTA V"))
        {
            sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
        }
    }

    fileDialog.setSidebarUrls(sidebarUrls);

fileDialogPreSave:
    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString saveFileFormat;
            QString selectedFile = selectedFiles.at(0);

            if (selectedFile.right(4) == ".jpg")
            {
                saveFileFormat = "JPEG";
            }
            else if (selectedFile.right(4) == ".jpeg")
            {
                saveFileFormat = "JPEG";
            }
            else if (selectedFile.right(4) == ".png")
            {
                saveFileFormat = "PNG";
            }
            else if (selectedFile.right(7) == ".suffix")
            {
                if (fileDialog.selectedNameFilter() == "JPEG picture (*.jpg)")
                {
                    selectedFile.replace(".suffix", ".jpg");
                }
                else if (fileDialog.selectedNameFilter() == "Portable Network Graphics (*.png)")
                {
                    selectedFile.replace(".suffix", ".png");
                }
                else
                {
                    selectedFile.replace(".suffix", ".jpg");
                }
            }

            bool isSaved = ui->labPicture->pixmap()->save(selectedFile, saveFileFormat.toStdString().c_str(), 100);

            if (!isSaved)
            {
                QMessageBox::warning(this, tr("Snapmatic Picture Exporter"), tr("Failed to save the picture"));
                goto fileDialogPreSave;
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Snapmatic Picture Exporter"), tr("No valid file is selected"));
            goto fileDialogPreSave;
        }
    }
}
