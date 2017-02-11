/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016-2017 Syping
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
#include "UiModLabel.h"

#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
#include <QtWinExtras/QtWin>
#include <QtWinExtras/QWinEvent>
#endif
#endif

#include <QDesktopWidget>
#include <QJsonDocument>
#include <QApplication>
#include <QStaticText>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMimeData>
#include <QToolBar>
#include <QPainter>
#include <QPicture>
#include <QBitmap>
#include <QBuffer>
#include <QDebug>
#include <QList>
#include <QDrag>
#include <QIcon>
#include <QUrl>
#include <QDir>

PictureDialog::PictureDialog(ProfileDatabase *profileDB, CrewDatabase *crewDB, QWidget *parent) :
    QDialog(parent), profileDB(profileDB), crewDB(crewDB),
    ui(new Ui::PictureDialog)
{
    ui->setupUi(this);
    windowTitleStr = this->windowTitle();
    jsonDrawString = ui->labJSON->text();
    ui->cmdExport->setEnabled(0);
    plyrsList = QStringList();
    fullscreenWidget = 0;
    rqfullscreen = 0;
    previewmode = 0;
    navienabled = 0;
    indexed = 0;
    picArea = "";
    picTitl = "";
    picPath = "";
    created = "";
    crewID = "";
    locX = "";
    locY = "";
    locZ = "";
    smpic = 0;

    // Avatar area
    avatarAreaPicture = QImage(":/img/avatararea.png");
    avatarLocX = 145;
    avatarLocY = 66;
    avatarSize = 470;

    // Overlay area
    renderOverlayPicture();
    overlayenabled = 1;

    // Export menu
    exportMenu = new QMenu(this);
    jpegExportAction = exportMenu->addAction(tr("Export as &JPG picture..."), this, SLOT(exportSnapmaticPicture()));
    pgtaExportAction = exportMenu->addAction(tr("Export as &GTA Snapmatic..."), this, SLOT(copySnapmaticPicture()));
    ui->cmdExport->setMenu(exportMenu);

    // Global map
    globalMap = GlobalString::getGlobalMap();

    // Event connects
    connect(ui->labJSON, SIGNAL(resized(QSize)), this, SLOT(adaptNewDialogSize(QSize)));

    // Dialog buttons
    if (QIcon::hasThemeIcon("dialog-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }

    installEventFilter(this);
    installEventFilter(ui->labPicture);
    ui->labPicture->setFocusPolicy(Qt::StrongFocus);
}

PictureDialog::~PictureDialog()
{
    delete jpegExportAction;
    delete pgtaExportAction;
    delete exportMenu;
    delete ui;
}

void PictureDialog::addPreviousNextButtons()
{
    // Windows Vista additions
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
    QPalette palette;
    QToolBar *uiToolbar = new QToolBar("Picture Toolbar", this);
    layout()->setMenuBar(uiToolbar);
    uiToolbar->addAction(QIcon(":/img/back.png"), "", this, SLOT(previousPictureRequestedSlot()));
    uiToolbar->addAction(QIcon(":/img/next.png"), "", this, SLOT(nextPictureRequestedSlot()));
    ui->jsonFrame->setStyleSheet(QString("QFrame { background: %1; }").arg(palette.window().color().name()));
    navienabled = true;
#endif
#endif
}

void PictureDialog::adaptNewDialogSize(QSize newLabelSize)
{
    Q_UNUSED(newLabelSize)
    int newDialogHeight = ui->labPicture->pixmap()->height();
    newDialogHeight = newDialogHeight + ui->jsonFrame->height();
    if (navienabled) newDialogHeight = newDialogHeight + layout()->menuBar()->height();
    setMinimumSize(width(), newDialogHeight);
    setMaximumSize(width(), newDialogHeight);
    setFixedHeight(newDialogHeight);
    ui->labPicture->updateGeometry();
    ui->jsonFrame->updateGeometry();
    updateGeometry();
}

void PictureDialog::stylizeDialog()
{
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
    if (QtWin::isCompositionEnabled())
    {
        QtWin::extendFrameIntoClientArea(this, 0, this->layout()->menuBar()->height(), 0, 0);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, false);
        setStyleSheet("PictureDialog { background: transparent; }");
    }
    else
    {
        QtWin::resetExtendedFrame(this);
        setAttribute(Qt::WA_TranslucentBackground, false);
        setStyleSheet(QString("PictureDialog { background: %1; }").arg(QtWin::realColorizationColor().name()));
    }
#endif
#endif
}

bool PictureDialog::event(QEvent *event)
{
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
    if (navienabled)
    {
        if (event->type() == QWinEvent::CompositionChange || event->type() == QWinEvent::ColorizationChange)
        {
            stylizeDialog();
        }
    }
#endif
#endif
    return QDialog::event(event);
}

void PictureDialog::nextPictureRequestedSlot()
{
    emit nextPictureRequested();
}

void PictureDialog::previousPictureRequestedSlot()
{
    emit previousPictureRequested();
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
            case Qt::Key_1:
                if (previewmode)
                {
                    previewmode = false;
                    renderPicture();
                }
                else
                {
                    previewmode = true;
                    renderPicture();
                }
                break;
            case Qt::Key_2:
                if (overlayenabled)
                {
                    overlayenabled = false;
                    if (!previewmode) renderPicture();
                }
                else
                {
                    overlayenabled = true;
                    if (!previewmode) renderPicture();
                }
                break;
#if QT_VERSION >= 0x050300
            case Qt::Key_Exit:
                ui->cmdClose->click();
                returnValue = true;
                break;
#endif
            case Qt::Key_Enter: case Qt::Key_Return:
                on_labPicture_mouseDoubleClicked(Qt::LeftButton);
                returnValue = true;
                break;
            }
        }
    }
    return returnValue;
}

void PictureDialog::triggerFullscreenDoubeClick()
{
    on_labPicture_mouseDoubleClicked(Qt::LeftButton);
}

void PictureDialog::exportCustomContextMenuRequestedPrivate(const QPoint &pos, bool fullscreen)
{
    rqfullscreen = fullscreen;
    exportMenu->popup(pos);
}

void PictureDialog::exportCustomContextMenuRequested(const QPoint &pos)
{
    exportCustomContextMenuRequestedPrivate(pos, true);
}

void PictureDialog::mousePressEvent(QMouseEvent *ev)
{
    QDialog::mousePressEvent(ev);
}

void PictureDialog::dialogNextPictureRequested()
{
    emit nextPictureRequested();
}

void PictureDialog::dialogPreviousPictureRequested()
{
    emit previousPictureRequested();
}

void PictureDialog::renderOverlayPicture()
{
    // Generating Overlay Preview
    QRect preferedRect = QRect(0, 0, 200, 160);
    QString overlayText = tr("Key 1 - Avatar Preview Mode\nKey 2 - Toggle Overlay\nArrow Keys - Navigate");
    QPixmap overlayPixmap(1, 1);
    overlayPixmap.fill(Qt::transparent);

    QPainter overlayPainter(&overlayPixmap);
    QFont overlayPainterFont;
    overlayPainterFont.setPixelSize(12);
    overlayPainter.setFont(overlayPainterFont);
    QRect overlaySpace = overlayPainter.boundingRect(preferedRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextWordWrap, overlayText);
    overlayPainter.end();

    int hOverlay = Qt::AlignTop;
    if (overlaySpace.height() < 74)
    {
        hOverlay = Qt::AlignVCenter;
        preferedRect.setHeight(71);
        overlaySpace.setHeight(80);
    }
    else
    {
        overlaySpace.setHeight(overlaySpace.height() + 6);
    }

    overlayPixmap = overlayPixmap.scaled(overlaySpace.size());
    overlayPainter.begin(&overlayPixmap);
    overlayPainter.setPen(QColor::fromRgb(255, 255, 255, 255));
    overlayPainter.setFont(overlayPainterFont);
    overlayPainter.drawText(preferedRect, Qt::AlignLeft | hOverlay | Qt::TextDontClip | Qt::TextWordWrap, overlayText);
    overlayPainter.end();

    if (overlaySpace.width() < 194)
    {
        overlaySpace.setWidth(200);
    }
    else
    {
        overlaySpace.setWidth(overlaySpace.width() + 6);
    }

    QPixmap overlayBorderImage(overlaySpace.width(), overlaySpace.height());
    overlayBorderImage.fill(QColor(15, 15, 15, 162));

    QPixmap overlayTempPixmap(overlaySpace.size());
    overlayTempPixmap.fill(Qt::transparent);
    QPainter overlayTempPainter(&overlayTempPixmap);
    overlayTempPainter.drawPixmap(0, 0, overlayBorderImage);
    overlayTempPainter.drawPixmap(3, 3, overlayPixmap);
    overlayTempPainter.end();
    overlayTempImage = overlayTempPixmap.toImage();
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk, bool _indexed, int _index)
{
    snapmaticPicture = QImage();
    indexed = _indexed;
    index = _index;
    picPath = picture->getPictureFilePath();
    smpic = picture;
    if (!readOk)
    {
        QMessageBox::warning(this, tr("Snapmatic Picture Viewer"), tr("Failed at %1").arg(picture->getLastStep()));
        return;
    }
    if (picture->isPicOk())
    {
        snapmaticPicture = picture->getImage();
        renderPicture();
        ui->cmdExport->setEnabled(true);
    }
    if (picture->isJsonOk())
    {
        locX = QString::number(picture->getSnapmaticProperties().location.x);
        locY = QString::number(picture->getSnapmaticProperties().location.y);
        locZ = QString::number(picture->getSnapmaticProperties().location.z);
        crewID = crewDB->getCrewName(picture->getSnapmaticProperties().crewID);
        created = picture->getSnapmaticProperties().createdDateTime.toString(Qt::DefaultLocaleShortDate);
        plyrsList = picture->getSnapmaticProperties().playersList;
        picTitl = picture->getPictureTitl();
        picArea = picture->getSnapmaticProperties().location.area;
        if (globalMap.contains(picArea))
        {
            picAreaStr = globalMap[picArea];
        }
        else
        {
            picAreaStr = picArea;
        }

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
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID, picTitl, picAreaStr, created));
    }
    else
    {
        ui->labJSON->setText(jsonDrawString.arg("0.0", "0.0", "0.0", tr("No player"), tr("No crew"), tr("Unknown Location")));
        QMessageBox::warning(this,tr("Snapmatic Picture Viewer"),tr("Failed at %1").arg(picture->getLastStep()));
    }
    emit newPictureCommited(snapmaticPicture);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk, int index)
{
    setSnapmaticPicture(picture, readOk, true, index);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk)
{
    setSnapmaticPicture(picture, readOk, false, 0);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, int index)
{
    setSnapmaticPicture(picture, true, index);
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture)
{
    setSnapmaticPicture(picture, true);
}

void PictureDialog::renderPicture()
{
    if (!previewmode)
    {
        if (overlayenabled)
        {
            QPixmap overlayAreaPixmap(960, 536);
            overlayAreaPixmap.fill(Qt::transparent);
            QPainter overlayAreaPainter(&overlayAreaPixmap);
            overlayAreaPainter.drawImage(0, 0, snapmaticPicture);
            overlayAreaPainter.drawImage(3, 3, overlayTempImage);
            overlayAreaPainter.end();
            ui->labPicture->setPixmap(overlayAreaPixmap);
        }
        else
        {
            ui->labPicture->setPixmap(QPixmap::fromImage(snapmaticPicture));
        }
    }
    else
    {
        // Generating Avatar Preview
        QPixmap avatarPixmap(960, 536);
        QPainter snapPainter(&avatarPixmap);
        QFont snapPainterFont;
        snapPainterFont.setPixelSize(12);
        snapPainter.drawImage(0, 0, snapmaticPicture);
        snapPainter.drawImage(0, 0, avatarAreaPicture);
        snapPainter.setPen(QColor::fromRgb(255, 255, 255, 255));
        snapPainter.setFont(snapPainterFont);
        snapPainter.drawText(QRect(3, 3, 140, 60), Qt::AlignLeft | Qt::TextWordWrap, tr("Avatar Preview Mode\nPress 1 for Default View"));
        snapPainter.end();
        ui->labPicture->setPixmap(avatarPixmap);
    }
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
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, plyrsStr, crewID, picTitl, picAreaStr, created));
    }
}

void PictureDialog::exportSnapmaticPicture()
{
    if (rqfullscreen && fullscreenWidget)
    {
        PictureExport::exportAsPicture(fullscreenWidget, smpic);
    }
    else
    {
        PictureExport::exportAsPicture(this, smpic);
    }
}

void PictureDialog::copySnapmaticPicture()
{
    if (rqfullscreen && fullscreenWidget)
    {
        PictureExport::exportAsSnapmatic(fullscreenWidget, smpic);
    }
    else
    {
        PictureExport::exportAsSnapmatic(this, smpic);
    }
}

void PictureDialog::on_labPicture_mouseDoubleClicked(Qt::MouseButton button)
{
    if (button == Qt::LeftButton)
    {
        QRect desktopRect = QApplication::desktop()->screenGeometry(this);
        PictureWidget *pictureWidget = new PictureWidget(this);
        pictureWidget->setObjectName("PictureWidget");
#if QT_VERSION >= 0x050600
        pictureWidget->setWindowFlags(pictureWidget->windowFlags()^Qt::FramelessWindowHint^Qt::WindowStaysOnTopHint^Qt::MaximizeUsingFullscreenGeometryHint);
#else
        pictureWidget->setWindowFlags(pictureWidget->windowFlags()^Qt::FramelessWindowHint^Qt::WindowStaysOnTopHint);
#endif
        pictureWidget->setWindowTitle(this->windowTitle());
        pictureWidget->setStyleSheet("QLabel#pictureLabel{background-color: black;}");
        pictureWidget->setImage(snapmaticPicture, desktopRect);
        pictureWidget->setModal(true);

        fullscreenWidget = pictureWidget;
        QObject::connect(this, SIGNAL(newPictureCommited(QImage)), pictureWidget, SLOT(setImage(QImage)));
        QObject::connect(pictureWidget, SIGNAL(nextPictureRequested()), this, SLOT(dialogNextPictureRequested()));
        QObject::connect(pictureWidget, SIGNAL(previousPictureRequested()), this, SLOT(dialogPreviousPictureRequested()));

        pictureWidget->move(desktopRect.x(), desktopRect.y());
        pictureWidget->resize(desktopRect.width(), desktopRect.height());
        pictureWidget->showFullScreen();
        pictureWidget->setFocus();
        pictureWidget->raise();
        pictureWidget->exec();

        fullscreenWidget = 0;
        delete pictureWidget;
    }
}

void PictureDialog::on_labPicture_customContextMenuRequested(const QPoint &pos)
{
    exportCustomContextMenuRequestedPrivate(ui->labPicture->mapToGlobal(pos), false);
}

bool PictureDialog::isIndexed()
{
    return indexed;
}

int PictureDialog::getIndex()
{
    return index;
}
