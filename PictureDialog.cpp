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
#include "PictureWidget.h"
#include "ProfileDatabase.h"
#include "ui_PictureDialog.h"
#include "SidebarGenerator.h"
#include "StandardPaths.h"
#include "PictureExport.h"
#include "GlobalString.h"
#include "PictureCopy.h"
#include "UiModLabel.h"

#include <QDesktopWidget>
#include <QJsonDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMimeData>
#include <QBuffer>
#include <QDebug>
#include <QList>
#include <QDrag>
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
    indexed = 0;
    picArea = "";
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

    installEventFilter(this);
    installEventFilter(ui->labPicture);
    ui->labPicture->setFocusPolicy(Qt::StrongFocus);
}

PictureDialog::~PictureDialog()
{
    delete exportMenu;
    delete ui;
}

bool PictureDialog::eventFilter(QObject *obj, QEvent *ev)
{
    bool returnValue = false;
    if (obj == this || obj == ui->labPicture)
    {
        if (ev->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = (QKeyEvent*)ev;
            switch (keyEvent->key()){
            case Qt::Key_Left:
                emit previousPictureRequested();
                returnValue = true;
                break;
            case Qt::Key_Right:
                emit nextPictureRequested();
                returnValue = true;
                break;
            case Qt::Key_E: case Qt::Key_S: case Qt::Key_Save:
                ui->cmdExport->click();
                returnValue = true;
                break;
#if QT_VERSION >= 0x050000
            case Qt::Key_Exit:
                ui->cmdClose->click();
                returnValue = true;
                break;
#endif
            case Qt::Key_Enter: case Qt::Key_Return:
                on_labPicture_mouseDoubleClicked();
                returnValue = true;
                break;
            }
        }
    }
    return returnValue;
}

void PictureDialog::mousePressEvent(QMouseEvent *ev)
{
    ev->accept();
}

void PictureDialog::dialogNextPictureRequested()
{
    emit nextPictureRequested();
}

void PictureDialog::dialogPreviousPictureRequested()
{
    emit previousPictureRequested();
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, QString picturePath, bool readOk, bool _indexed, int _index)
{
    snapmaticPicture = QImage();
    indexed = _indexed;
    index = _index;
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
        picArea = picture->getArea();
        picAreaStr = GlobalString::getString(picArea);

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
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID, picTitl, picAreaStr));
    }
    else
    {
        ui->labJSON->setText(jsonDrawString.arg("0.0", "0.0", "0.0", tr("No player"), tr("No crew"), tr("Unknown Location")));
        QMessageBox::warning(this,tr("Snapmatic Picture Viewer"),tr("Failed at %1").arg(picture->getLastStep()));
    }
    emit newPictureCommited(snapmaticPicture);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, QString picPath, bool readOk)
{
    setSnapmaticPicture(picture, picPath, readOk, false, 0);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, QString picPath)
{
    setSnapmaticPicture(picture, picPath, true);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk, int index)
{
    setSnapmaticPicture(picture, picture->getPictureFileName(), readOk, true, index);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk)
{
    setSnapmaticPicture(picture, picture->getPictureFileName(), readOk);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, int index)
{
    setSnapmaticPicture(picture, true, index);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture)
{
    setSnapmaticPicture(picture, true);
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
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID, picTitl, picAreaStr));
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
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    PictureWidget *pictureWidget = new PictureWidget(this);
    pictureWidget->setWindowFlags(pictureWidget->windowFlags()^Qt::WindowContextHelpButtonHint);
    pictureWidget->setWindowTitle(this->windowTitle());
    pictureWidget->setStyleSheet("background-color: black;");
    pictureWidget->setImage(snapmaticPicture, desktopRect);
    pictureWidget->setModal(true);

    QObject::connect(this, SIGNAL(newPictureCommited(QImage)), pictureWidget, SLOT(setImage(QImage)));
    QObject::connect(pictureWidget, SIGNAL(nextPictureRequested()), this, SLOT(dialogNextPictureRequested()));
    QObject::connect(pictureWidget, SIGNAL(previousPictureRequested()), this, SLOT(dialogPreviousPictureRequested()));

    pictureWidget->showFullScreen();
    pictureWidget->setFocus();
    pictureWidget->exec();

    delete pictureWidget;
}

bool PictureDialog::isIndexed()
{
    return indexed;
}

int PictureDialog::getIndex()
{
    return index;
}
