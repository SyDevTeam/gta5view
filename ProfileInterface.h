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

#ifndef PROFILEINTERFACE_H
#define PROFILEINTERFACE_H

#include "SnapmaticPicture.h"
#include "SnapmaticWidget.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "SavegameWidget.h"
#include "ProfileLoader.h"
#include "ProfileWidget.h"
#include "ExportThread.h"
#include "SavegameData.h"
#include "CrewDatabase.h"
#include <QProgressDialog>
#include <QSpacerItem>
#include <QDateTime>
#include <QWidget>
#include <QList>
#include <QMap>

namespace Ui {
class ProfileInterface;
}

enum class MassTool : int { Qualify = 0, Players = 1, Crew = 2, Title = 3 };

class ProfileInterface : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent = 0);
    void setProfileFolder(QString folder, QString profile);
    void settingsApplied(int contentMode, bool languageChanged);
    void setupProfileInterface();
    void massTool(MassTool tool);
    void disableSelected();
    void enableSelected();
    int selectedWidgets();
    void retranslateUi();
    ~ProfileInterface();

public slots:
    void contextMenuTriggeredPIC(QContextMenuEvent* ev);
    void contextMenuTriggeredSGD(QContextMenuEvent* ev);
    void hoverProfileWidgetCheck();
    void selectAllWidgets();
    void deselectAllWidgets();
    void exportSelected();
    void deleteSelected();
    void updatePalette();
    void importFiles();

private slots:
    void on_cmdCloseProfile_clicked();
    void on_cmdImport_clicked();
    void pictureLoaded_event(SnapmaticPicture *picture);
    void pictureFixed_event(SnapmaticPicture *picture);
    void savegameLoaded_event(SavegameData *savegame, QString savegamePath);
    void loadingProgress(int value, int maximum);
    void pictureDeleted_event();
    void savegameDeleted_event();
    void profileLoaded_p();
    void profileWidgetSelected();
    void profileWidgetDeselected();
    void dialogNextPictureRequested(QWidget *dialog);
    void dialogPreviousPictureRequested(QWidget *dialog);
    void on_saProfileContent_dropped(const QMimeData *mimeData);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    ProfileDatabase *profileDB;
    CrewDatabase *crewDB;
    DatabaseThread *threadDB;
    Ui::ProfileInterface *ui;

    ProfileLoader *profileLoader;
    ProfileWidget *previousWidget;
    QList<SavegameData*> savegames;
    QList<SnapmaticPicture*> pictures;
    QMap<ProfileWidget*,QString> widgets;
    QSpacerItem *saSpacerItem;
    QStringList fixedPictures;
    QColor highlightBackColor;
    QColor highlightTextColor;
    QString enabledPicStr;
    QString profileFolder;
    QString profileName;
    QString loadingStr;
    QString language;
    bool contextMenuOpened;
    bool isProfileLoaded;
    int selectedWidgts;
    int contentMode;

    bool isSupportedImageFile(QString selectedFileName);
    bool importFile(QString selectedFile, QDateTime importDateTime, int *currentTime, bool notMultiple);
    void importFilesProgress(QStringList selectedFiles);
    bool importSnapmaticPicture(SnapmaticPicture *picture, bool warn = true);
    bool importSavegameData(SavegameData *savegame, QString sgdPath, bool warn = true);
    void pictureLoaded(SnapmaticPicture *picture, bool inserted);
    void savegameLoaded(SavegameData *savegame, QString savegamePath, bool inserted);
    void savegameDeleted(SavegameWidget *sgdWidget, bool isRemoteEmited = false);
    void pictureDeleted(SnapmaticWidget *picWidget, bool isRemoteEmited = false);
    void insertSnapmaticIPI(QWidget *widget);
    void insertSavegameIPI(QWidget *widget);
    void sortingProfileInterface();

signals:
    void profileLoaded();
    void profileClosed();
};

#endif // PROFILEINTERFACE_H
