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
#include "SidebarGenerator.h"
#include "StandardPaths.h"
#include "PictureExport.h"
#include "PictureCopy.h"
#include "UiModLabel.h"

#include <QDesktopWidget>
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
    picPath = "";
    crewID = "";
    locX = "";
    locY = "";
    locZ = "";
    smpic = 0;

    // Export menu
    exportMenu = new QMenu(this);
    exportMenu->addAction(tr("Export as &JPG picture..."), this, SLOT(exportSnapmaticPicture()));
    exportMenu->addAction(tr("Export as &GTA Snapmatic..."), this, SLOT(copySnapmaticPicture()));
    ui->cmdExport->setMenu(exportMenu);
}

PictureDialog::~PictureDialog()
{
    delete exportMenu;
    delete ui;
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, QString picturePath, bool readOk)
{
    // Showing error if reading error
    QImage snapmaticPicture;
    picPath = picturePath;
    smpic = picture;
    if (!readOk)
    {
        QMessageBox::warning(this, tr("Snapmatic Picture Viewer"), tr("Failed at %1").arg(picture->getLastStep()));
        return;
    }

    if (picture->isPicOk())
    {
        snapmaticPicture = picture->getPicture();
        ui->labPicture->setPixmap(QPixmap::fromImage(snapmaticPicture, Qt::AutoColor));
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
                plyrsStr.append(", <a href=\"https://socialclub.rockstargames.com/member/");
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

    int jsn_h = ui->jsonFrame->height();
    int spc_h = layout()->spacing();
    int pix_h = snapmaticPicture.height();
    int frm_h = jsn_h+spc_h+pix_h;

    this->setMinimumSize(960, frm_h);
    this->setMaximumSize(960, frm_h);
}

void PictureDialog::playerNameUpdated()
{
    if (plyrsList.count() >= 1)
    {
        QString plyrsStr;
        foreach (const QString &player, plyrsList)
        {
            QString playerName = profileDB->getPlayerName(player.toInt());
            plyrsStr.append(", <a href=\"https://socialclub.rockstargames.com/member/");
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

void PictureDialog::exportSnapmaticPicture()
{
    PictureExport::exportPicture(this, smpic);
}

void PictureDialog::copySnapmaticPicture()
{
    PictureCopy::copyPicture(this, picPath);
}

void PictureDialog::on_labPicture_mouseDoubleClicked()
{
    QDialog *pictureWidget = new QDialog(this);
    QRect rec = QApplication::desktop()->screenGeometry();
    QHBoxLayout *widgetLayout = new QHBoxLayout(pictureWidget);
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);

    UiModLabel *pictureLabel = new UiModLabel(pictureWidget);
    pictureLabel->setPixmap(ui->labPicture->pixmap()->scaled(rec.width(), rec.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pictureLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pictureLabel->setAlignment(Qt::AlignCenter);
    widgetLayout->addWidget(pictureLabel);

    QObject::connect(pictureLabel, SIGNAL(mouseDoubleClicked()), pictureWidget, SLOT(close()));

    pictureWidget->setLayout(widgetLayout);
    pictureWidget->setWindowFlags(pictureWidget->windowFlags()^Qt::WindowContextHelpButtonHint);
    pictureWidget->setWindowTitle(this->windowTitle());
    pictureWidget->setStyleSheet("background-color: black;");
    pictureWidget->showFullScreen();
    pictureWidget->setModal(true);
    pictureWidget->exec();

    widgetLayout->deleteLater();
    delete widgetLayout;
    pictureLabel->deleteLater();
    delete pictureLabel;
    pictureWidget->deleteLater();
    delete pictureWidget;
}
