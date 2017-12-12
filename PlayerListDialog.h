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

#ifndef PLAYERLISTDIALOG_H
#define PLAYERLISTDIALOG_H

#include "ProfileDatabase.h"
#include <QDialog>

namespace Ui {
class PlayerListDialog;
}

class PlayerListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayerListDialog(QStringList players, ProfileDatabase *profileDB, QWidget *parent = 0);
    ~PlayerListDialog();

private slots:
    void on_cmdCancel_clicked();
    void on_cmdMakeAv_clicked();
    void on_cmdMakeSe_clicked();
    void on_cmdMakeAd_clicked();
    void on_cmdApply_clicked();

private:
    QStringList players;
    ProfileDatabase *profileDB;
    Ui::PlayerListDialog *ui;
    void drawSwitchButtons();
    void buildInterface();

signals:
    void playerListUpdated(QStringList playerList);
};

#endif // PLAYERLISTDIALOG_H
