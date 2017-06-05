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
#include "StringParser.h"
#include "GlobalString.h"
#include "UiModLabel.h"
#include "AppEnv.h"

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
#include <QImage>
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
    primaryWindow = false;
    setupPictureDialog(true);
}

PictureDialog::PictureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PictureDialog)
{
    primaryWindow = false;
    setupPictureDialog(false);
}

PictureDialog::PictureDialog(bool primaryWindow, ProfileDatabase *profileDB, CrewDatabase *crewDB, QWidget *parent) :
    QDialog(parent), primaryWindow(primaryWindow), profileDB(profileDB), crewDB(crewDB),
    ui(new Ui::PictureDialog)
{
    setupPictureDialog(true);
}

PictureDialog::PictureDialog(bool primaryWindow, QWidget *parent) :
    QDialog(parent), primaryWindow(primaryWindow),
    ui(new Ui::PictureDialog)
{
    setupPictureDialog(false);
}

void PictureDialog::setupPictureDialog(bool withDatabase_)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    // Setup User Interface
    ui->setupUi(this);
    windowTitleStr = this->windowTitle();
    jsonDrawString = ui->labJSON->text();
    ui->cmdExport->setEnabled(0);
    plyrsList = QStringList();
    fullscreenWidget = 0;
    rqFullscreen = 0;
    previewMode = 0;
    naviEnabled = 0;
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

    // With datebase
    withDatabase = withDatabase_;

    // Avatar area
    qreal screenRatio = AppEnv::screenRatio();
    if (screenRatio != 1)
    {
        avatarAreaPicture = QImage(":/img/avatararea.png").scaledToHeight(536 * screenRatio, Qt::FastTransformation);
    }
    else
    {
        avatarAreaPicture = QImage(":/img/avatararea.png");
    }
    avatarLocX = 145;
    avatarLocY = 66;
    avatarSize = 470;

    // Overlay area
    renderOverlayPicture();
    overlayEnabled = 1;

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
    ui->labPicture->setFixedSize(960 * screenRatio, 536 * screenRatio);
    ui->labPicture->setFocusPolicy(Qt::StrongFocus);
}

PictureDialog::~PictureDialog()
{
    delete jpegExportAction;
    delete pgtaExportAction;
    delete exportMenu;
    delete ui;
}

void PictureDialog::closeEvent(QCloseEvent *ev)
{
    Q_UNUSED(ev)
    if (primaryWindow && withDatabase)
    {
        emit endDatabaseThread();
    }
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
    naviEnabled = true;
#endif
#endif
}

void PictureDialog::adaptNewDialogSize(QSize newLabelSize)
{
    Q_UNUSED(newLabelSize)
    int newDialogHeight = ui->labPicture->pixmap()->height();
    newDialogHeight = newDialogHeight + ui->jsonFrame->height();
    if (naviEnabled) newDialogHeight = newDialogHeight + layout()->menuBar()->height();
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
    if (naviEnabled)
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
                if (previewMode)
                {
                    previewMode = false;
                    renderPicture();
                }
                else
                {
                    previewMode = true;
                    renderPicture();
                }
                break;
            case Qt::Key_2:
                if (overlayEnabled)
                {
                    overlayEnabled = false;
                    if (!previewMode) renderPicture();
                }
                else
                {
                    overlayEnabled = true;
                    if (!previewMode) renderPicture();
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
            case Qt::Key_Escape:
                ui->cmdClose->click();
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
    rqFullscreen = fullscreen;
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
    qreal screenRatio = AppEnv::screenRatio();
    QRect preferedRect = QRect(0, 0, 200 * screenRatio, 160 * screenRatio);
    QString overlayText = tr("Key 1 - Avatar Preview Mode\nKey 2 - Toggle Overlay\nArrow Keys - Navigate");
    QImage overlayImage(1, 1, QImage::Format_ARGB32_Premultiplied);
    overlayImage.fill(Qt::transparent);

    QPainter overlayPainter(&overlayImage);
    QFont overlayPainterFont;
    overlayPainterFont.setPixelSize(12 * screenRatio);
    overlayPainter.setFont(overlayPainterFont);
    QRect overlaySpace = overlayPainter.boundingRect(preferedRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextWordWrap, overlayText);
    overlayPainter.end();

    int hOverlay = Qt::AlignTop;
    if (overlaySpace.height() < 74 * screenRatio)
    {
        hOverlay = Qt::AlignVCenter;
        preferedRect.setHeight(71 * screenRatio);
        overlaySpace.setHeight(80 * screenRatio);
    }
    else
    {
        overlaySpace.setHeight(overlaySpace.height() + 6 * screenRatio);
    }

    overlayImage = overlayImage.scaled(overlaySpace.size());
    overlayPainter.begin(&overlayImage);
    overlayPainter.setPen(QColor::fromRgb(255, 255, 255, 255));
    overlayPainter.setFont(overlayPainterFont);
    overlayPainter.drawText(preferedRect, Qt::AlignLeft | hOverlay | Qt::TextDontClip | Qt::TextWordWrap, overlayText);
    overlayPainter.end();

    if (overlaySpace.width() < 194 * screenRatio)
    {
        overlaySpace.setWidth(200 * screenRatio);
    }
    else
    {
        overlaySpace.setWidth(overlaySpace.width() + 6 * screenRatio);
    }

    QImage overlayBorderImage(overlaySpace.width(), overlaySpace.height(), QImage::Format_ARGB6666_Premultiplied);
    overlayBorderImage.fill(QColor(15, 15, 15, 162));

    overlayTempImage = QImage(overlaySpace.width(), overlaySpace.height(), QImage::Format_ARGB6666_Premultiplied);
    overlayTempImage.fill(Qt::transparent);
    QPainter overlayTempPainter(&overlayTempImage);
    overlayTempPainter.drawImage(0, 0, overlayBorderImage);
    overlayTempPainter.drawImage(3 * screenRatio, 3 * screenRatio, overlayImage);
    overlayTempPainter.end();
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
        if (withDatabase)
        {
            crewID = crewDB->getCrewName(picture->getSnapmaticProperties().crewID);
        }
        else
        {
            crewID = QString::number(picture->getSnapmaticProperties().crewID);
        }
        created = picture->getSnapmaticProperties().createdDateTime.toString(Qt::DefaultLocaleShortDate);
        plyrsList = picture->getSnapmaticProperties().playersList;
        picTitl = StringParser::escapeString(picture->getPictureTitle());
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
                QString playerName;
                if (withDatabase)
                {
                    playerName = profileDB->getPlayerName(player.toInt());
                }
                else
                {
                    playerName = player;
                }
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
    qreal screenRatio = AppEnv::screenRatio();
    if (!previewMode)
    {
        if (overlayEnabled)
        {
            QPixmap shownImagePixmap(960 * screenRatio, 536 * screenRatio);
            shownImagePixmap.fill(Qt::transparent);
            QPainter shownImagePainter(&shownImagePixmap);
            if (screenRatio == 1)
            {
                shownImagePainter.drawImage(0, 0, snapmaticPicture);
                shownImagePainter.drawImage(3 * screenRatio, 3 * screenRatio, overlayTempImage);
            }
            else
            {
                shownImagePainter.drawImage(0, 0, snapmaticPicture.scaledToHeight(536 * screenRatio, Qt::SmoothTransformation));
                shownImagePainter.drawImage(3 * screenRatio, 3 * screenRatio, overlayTempImage);
            }
            shownImagePainter.end();
            ui->labPicture->setPixmap(shownImagePixmap);
        }
        else
        {
            if (screenRatio != 1)
            {
                QPixmap shownImagePixmap(960 * screenRatio, 536 * screenRatio);
                shownImagePixmap.fill(Qt::transparent);
                QPainter shownImagePainter(&shownImagePixmap);
                shownImagePainter.drawImage(0, 0, snapmaticPicture.scaledToHeight(536 * screenRatio, Qt::SmoothTransformation));
                shownImagePainter.end();
                ui->labPicture->setPixmap(shownImagePixmap);
            }
            else
            {
                ui->labPicture->setPixmap(QPixmap::fromImage(snapmaticPicture));
            }
        }
    }
    else
    {
        // Generating Avatar Preview
        QPixmap avatarPixmap(960 * screenRatio, 536 * screenRatio);
        QPainter snapPainter(&avatarPixmap);
        QFont snapPainterFont;
        snapPainterFont.setPixelSize(12 * screenRatio);
        if (screenRatio == 1)
        {
            snapPainter.drawImage(0, 0, snapmaticPicture);
        }
        else
        {
            snapPainter.drawImage(0, 0, snapmaticPicture.scaledToHeight(536 * screenRatio, Qt::SmoothTransformation));
        }
        snapPainter.drawImage(0, 0, avatarAreaPicture);
        snapPainter.setPen(QColor::fromRgb(255, 255, 255, 255));
        snapPainter.setFont(snapPainterFont);
        snapPainter.drawText(QRect(3 * screenRatio, 3 * screenRatio, 140 * screenRatio, 60 * screenRatio), Qt::AlignLeft | Qt::TextWordWrap, tr("Avatar Preview Mode\nPress 1 for Default View"));
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
            QString playerName;
            if (withDatabase)
            {
                playerName = profileDB->getPlayerName(player.toInt());
            }
            else
            {
                playerName = player;
            }
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
    if (rqFullscreen && fullscreenWidget)
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
    if (rqFullscreen && fullscreenWidget)
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
        PictureWidget *pictureWidget = new PictureWidget(this); // Work!
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

        fullscreenWidget = 0; // Work!
        delete pictureWidget; // Work!
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
