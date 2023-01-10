/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
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

#ifndef SNAPMATICEDITOR_H
#define SNAPMATICEDITOR_H

#include <QDialog>
#include "CrewDatabase.h"
#include "ProfileDatabase.h"
#include "SnapmaticPicture.h"

namespace Ui {
class SnapmaticEditor;
}

class SnapmaticEditor : public QDialog
{
    Q_OBJECT

public:
    explicit SnapmaticEditor(CrewDatabase *crewDB, ProfileDatabase *profileDB, QWidget *parent = 0);
    void setSnapmaticPicture(SnapmaticPicture *picture);
    void setSnapmaticPlayers(const QStringList &players);
    void setSnapmaticTitle(const QString &title);
    void setSnapmaticCrew(const QString &crew = "");
    QString returnCrewName(int crewID);
    ~SnapmaticEditor();

private slots:
    void on_rbSelfie_toggled(bool checked);
    void on_rbMugshot_toggled(bool checked);
    void on_rbEditor_toggled(bool checked);
    void on_rbCustom_toggled(bool checked);
    void on_cmdCancel_clicked();
    void on_cmdApply_clicked();
    void on_cbQualify_toggled(bool checked);
    void on_labPlayers_linkActivated(const QString &link);
    void on_labTitle_linkActivated(const QString &link);
    void on_labCrew_linkActivated(const QString &link);
    void playerListUpdated(QStringList playerList);

private:
    CrewDatabase *crewDB;
    ProfileDatabase *profileDB;
    Ui::SnapmaticEditor *ui;
    SnapmaticProperties snapmaticProperties;
    SnapmaticPicture *smpic;
    QStringList playersList;
    QString snapmaticTitle;
    int crewID;
    bool isSelfie;
    bool isMugshot;
    bool isEditor;
    void selfie_toggled(bool checked);
    void mugshot_toggled(bool checked);
    void editor_toggled(bool checked);
    void qualifyAvatar();
    void insertPlayerNames(QStringList *players);
    QStringList insertPlayerNames(const QStringList &players);
};

#endif // SNAPMATICEDITOR_H
