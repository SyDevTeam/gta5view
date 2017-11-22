/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2017 Syping
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

#ifndef MAPLOCATIONDIALOG_H
#define MAPLOCATIONDIALOG_H

#include <QDialog>
#include <QMouseEvent>

namespace Ui {
class MapLocationDialog;
}

class MapLocationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapLocationDialog(double x, double y, QWidget *parent = 0);
    void drawPointOnMap(double x, double y);
    bool propUpdated();
    double getXpos();
    double getYpos();
    ~MapLocationDialog();

protected:
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

private slots:
    void on_cmdDone_clicked();
    void on_cmdApply_clicked();
    void on_cmdChange_clicked();
    void on_cmdRevert_clicked();
    void updatePosFromEvent(int x, int y);
    void on_cmdClose_clicked();

private:
    double xpos_old;
    double ypos_old;
    double xpos_new;
    double ypos_new;
    bool propUpdate;
    bool changeMode;
    Ui::MapLocationDialog *ui;
};

#endif // MAPLOCATIONDIALOG_H
