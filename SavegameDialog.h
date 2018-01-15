/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016-2018 Syping
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

#ifndef SAVEGAMEDIALOG_H
#define SAVEGAMEDIALOG_H

#include "SavegameData.h"
#include <QDialog>

namespace Ui {
class SavegameDialog;
}

class SavegameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SavegameDialog(QWidget *parent = 0);
    void setSavegameData(SavegameData *savegame, QString sgdPath, bool readOk);
    ~SavegameDialog();

private slots:
    void on_cmdClose_clicked();
    void on_cmdCopy_clicked();
    void refreshWindowSize();

private:
    Ui::SavegameDialog *ui;
    QString savegameLabStr;
    QString sgdPath;
};

#endif // SAVEGAMEDIALOG_H
