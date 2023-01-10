/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2021 Syping
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
#include "MapLocationDialog.h"
#include "JsonEditorDialog.h"
#include "SnapmaticEditor.h"
#include "StandardPaths.h"
#include "PictureExport.h"
#include "ImportDialog.h"
#include "StringParser.h"
#include "GlobalString.h"
#include "UiModLabel.h"
#include "AppEnv.h"
#include "config.h"

#if QT_VERSION < 0x060000
#include <QDesktopWidget>
#endif

#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050000
#include "dwmapi.h"
#endif
#endif

#ifdef Q_OS_MAC
#include <QStyleFactory>
#endif

#include <QStringBuilder>
#include <QJsonDocument>
#include <QApplication>
#include <QFontMetrics>
#include <QJsonObject>
#include <QSizePolicy>
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
#include <QTimer>
#include <QImage>
#include <QDebug>
#include <QList>
#include <QDrag>
#include <QIcon>
#include <QUrl>
#include <QDir>

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#endif

// Macros for better Overview + RAM
#define locX QString::number(picture->getSnapmaticProperties().location.x)
#define locY QString::number(picture->getSnapmaticProperties().location.y)
#define locZ QString::number(picture->getSnapmaticProperties().location.z)
#define crewID QString::number(picture->getSnapmaticProperties().crewID)
#define picArea picture->getSnapmaticProperties().location.area
#define picPath picture->getPictureFilePath()
#define picTitl StringParser::escapeString(picture->getPictureTitle())
#define plyrsList picture->getSnapmaticProperties().playersList
#if QT_VERSION >= 0x060000
#define created QLocale().toString(picture->getSnapmaticProperties().createdDateTime, QLocale::ShortFormat)
#else
#define created picture->getSnapmaticProperties().createdDateTime.toString(Qt::DefaultLocaleShortDate)
#endif

PictureDialog::PictureDialog(ProfileDatabase *profileDB, CrewDatabase *crewDB, QWidget *parent) :
    QDialog(parent), profileDB(profileDB), crewDB(crewDB),
    ui(new Ui::PictureDialog)
{
    primaryWindow = false;
    setupPictureDialog();
}

PictureDialog::PictureDialog(ProfileDatabase *profileDB, CrewDatabase *crewDB, QString profileName, QWidget *parent) :
    QDialog(parent), profileDB(profileDB), crewDB(crewDB), profileName(profileName),
    ui(new Ui::PictureDialog)
{
    primaryWindow = false;
    setupPictureDialog();
}

PictureDialog::PictureDialog(bool primaryWindow, ProfileDatabase *profileDB, CrewDatabase *crewDB, QWidget *parent) :
    QDialog(parent), primaryWindow(primaryWindow), profileDB(profileDB), crewDB(crewDB),
    ui(new Ui::PictureDialog)
{
    setupPictureDialog();
}

PictureDialog::PictureDialog(bool primaryWindow, ProfileDatabase *profileDB, CrewDatabase *crewDB, QString profileName, QWidget *parent) :
    QDialog(parent), primaryWindow(primaryWindow), profileDB(profileDB), crewDB(crewDB), profileName(profileName),
    ui(new Ui::PictureDialog)
{
    setupPictureDialog();
}

void PictureDialog::setupPictureDialog()
{
    // Set Window Flags
#if QT_VERSION >= 0x050900
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowFlag(Qt::CustomizeWindowHint, true);
#else
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint^Qt::CustomizeWindowHint);
#endif
#ifdef Q_OS_LINUX
    // for stupid Window Manager like GNOME
#if QT_VERSION >= 0x050900
    setWindowFlag(Qt::Dialog, false);
    setWindowFlag(Qt::Window, true);
#else
    setWindowFlags(windowFlags()^Qt::Dialog^Qt::Window);
#endif
#endif

    // Setup User Interface
    ui->setupUi(this);
    windowTitleStr = this->windowTitle();
    jsonDrawString = ui->labJSON->text();
    ui->cmdManage->setEnabled(false);
    fullscreenWidget = nullptr;
    rqFullscreen = false;
    previewMode = false;
    naviEnabled = false;
    indexed = false;
    smpic = nullptr;
    crewStr = "";

    // Get Snapmatic Resolution
    const QSize snapmaticResolution = SnapmaticPicture::getSnapmaticResolution();

    // Avatar area
    qreal screenRatio = AppEnv::screenRatio();
    qreal screenRatioPR = AppEnv::screenRatioPR();
    if (screenRatio != 1 || screenRatioPR != 1) {
        avatarAreaPicture = QImage(AppEnv::getImagesFolder() % "/avatararea.png").scaledToHeight(snapmaticResolution.height() * screenRatio * screenRatioPR, Qt::FastTransformation);
    }
    else {
        avatarAreaPicture = QImage(AppEnv::getImagesFolder() % "/avatararea.png");
    }
    avatarLocX = 145;
    avatarLocY = 66;
    avatarSize = 470;

    // DPI calculation (picture)
    ui->labPicture->setFixedSize(snapmaticResolution.width() * screenRatio, snapmaticResolution.height() * screenRatio);
    ui->labPicture->setFocusPolicy(Qt::StrongFocus);
    ui->labPicture->setScaledContents(true);

    // Overlay area
    renderOverlayPicture();
    overlayEnabled = true;

    // Manage menu
    manageMenu = new QMenu(this);
    manageMenu->addAction(tr("Export as &Picture..."), this, SLOT(exportSnapmaticPicture()));
    manageMenu->addAction(tr("Export as &Snapmatic..."), this, SLOT(copySnapmaticPicture()));
    manageMenu->addSeparator();
    manageMenu->addAction(tr("&Edit Properties..."), this, SLOT(editSnapmaticProperties()));
    manageMenu->addAction(tr("&Overwrite Image..."), this, SLOT(editSnapmaticImage()));
    manageMenu->addSeparator();
    QAction *openViewerAction = manageMenu->addAction(tr("Open &Map Viewer..."), this, SLOT(openPreviewMap()));
    openViewerAction->setShortcut(Qt::Key_M);
    manageMenu->addAction(tr("Open &JSON Editor..."), this, SLOT(editSnapmaticRawJson()));
    ui->cmdManage->setMenu(manageMenu);

    // Global map
    globalMap = GlobalString::getGlobalMap();

    // Set Icon for Close Button
    if (QIcon::hasThemeIcon("dialog-close")) {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close")) {
        ui->cmdClose->setIcon(QIcon::fromTheme("gtk-close"));
    }

    installEventFilter(this);

    // DPI calculation
    ui->hlButtons->setSpacing(6 * screenRatio);
    ui->vlButtons->setSpacing(6 * screenRatio);
    ui->vlButtons->setContentsMargins(0, 0, 5 * screenRatio, 5 * screenRatio);
    ui->jsonLayout->setContentsMargins(4 * screenRatio, 10 * screenRatio, 4 * screenRatio, 4 * screenRatio);

    // Pre-adapt window for DPI
    const QSize windowSize(snapmaticResolution.width() * screenRatio, snapmaticResolution.height() * screenRatio);
    setMinimumSize(windowSize);
    setMaximumSize(windowSize);
}

PictureDialog::~PictureDialog()
{
    delete ui;
}

void PictureDialog::closeEvent(QCloseEvent *ev)
{
    Q_UNUSED(ev)
    if (primaryWindow)
        emit endDatabaseThread();
}

void PictureDialog::addPreviousNextButtons()
{
    QToolBar *uiToolbar = new QToolBar("Picture Toolbar", this);
#if QT_VERSION < 0x050600
    qreal screenRatio = AppEnv::screenRatio();
    if (screenRatio != 1) {
        QSize iconSize = uiToolbar->iconSize();
        uiToolbar->setIconSize(QSize(iconSize.width() * screenRatio, iconSize.height() * screenRatio));
    }
#endif
    uiToolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    uiToolbar->setObjectName("UiToolbar");
    uiToolbar->addAction(QIcon(AppEnv::getImagesFolder() % "/back.svgz"), "", this, SLOT(previousPictureRequestedSlot()));
    uiToolbar->addAction(QIcon(AppEnv::getImagesFolder() % "/next.svgz"), "", this, SLOT(nextPictureRequestedSlot()));
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x050000
    uiToolbar->setStyle(QStyleFactory::create("Fusion"));
#endif
#endif
    layout()->setMenuBar(uiToolbar);
    naviEnabled = true;
}

void PictureDialog::adaptDialogSize()
{
    int newDialogHeight = (SnapmaticPicture::getSnapmaticResolution().height() * AppEnv::screenRatio()) + ui->jsonFrame->heightForWidth(width());
    if (naviEnabled)
        newDialogHeight = newDialogHeight + layout()->menuBar()->height();
    const QSize windowSize(width(), newDialogHeight);
    setMinimumSize(windowSize);
    setMaximumSize(windowSize);
}

void PictureDialog::styliseDialog()
{
#ifdef Q_OS_WIN
    BOOL isEnabled;
    DwmIsCompositionEnabled(&isEnabled);
    if (isEnabled == TRUE) {
        MARGINS margins = {0, 0, qRound(layout()->menuBar()->height() * AppEnv::screenRatioPR()), 0};
        HRESULT hr = S_OK;
        hr = DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(winId()), &margins);
        if (SUCCEEDED(hr)) {
            setStyleSheet("PictureDialog{background:transparent}");
        }
    }
    else {
        MARGINS margins = {0, 0, 0, 0};
        DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(winId()), &margins);
        bool colorOk = false;
        QSettings dwmRegistry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM", QSettings::NativeFormat);
        QRgb color = dwmRegistry.value("ColorizationColor").toUInt(&colorOk);
        if (colorOk) {
            setStyleSheet(QString("PictureDialog{background:%1}").arg(QColor::fromRgba(color).name()));
        }
        else {
            HRESULT hr = S_OK;
            BOOL isOpaqueBlend;
            DWORD colorization;
            hr = DwmGetColorizationColor(&colorization, &isOpaqueBlend);
            if (SUCCEEDED(hr) && isOpaqueBlend == FALSE) {
                color = colorization;
                setStyleSheet(QString("PictureDialog{background:%1}").arg(QColor::fromRgba(color).name()));
            }
            else {
                setStyleSheet("PictureDialog{background:palette(window)}");
            }
        }
    }
    ui->jsonFrame->setStyleSheet("QFrame{background:palette(window)}");
#endif
}

#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050000
#if QT_VERSION >= 0x060000
bool PictureDialog::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool PictureDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
    MSG *msg = reinterpret_cast<MSG*>(message);
    if (msg->message == 0x031e || msg->message == 0x0320) {
        styliseDialog();
    }
    return QWidget::nativeEvent(eventType, message, result);
}
#endif
#endif

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
    if (obj == this || obj == ui->labPicture) {
        if (ev->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(ev);
            switch (keyEvent->key()) {
            case Qt::Key_Left:
                emit previousPictureRequested();
                returnValue = true;
                break;
            case Qt::Key_Right:
                emit nextPictureRequested();
                returnValue = true;
                break;
            case Qt::Key_1:
                if (previewMode) {
                    previewMode = false;
                    renderPicture();
                }
                else {
                    previewMode = true;
                    renderPicture();
                }
                break;
            case Qt::Key_2:
                if (overlayEnabled) {
                    overlayEnabled = false;
                    if (!previewMode) renderPicture();
                }
                else {
                    overlayEnabled = true;
                    if (!previewMode) renderPicture();
                }
                break;
            case Qt::Key_M:
                openPreviewMap();
                returnValue = true;
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
#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050000
        if (obj != ui->labPicture && naviEnabled) {
            if (ev->type() == QEvent::MouseButtonPress) {
                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(ev);
                if (mouseEvent->pos().y() <= layout()->menuBar()->height()) {
                    if (mouseEvent->button() == Qt::LeftButton) {
                        dragPosition = mouseEvent->pos();
                        dragStart = true;
                    }
                }
            }
            if (ev->type() == QEvent::MouseButtonRelease) {
                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(ev);
                if (mouseEvent->pos().y() <= layout()->menuBar()->height()) {
                    if (mouseEvent->button() == Qt::LeftButton) {
                        dragStart = false;
                    }
                }
            }
            if (dragStart && ev->type() == QEvent::MouseMove) {
                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(ev);
                if (mouseEvent->pos().y() <= layout()->menuBar()->height()) {
                    if (mouseEvent->buttons() & Qt::LeftButton) {
                        const QPoint diff = mouseEvent->pos() - dragPosition;
                        move(QPoint(pos() + diff));
                        updateGeometry();
                    }
                }
            }
        }
#endif
#endif
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
    manageMenu->popup(pos);
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
    qreal screenRatioPR = AppEnv::screenRatioPR();
    QRect preferedRect = QRect(0, 0, 200 * screenRatio * screenRatioPR, 160 * screenRatio * screenRatioPR);
    QString overlayText = tr("Key 1 - Avatar Preview Mode\nKey 2 - Toggle Overlay\nArrow Keys - Navigate");

    QFont overlayPainterFont;
    overlayPainterFont.setPixelSize(12 * screenRatio * screenRatioPR);
    QFontMetrics fontMetrics(overlayPainterFont);
    QRect overlaySpace = fontMetrics.boundingRect(preferedRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextWordWrap, overlayText);

    int hOverlay = Qt::AlignTop;
    if (overlaySpace.height() < 74 * screenRatio * screenRatioPR) {
        hOverlay = Qt::AlignVCenter;
        preferedRect.setHeight(71 * screenRatio * screenRatioPR);
        overlaySpace.setHeight(80 * screenRatio * screenRatioPR);
    }
    else {
        overlaySpace.setHeight(overlaySpace.height() + 6 * screenRatio * screenRatioPR);
    }

    QImage overlayImage(overlaySpace.size(), QImage::Format_ARGB32_Premultiplied);
    overlayImage.fill(Qt::transparent);

    QPainter overlayPainter(&overlayImage);
    overlayPainter.setPen(QColor::fromRgb(255, 255, 255, 255));
    overlayPainter.setFont(overlayPainterFont);
    overlayPainter.drawText(preferedRect, Qt::AlignLeft | hOverlay | Qt::TextDontClip | Qt::TextWordWrap, overlayText);
    overlayPainter.end();

    if (overlaySpace.width() < 194 * screenRatio * screenRatioPR) {
        overlaySpace.setWidth(200 * screenRatio * screenRatioPR);
    }
    else {
        overlaySpace.setWidth(overlaySpace.width() + 6 * screenRatio * screenRatioPR);
    }

    QImage overlayBorderImage(overlaySpace.width(), overlaySpace.height(), QImage::Format_ARGB6666_Premultiplied);
    overlayBorderImage.fill(QColor(15, 15, 15, 162));

    overlayTempImage = QImage(overlaySpace.width(), overlaySpace.height(), QImage::Format_ARGB6666_Premultiplied);
    overlayTempImage.fill(Qt::transparent);
    QPainter overlayTempPainter(&overlayTempImage);
    overlayTempPainter.drawImage(0, 0, overlayBorderImage);
    overlayTempPainter.drawImage(3 * screenRatio * screenRatioPR, 3 * screenRatio * screenRatioPR, overlayImage);
    overlayTempPainter.end();
}

void PictureDialog::setSnapmaticPicture(SnapmaticPicture *picture, bool readOk, bool _indexed, int _index)
{
    if (smpic != nullptr) {
        QObject::disconnect(smpic, SIGNAL(updated()), this, SLOT(updated()));
        QObject::disconnect(smpic, SIGNAL(customSignal(QString)), this, SLOT(customSignal(QString)));
    }
    snapmaticPicture = QImage();
    indexed = _indexed;
    index = _index;
    smpic = picture;
    if (!readOk) {
        QMessageBox::warning(this, tr("Snapmatic Picture Viewer"), tr("Failed at %1").arg(picture->getLastStep()));
        return;
    }
    if (picture->isPicOk()) {
        snapmaticPicture = picture->getImage();
        renderPicture();
        ui->cmdManage->setEnabled(true);
    }
    if (picture->isJsonOk()) {
        crewStr = crewDB->getCrewName(crewID);
        if (globalMap.contains(picArea)) {
            picAreaStr = globalMap.value(picArea);
        }
        else {
            picAreaStr = picArea;
        }
        setWindowTitle(windowTitleStr.arg(picTitl));
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
        QTimer::singleShot(0, this, SLOT(adaptDialogSize()));
    }
    else {
        ui->labJSON->setText(jsonDrawString.arg("0", "0", "0", tr("No Players"), tr("No Crew"), tr("Unknown Location")));
        QTimer::singleShot(0, this, SLOT(adaptDialogSize()));
    }
    QObject::connect(smpic, SIGNAL(updated()), this, SLOT(updated()));
    QObject::connect(smpic, SIGNAL(customSignal(QString)), this, SLOT(customSignal(QString)));
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
    const qreal screenRatio = AppEnv::screenRatio();
    const qreal screenRatioPR = AppEnv::screenRatioPR();
    const QSize snapmaticResolution(SnapmaticPicture::getSnapmaticResolution());
    const QSize renderResolution(snapmaticResolution.width() * screenRatio * screenRatioPR, snapmaticResolution.height() * screenRatio * screenRatioPR);
    QPixmap shownImagePixmap(renderResolution);
    shownImagePixmap.fill(Qt::black);
    QPainter shownImagePainter(&shownImagePixmap);
    const QImage renderImage = snapmaticPicture.scaled(renderResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (renderImage.width() < renderResolution.width()) {
        shownImagePainter.drawImage((renderResolution.width() - renderImage.width()) / 2, 0, renderImage, Qt::AutoColor);
    }
    else if (renderImage.height() < renderResolution.height()) {
        shownImagePainter.drawImage(0, (renderResolution.height() - renderImage.height()) / 2, renderImage, Qt::AutoColor);
    }
    else {
        shownImagePainter.drawImage(0, 0, renderImage, Qt::AutoColor);
    }
    if (previewMode) {
        QFont shownImagePainterFont;
        shownImagePainterFont.setPixelSize(12 * screenRatio * screenRatioPR);
        shownImagePainter.drawImage(0, 0, avatarAreaPicture);
        shownImagePainter.setPen(QColor::fromRgb(255, 255, 255, 255));
        shownImagePainter.setFont(shownImagePainterFont);
        shownImagePainter.drawText(QRect(3 * screenRatio * screenRatioPR, 3 * screenRatio * screenRatioPR, 140 * screenRatio * screenRatioPR, snapmaticResolution.height() * screenRatio * screenRatioPR), Qt::AlignLeft | Qt::TextWordWrap, tr("Avatar Preview Mode\nPress 1 for Default View"));
    }
    else if (overlayEnabled) {
        shownImagePainter.drawImage(3 * screenRatio * screenRatioPR, 3 * screenRatio * screenRatioPR, overlayTempImage, Qt::AutoColor);
    }
    shownImagePainter.end();
#if QT_VERSION >= 0x050600
    shownImagePixmap.setDevicePixelRatio(screenRatioPR);
#endif
    ui->labPicture->setPixmap(shownImagePixmap);
}

void PictureDialog::crewNameUpdated()
{
    SnapmaticPicture *picture = smpic; // used by macro
    QString crewIDStr = crewID;
    if (crewIDStr == crewStr) {
        crewStr = crewDB->getCrewName(crewIDStr);
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
        QTimer::singleShot(0, this, SLOT(adaptDialogSize()));
    }
}

void PictureDialog::playerNameUpdated()
{
    SnapmaticPicture *picture = smpic; // used by macro
    if (plyrsList.count() >= 1) {
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
        QTimer::singleShot(0, this, SLOT(adaptDialogSize()));
    }
}

QString PictureDialog::generateCrewString()
{
    SnapmaticPicture *picture = smpic; // used by macro
    const QString crewIDStr = crewID; // save operation time
    if (crewIDStr != "0" && !crewIDStr.isEmpty()) {
        if (crewIDStr != crewStr) {
            return QString("<a href=\"https://socialclub.rockstargames.com/crew/" % QString(crewStr).replace(" ", "_") % "/" % crewIDStr % "\">" % crewStr % "</a>");
        }
        else {
            return QString(crewIDStr);
        }
    }
    return tr("No Crew");
}

QString PictureDialog::generatePlayersString()
{
    SnapmaticPicture *picture = smpic; // used by macro
    const QStringList playersList = plyrsList; // save operation time
    QString plyrsStr;
    if (playersList.length() >= 1) {
        for (const QString &player : playersList) {
            const QString playerName = profileDB->getPlayerName(player);
            if (player != playerName) {
                plyrsStr += ", <a href=\"https://socialclub.rockstargames.com/member/" % playerName % "/" % player % "\">" % playerName % "</a>";
            }
            else {
                plyrsStr += ", " % player;
            }
        }
        plyrsStr.remove(0, 2);
    }
    else {
        plyrsStr = tr("No Players");
    }
    return plyrsStr;
}

void PictureDialog::exportSnapmaticPicture()
{
    if (rqFullscreen && fullscreenWidget != nullptr) {
        PictureExport::exportAsPicture(fullscreenWidget, smpic);
    }
    else {
        PictureExport::exportAsPicture(this, smpic);
    }
}

void PictureDialog::copySnapmaticPicture()
{
    if (rqFullscreen && fullscreenWidget != nullptr) {
        PictureExport::exportAsSnapmatic(fullscreenWidget, smpic);
    }
    else {
        PictureExport::exportAsSnapmatic(this, smpic);
    }
}

void PictureDialog::on_labPicture_mouseDoubleClicked(Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
#if QT_VERSION >= 0x060000
        QRect desktopRect = QApplication::screenAt(pos())->geometry();
#else
        QRect desktopRect = QApplication::desktop()->screenGeometry(this);
#endif
        PictureWidget *pictureWidget = new PictureWidget(this); // Work!
        pictureWidget->setObjectName("PictureWidget");
#if QT_VERSION >= 0x050900
        pictureWidget->setWindowFlag(Qt::FramelessWindowHint, true);
        pictureWidget->setWindowFlag(Qt::MaximizeUsingFullscreenGeometryHint, true);
#elif QT_VERSION >= 0x050600
        pictureWidget->setWindowFlags(pictureWidget->windowFlags()^Qt::FramelessWindowHint^Qt::MaximizeUsingFullscreenGeometryHint);
#else
        pictureWidget->setWindowFlags(pictureWidget->windowFlags()^Qt::FramelessWindowHint);
#endif
        pictureWidget->setWindowTitle(windowTitle());
        pictureWidget->setStyleSheet("QLabel#pictureLabel{background-color:black;}");
        pictureWidget->setImage(smpic->getImage(), desktopRect);
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

        fullscreenWidget = nullptr; // Work!
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

void PictureDialog::openPreviewMap()
{
    SnapmaticPicture *picture = smpic;
    SnapmaticProperties currentProperties = picture->getSnapmaticProperties();
    MapLocationDialog *mapLocDialog;
    if (rqFullscreen && fullscreenWidget != nullptr) {
        mapLocDialog = new MapLocationDialog(currentProperties.location.x, currentProperties.location.y, fullscreenWidget);
    }
    else {
        mapLocDialog = new MapLocationDialog(currentProperties.location.x, currentProperties.location.y, this);
    }
    mapLocDialog->setCayoPerico(currentProperties.location.isCayoPerico);
    mapLocDialog->setWindowIcon(windowIcon());
    mapLocDialog->setModal(true);
#ifndef Q_OS_ANDROID
    mapLocDialog->show();
#else
    mapLocDialog->showMaximized();
#endif
    mapLocDialog->exec();
    if (mapLocDialog->propUpdated()) {
        // Update Snapmatic Properties
        currentProperties.location.x = mapLocDialog->getXpos();
        currentProperties.location.y = mapLocDialog->getYpos();
        currentProperties.location.z = 0;

        // Update Snapmatic Picture
        QString currentFilePath = picture->getPictureFilePath();
        QString originalFilePath = picture->getOriginalPictureFilePath();
        QString backupFileName = originalFilePath % ".bak";
        if (!QFile::exists(backupFileName)) {
            QFile::copy(currentFilePath, backupFileName);
        }
        SnapmaticProperties fallbackProperties = picture->getSnapmaticProperties();
        picture->setSnapmaticProperties(currentProperties);
        if (!picture->exportPicture(currentFilePath)) {
            QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("Patching of Snapmatic Properties failed because of I/O Error"));
            picture->setSnapmaticProperties(fallbackProperties);
        }
        else {
            updated();
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush()) {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "LocationEdited";
                jsonObject["ExtraFlags"] = "Viewer";
                jsonObject["EditedSize"] = QString::number(picture->getContentMaxLength());
#if QT_VERSION >= 0x060000
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
#endif
        }
    }
    delete mapLocDialog;
}

void PictureDialog::editSnapmaticProperties()
{
    SnapmaticPicture *picture = smpic;
    SnapmaticEditor *snapmaticEditor;
    if (rqFullscreen && fullscreenWidget != nullptr) {
        snapmaticEditor = new SnapmaticEditor(crewDB, profileDB, fullscreenWidget);
    }
    else {
        snapmaticEditor = new SnapmaticEditor(crewDB, profileDB, this);
    }
    snapmaticEditor->setWindowIcon(windowIcon());
    snapmaticEditor->setSnapmaticPicture(picture);
    snapmaticEditor->setModal(true);
#ifndef Q_OS_ANDROID
    snapmaticEditor->show();
#else
    snapmaticEditor->showMaximized();
#endif
    snapmaticEditor->exec();
    delete snapmaticEditor;
}

void PictureDialog::editSnapmaticImage()
{
    QImage *currentImage = new QImage(smpic->getImage());
    ImportDialog *importDialog;
    if (rqFullscreen && fullscreenWidget != nullptr) {
        importDialog = new ImportDialog(profileName, fullscreenWidget);
    }
    else {
        importDialog = new ImportDialog(profileName, this);
    }
    importDialog->setWindowIcon(windowIcon());
    importDialog->setImage(currentImage);
    importDialog->enableOverwriteMode();
    importDialog->setModal(true);
    importDialog->exec();
    if (importDialog->isImportAgreed()) {
        const QByteArray previousPicture = smpic->getPictureStream();
        bool success = smpic->setImage(importDialog->image(), importDialog->isUnlimitedBuffer());
        if (success) {
            QString currentFilePath = smpic->getPictureFilePath();
            QString originalFilePath = smpic->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName)) {
                QFile::copy(currentFilePath, backupFileName);
            }
            if (!smpic->exportPicture(currentFilePath)) {
                smpic->setPictureStream(previousPicture);
                QMessageBox::warning(this, QApplication::translate("ImageEditorDialog", "Snapmatic Image Editor"), QApplication::translate("ImageEditorDialog", "Patching of Snapmatic Image failed because of I/O Error"));
                return;
            }
            smpic->emitCustomSignal("PictureUpdated");
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush()) {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "ImageEdited";
                jsonObject["ExtraFlags"] = "Viewer";
                jsonObject["EditedSize"] = QString::number(smpic->getContentMaxLength());
#if QT_VERSION >= 0x060000
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
#endif
        }
        else {
            QMessageBox::warning(this, QApplication::translate("ImageEditorDialog", "Snapmatic Image Editor"), QApplication::translate("ImageEditorDialog", "Patching of Snapmatic Image failed because of Image Error"));
            return;
        }
    }
    delete importDialog;
}

void PictureDialog::editSnapmaticRawJson()
{
    SnapmaticPicture *picture = smpic;
    JsonEditorDialog *jsonEditor;
    if (rqFullscreen && fullscreenWidget != nullptr) {
        jsonEditor = new JsonEditorDialog(picture, fullscreenWidget);
    }
    else {
        jsonEditor = new JsonEditorDialog(picture, this);
    }
    jsonEditor->setWindowIcon(windowIcon());
    jsonEditor->setModal(true);
#ifndef Q_OS_ANDROID
    jsonEditor->show();
#else
    jsonEditor->showMaximized();
#endif
    jsonEditor->exec();
    delete jsonEditor;
}

void PictureDialog::updated()
{
    SnapmaticPicture *picture = smpic; // used by macro
    crewStr = crewDB->getCrewName(crewID);
    if (globalMap.contains(picArea)) {
        picAreaStr = globalMap[picArea];
    }
    else {
        picAreaStr = picArea;
    }
    setWindowTitle(windowTitleStr.arg(picTitl));
    ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
    QTimer::singleShot(0, this, SLOT(adaptDialogSize()));
}

void PictureDialog::customSignal(QString signal)
{
    SnapmaticPicture *picture = smpic; // used by macro
    if (signal == "PictureUpdated") {
        snapmaticPicture = picture->getImage();
        renderPicture();
    }
}
