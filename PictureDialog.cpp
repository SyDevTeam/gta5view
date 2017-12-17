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
#include "MapLocationDialog.h"
#include "ImageEditorDialog.h"
#include "JsonEditorDialog.h"
#include "SnapmaticEditor.h"
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

#include <QStringBuilder>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QApplication>
#include <QFontMetrics>
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
#include <QImage>
#include <QDebug>
#include <QList>
#include <QDrag>
#include <QIcon>
#include <QUrl>
#include <QDir>

// Macros for better Overview + RAM
#define locX QString::number(picture->getSnapmaticProperties().location.x)
#define locY QString::number(picture->getSnapmaticProperties().location.y)
#define locZ QString::number(picture->getSnapmaticProperties().location.z)
#define crewID QString::number(picture->getSnapmaticProperties().crewID)
#define picArea picture->getSnapmaticProperties().location.area
#define picPath picture->getPictureFilePath()
#define picTitl StringParser::escapeString(picture->getPictureTitle())
#define plyrsList picture->getSnapmaticProperties().playersList
#define created picture->getSnapmaticProperties().createdDateTime.toString(Qt::DefaultLocaleShortDate)

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
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint^Qt::CustomizeWindowHint);
#ifdef Q_OS_LINUX
    // for stupid Window Manager (GNOME 3 should feel triggered)
    setWindowFlags(windowFlags()^Qt::Dialog^Qt::Window);
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

    // Pre-adapt window for DPI
    setFixedWidth(960 * screenRatio);
    setFixedHeight(536 * screenRatio);
}

PictureDialog::~PictureDialog()
{
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
    if (naviEnabled)
    {
        for (QObject *obj : layout()->menuBar()->children())
        {
            delete obj;
        }
        delete layout()->menuBar();
    }
#endif
#endif
    for (QObject *obj : manageMenu->children())
    {
        delete obj;
    }
    delete manageMenu;
    delete ui;
}

void PictureDialog::closeEvent(QCloseEvent *ev)
{
    Q_UNUSED(ev)
    if (primaryWindow)
    {
        emit endDatabaseThread();
    }
}

void PictureDialog::addPreviousNextButtons()
{
    // Windows Vista additions
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
    QToolBar *uiToolbar = new QToolBar("Picture Toolbar", this);
    uiToolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    uiToolbar->setObjectName("uiToolbar");
    uiToolbar->addAction(QIcon(":/img/back.png"), "", this, SLOT(previousPictureRequestedSlot()));
    uiToolbar->addAction(QIcon(":/img/next.png"), "", this, SLOT(nextPictureRequestedSlot()));
    layout()->setMenuBar(uiToolbar);

    naviEnabled = true;
#endif
#endif
}

#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
#ifdef GTA5SYNC_APV
bool PictureDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    *result = 0;
    MSG *msg = static_cast<MSG*>(message);
    LRESULT lRet = 0;

    if (naviEnabled && QtWin::isCompositionEnabled())
    {
        if (msg->message == WM_NCCALCSIZE && msg->wParam == TRUE)
        {
            NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);

            int sideBorderSize = ((frameSize().width() - size().width()) / 2);
#ifdef GTA5SYNC_APV_SIDE
            int buttomBorderSize = sideBorderSize;
#else
            int buttomBorderSize = (frameSize().height() - size().height());
#endif
            pncsp->rgrc[0].left += sideBorderSize;
            pncsp->rgrc[0].right -= sideBorderSize;
            pncsp->rgrc[0].bottom -= buttomBorderSize;
        }
        else if (msg->message == WM_NCHITTEST)
        {
            int CLOSE_BUTTON_ID = 20;
            lRet = HitTestNCA(msg->hwnd, msg->lParam);
            DwmDefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam, &lRet);
            *result = lRet;
            if (lRet != CLOSE_BUTTON_ID) { return QWidget::nativeEvent(eventType, message, result); }
        }
        else
        {
            return QWidget::nativeEvent(eventType, message, result);
        }
    }
    else
    {
        return QWidget::nativeEvent(eventType, message, result);
    }
    return true;
}

LRESULT PictureDialog::HitTestNCA(HWND hWnd, LPARAM lParam)
{
    int LEFTEXTENDWIDTH = 0;
    int RIGHTEXTENDWIDTH = 0;
    int BOTTOMEXTENDWIDTH = 0;
    int TOPEXTENDWIDTH = layout()->menuBar()->height();

    POINT ptMouse = {(int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)};

    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    RECT rcFrame = {};
    AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

    USHORT uRow = 1;
    USHORT uCol = 1;
    bool fOnResizeBorder = false;

    if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + TOPEXTENDWIDTH)
    {
        fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
        uRow = 0;
    }
    else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - BOTTOMEXTENDWIDTH)
    {
        uRow = 2;
    }

    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + LEFTEXTENDWIDTH)
    {
        uCol = 0;
    }
    else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - RIGHTEXTENDWIDTH)
    {
        uCol = 2;
    }

    LRESULT hitTests[3][3] =
    {
        { HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
        { HTLEFT,       HTNOWHERE,     HTRIGHT },
        { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
    };

    return hitTests[uRow][uCol];
}

void PictureDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    //    int newDialogHeight = ui->labPicture->pixmap()->height();
    //    newDialogHeight = newDialogHeight + ui->jsonFrame->height();
    //    if (naviEnabled) newDialogHeight = newDialogHeight + layout()->menuBar()->height();
    //    int buttomBorderSize = (frameSize().height() - size().height());
    //    int sideBorderSize = ((frameSize().width() - size().width()) / 2);
    //    int brokenDialogHeight = newDialogHeight + (buttomBorderSize - sideBorderSize);
    //    if (event->size().height() == brokenDialogHeight)
    //    {
    //        qDebug() << "BROKEN 1";
    //        setGeometry(geometry().x(), geometry().y(), width(), newDialogHeight);
    //        qDebug() << "BROKEN 2";
    //        event->ignore();
    //    }
}
#endif
#endif
#endif

void PictureDialog::adaptNewDialogSize(QSize newLabelSize)
{
    Q_UNUSED(newLabelSize)
    int newDialogHeight = ui->labPicture->pixmap()->height();
    newDialogHeight = newDialogHeight + ui->jsonFrame->height();
    if (naviEnabled) newDialogHeight = newDialogHeight + layout()->menuBar()->height();
    setMaximumSize(width(), newDialogHeight);
    setMinimumSize(width(), newDialogHeight);
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
        QPalette palette;
        QtWin::extendFrameIntoClientArea(this, 0, this->layout()->menuBar()->height(), 0, 0);
        ui->jsonFrame->setStyleSheet(QString("QFrame { background: %1; }").arg(palette.window().color().name()));
        setStyleSheet("PictureDialog { background: transparent; }");
    }
    else
    {
        QPalette palette;
        QtWin::resetExtendedFrame(this);
        ui->jsonFrame->setStyleSheet(QString("QFrame { background: %1; }").arg(palette.window().color().name()));
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
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(ev);
            switch (keyEvent->key()){
            case Qt::Key_Left:
                emit previousPictureRequested();
                returnValue = true;
                break;
            case Qt::Key_Right:
                emit nextPictureRequested();
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
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
        if (obj != ui->labPicture && naviEnabled)
        {
            if (ev->type() == QEvent::MouseButtonPress)
            {
                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(ev);
                if (mouseEvent->pos().y() <= layout()->menuBar()->height())
                {
                    if (mouseEvent->button() == Qt::LeftButton)
                    {
                        dragPosition = mouseEvent->pos();
                        dragStart = true;
                    }
                }
            }
            if (ev->type() == QEvent::MouseButtonRelease)
            {
                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(ev);
                if (mouseEvent->pos().y() <= layout()->menuBar()->height())
                {
                    if (mouseEvent->button() == Qt::LeftButton)
                    {
                        dragStart = false;
                    }
                }
            }
            if (ev->type() == QEvent::MouseMove && dragStart)
            {
                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(ev);
                if (mouseEvent->pos().y() <= layout()->menuBar()->height())
                {
                    if (mouseEvent->buttons() & Qt::LeftButton)
                    {
                        QPoint diff = mouseEvent->pos() - dragPosition;
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
    QRect preferedRect = QRect(0, 0, 200 * screenRatio, 160 * screenRatio);
    QString overlayText = tr("Key 1 - Avatar Preview Mode\nKey 2 - Toggle Overlay\nArrow Keys - Navigate");

    QFont overlayPainterFont;
    overlayPainterFont.setPixelSize(12 * screenRatio);
    QFontMetrics fontMetrics(overlayPainterFont);
    QRect overlaySpace = fontMetrics.boundingRect(preferedRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextWordWrap, overlayText);

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

    QImage overlayImage(overlaySpace.size(), QImage::Format_ARGB32_Premultiplied);
    overlayImage.fill(Qt::transparent);

    QPainter overlayPainter(&overlayImage);
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
    if (smpic != nullptr) smpic->disconnect();
    snapmaticPicture = QImage();
    indexed = _indexed;
    index = _index;
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
        ui->cmdManage->setEnabled(true);
    }
    if (picture->isJsonOk())
    {
        crewStr = crewDB->getCrewName(crewID);
        if (globalMap.contains(picArea))
        {
            picAreaStr = globalMap[picArea];
        }
        else
        {
            picAreaStr = picArea;
        }

        this->setWindowTitle(windowTitleStr.arg(picture->getPictureStr()));
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
    }
    else
    {
        ui->labJSON->setText(jsonDrawString.arg("0", "0", "0", tr("No Players"), tr("No Crew"), tr("Unknown Location")));
        QMessageBox::warning(this,tr("Snapmatic Picture Viewer"),tr("Failed at %1").arg(picture->getLastStep()));
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
        snapPainter.drawText(QRect(3 * screenRatio, 3 * screenRatio, 140 * screenRatio, 536 * screenRatio), Qt::AlignLeft | Qt::TextWordWrap, tr("Avatar Preview Mode\nPress 1 for Default View"));
        snapPainter.end();
        ui->labPicture->setPixmap(avatarPixmap);
    }
}

void PictureDialog::crewNameUpdated()
{
    SnapmaticPicture *picture = smpic; // used by macro
    QString crewIDStr = crewID;
    if (crewIDStr == crewStr)
    {
        crewStr = crewDB->getCrewName(crewIDStr);
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
    }
}

void PictureDialog::playerNameUpdated()
{
    SnapmaticPicture *picture = smpic; // used by macro
    if (plyrsList.count() >= 1)
    {
        ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
    }
}

QString PictureDialog::generateCrewString()
{
    SnapmaticPicture *picture = smpic; // used by macro
    QString crewIDStr = crewID; // save operation time
    if (crewIDStr != "0" && !crewIDStr.isEmpty())
    {
        return QString("<a href=\"https://socialclub.rockstargames.com/crew/" % QString(crewStr).replace(" ", "_") % "/" % crewIDStr % "\">" % crewStr % "</a>");
    }
    return tr("No Crew");
}

QString PictureDialog::generatePlayersString()
{
    SnapmaticPicture *picture = smpic; // used by macro
    const QStringList playersList = plyrsList; // save operation time
    QString plyrsStr;
    if (playersList.length() >= 1)
    {
        for (QString player : playersList)
        {
            QString playerName;
            playerName = profileDB->getPlayerName(player);
            plyrsStr += ", <a href=\"https://socialclub.rockstargames.com/member/" % playerName % "/" % player % "\">" % playerName % "</a>";
        }
        plyrsStr.remove(0, 2);
    }
    else
    {
        plyrsStr = tr("No Players");
    }
    return plyrsStr;
}

void PictureDialog::exportSnapmaticPicture()
{
    if (rqFullscreen && fullscreenWidget != nullptr)
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
    if (rqFullscreen && fullscreenWidget != nullptr)
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
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
        QtWin::markFullscreenWindow(pictureWidget, true);
#endif
#endif
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
    MapLocationDialog *mapLocDialog;
    if (rqFullscreen && fullscreenWidget != nullptr)
    {
        mapLocDialog = new MapLocationDialog(picture->getSnapmaticProperties().location.x, picture->getSnapmaticProperties().location.y, fullscreenWidget);
    }
    else
    {
        mapLocDialog = new MapLocationDialog(picture->getSnapmaticProperties().location.x, picture->getSnapmaticProperties().location.y, this);
    }
    mapLocDialog->setWindowIcon(windowIcon());
    mapLocDialog->setModal(true);
#ifndef Q_OS_ANDROID
    mapLocDialog->show();
#else
    mapLocDialog->showMaximized();
#endif
    mapLocDialog->exec();
    if (mapLocDialog->propUpdated())
    {
        // Update Snapmatic Properties
        SnapmaticProperties localSpJson = picture->getSnapmaticProperties();
        localSpJson.location.x = mapLocDialog->getXpos();
        localSpJson.location.y = mapLocDialog->getYpos();
        localSpJson.location.z = 0;

        // Update Snapmatic Picture
        QString currentFilePath = picture->getPictureFilePath();
        QString originalFilePath = picture->getOriginalPictureFilePath();
        QString backupFileName = originalFilePath % ".bak";
        if (!QFile::exists(backupFileName))
        {
            QFile::copy(currentFilePath, backupFileName);
        }
        SnapmaticProperties fallbackProperties = picture->getSnapmaticProperties();
        picture->setSnapmaticProperties(localSpJson);
        if (!picture->exportPicture(currentFilePath))
        {
            QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("Patching of Snapmatic Properties failed because of I/O Error"));
            picture->setSnapmaticProperties(fallbackProperties);
        }
        else
        {
            updated();
        }
    }
    delete mapLocDialog;
}

void PictureDialog::editSnapmaticProperties()
{
    SnapmaticPicture *picture = smpic;
    SnapmaticEditor *snapmaticEditor;
    if (rqFullscreen && fullscreenWidget != nullptr)
    {
        snapmaticEditor = new SnapmaticEditor(crewDB, profileDB, fullscreenWidget);
    }
    else
    {
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
    SnapmaticPicture *picture = smpic;
    ImageEditorDialog *imageEditor;
    if (rqFullscreen && fullscreenWidget != nullptr)
    {
        imageEditor = new ImageEditorDialog(picture, profileName, fullscreenWidget);
    }
    else
    {
        imageEditor = new ImageEditorDialog(picture, profileName, this);
    }
    imageEditor->setWindowIcon(windowIcon());
    imageEditor->setModal(true);
#ifndef Q_OS_ANDROID
    imageEditor->show();
#else
    snapmaticEditor->showMaximized();
#endif
    imageEditor->exec();
    delete imageEditor;
}

void PictureDialog::editSnapmaticRawJson()
{
    SnapmaticPicture *picture = smpic;
    JsonEditorDialog *jsonEditor;
    if (rqFullscreen && fullscreenWidget != nullptr)
    {
        jsonEditor = new JsonEditorDialog(picture, fullscreenWidget);
    }
    else
    {
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
    if (globalMap.contains(picArea))
    {
        picAreaStr = globalMap[picArea];
    }
    else
    {
        picAreaStr = picArea;
    }
    ui->labJSON->setText(jsonDrawString.arg(locX, locY, locZ, generatePlayersString(), generateCrewString(), picTitl, picAreaStr, created));
}

void PictureDialog::customSignal(QString signal)
{
    SnapmaticPicture *picture = smpic; // used by macro
    if (signal == "PictureUpdated")
    {
        snapmaticPicture = picture->getImage();
        renderPicture();
    }
}
