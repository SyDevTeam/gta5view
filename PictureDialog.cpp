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
#include "StandardPaths.h"

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

PictureDialog::PictureDialog(ProfileDatabase *profileDB, QWidget *parent) :
    QDialog(parent), profileDB(profileDB),
    ui(new Ui::PictureDialog)
{
    ui->setupUi(this);
    windowTitleStr = this->windowTitle();
    jsonDrawString = ui->labJSON->text();
    ui->cmdExport->setEnabled(0);
    plyrsList = QStringList();
    picTitl = "";
    crewID = "";
    locX = "";
    locY = "";
    locZ = "";
    smpic = 0;
}

PictureDialog::~PictureDialog()
{
    delete ui;
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk)
{
    // Showing error if reading error
    smpic = picture;
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
        picTitl = picture->getPictureTitl();

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
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID, picTitl));
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
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID, picTitl));
    }
}

void PictureDialog::on_cmdClose_clicked()
{
    this->close();
}

void PictureDialog::on_cmdExport_clicked()
{
    QSettings settings("Syping", "gta5sync");
    settings.beginGroup("FileDialogs");

fileDialogPreSave:
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix("suffix");
    fileDialog.setWindowTitle(tr("Export picture"));
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);

    QStringList filters;
    filters << tr("JPEG picture (*.jpg)");
    filters << tr("Portable Network Graphics (*.png)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = fileDialog.sidebarUrls();
    QDir dir;

    // Get Documents + Desktop Location
    QString documentsLocation = StandardPaths::documentsLocation();
    QString desktopLocation = StandardPaths::desktopLocation();

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
    fileDialog.restoreState(settings.value("ExportPicture","").toByteArray());

    if (smpic != 0)
    {
        QString newPictureFileName;
        QString pictureStr = smpic->getPictureStr();
        QStringList pictureStrList = pictureStr.split(" - ");
        if (pictureStrList.length() <= 2)
        {
            QString dtStr = pictureStrList.at(1);
            QStringList dtStrList = dtStr.split(" ");
            if (dtStrList.length() <= 2)
            {
                QString dayStr;
                QString yearStr;
                QString monthStr;
                QString dateStr = dtStrList.at(0);
                QString timeStr = dtStrList.at(1);
                timeStr.replace(":","");
                QStringList dateStrList = dateStr.split("/");
                if (dateStrList.length() <= 3)
                {
                    dayStr = dateStrList.at(1);
                    yearStr = dateStrList.at(2);
                    monthStr = dateStrList.at(0);
                }
                newPictureFileName = yearStr + "-" + monthStr + "-" + dayStr + "_" + timeStr + ".jpg";
            }
        }
        fileDialog.selectFile(newPictureFileName);
    }

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

            if (QFile::exists(selectedFile))
            {
                if (QMessageBox::Yes == QMessageBox::warning(this, tr("Export picture"), tr("Overwrite %1 with current Snapmatic picture?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                {
                    if (!QFile::remove(selectedFile))
                    {
                        QMessageBox::warning(this, tr("Export picture"), tr("Failed to overwrite %1 with current Snapmatic picture").arg("\""+selectedFile+"\""));
                        goto fileDialogPreSave;
                    }
                }
                else
                {
                    goto fileDialogPreSave;
                }
            }

            bool isSaved = ui->labPicture->pixmap()->save(selectedFile, saveFileFormat.toStdString().c_str(), 100);

            if (!isSaved)
            {
                QMessageBox::warning(this, tr("Export picture"), tr("Failed to save current picture"));
                goto fileDialogPreSave;
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Export picture"), tr("No valid file is selected"));
            goto fileDialogPreSave;
        }
    }

    settings.setValue("ExportPicture", fileDialog.saveState());
    settings.endGroup();
}
