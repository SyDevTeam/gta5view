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

#ifndef PICTUREDIALOG_H
#define PICTUREDIALOG_H

#include "SnapmaticPicture.h"
#include "ProfileDatabase.h"
#include "CrewDatabase.h"
#include <QResizeEvent>
#include <QMouseEvent>
#include <QToolBar>
#include <QDialog>
#include <QEvent>
#include <QMenu>

namespace Ui {
class PictureDialog;
}

class PictureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PictureDialog(ProfileDatabase *profileDB, CrewDatabase *crewDB, QWidget *parent = 0);
    explicit PictureDialog(ProfileDatabase *profileDB, CrewDatabase *crewDB, QString profileName, QWidget *parent = 0);
    explicit PictureDialog(bool primaryWindow, ProfileDatabase *profileDB, CrewDatabase *crewDB, QWidget *parent = 0);
    explicit PictureDialog(bool primaryWindow, ProfileDatabase *profileDB, CrewDatabase *crewDB, QString profileName, QWidget *parent = 0);
    void setupPictureDialog();
    void setSnapmaticPicture(SnapmaticPicture *picture, bool readOk, bool indexed, int index);
    void setSnapmaticPicture(SnapmaticPicture *picture, bool readOk, int index);
    void setSnapmaticPicture(SnapmaticPicture *picture, bool readOk);
    void setSnapmaticPicture(SnapmaticPicture *picture, int index);
    void setSnapmaticPicture(SnapmaticPicture *picture);
    void addPreviousNextButtons();
    void styliseDialog();
    bool isIndexed();
    int getIndex();
    ~PictureDialog();

public slots:
    void adaptDialogSize();
    void crewNameUpdated();
    void playerNameUpdated();
    void dialogNextPictureRequested();
    void dialogPreviousPictureRequested();
    void exportCustomContextMenuRequested(const QPoint &pos);

private slots:
    void copySnapmaticPicture();
    void exportSnapmaticPicture();
    void triggerFullscreenDoubeClick();
    void on_labPicture_mouseDoubleClicked(Qt::MouseButton button);
    void on_labPicture_customContextMenuRequested(const QPoint &pos);
    void exportCustomContextMenuRequestedPrivate(const QPoint &pos, bool fullscreen);
    void nextPictureRequestedSlot();
    void previousPictureRequestedSlot();
    void editSnapmaticProperties();
    void editSnapmaticRawJson();
    void editSnapmaticImage();
    void renderOverlayPicture();
    void renderPicture();
    void openPreviewMap();
    void updated();
    void customSignal(QString signal);

signals:
    void nextPictureRequested();
    void previousPictureRequested();
    void newPictureCommited(QImage picture);
    void endDatabaseThread();

protected:
    void closeEvent(QCloseEvent *ev);
    bool eventFilter(QObject *obj, QEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
#ifdef Q_OS_WIN
#if QT_VERSION >= 0x060000
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);
#elif QT_VERSION >= 0x050000
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif
#endif

private:
    QString generateCrewString();
    QString generatePlayersString();
    bool primaryWindow;
    ProfileDatabase *profileDB;
    CrewDatabase *crewDB;
    QString profileName;
    Ui::PictureDialog *ui;
    QMap<QString, QString> globalMap;
    SnapmaticPicture *smpic;
    QWidget *fullscreenWidget;
    QImage avatarAreaPicture;
    QImage snapmaticPicture;
    QImage overlayTempImage;
    QString jsonDrawString;
    QString windowTitleStr;
    QString picAreaStr;
    QString crewStr;
    bool overlayEnabled;
    bool rqFullscreen;
    bool naviEnabled;
    bool previewMode;
    bool indexed;
    int index;
    int avatarLocX;
    int avatarLocY;
    int avatarSize;
    QMenu *manageMenu;
#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050000
    QPoint dragPosition;
    bool dragStart;
#endif
#endif
};

#endif // PICTUREDIALOG_H
