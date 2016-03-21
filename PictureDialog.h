/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
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
#include <QDialog>

namespace Ui {
class PictureDialog;
}

class PictureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PictureDialog(QWidget *parent = 0);
    void setSnapmaticPicture(SnapmaticPicture *picture, bool readOk);
    ~PictureDialog();

private slots:
    void on_cmdClose_clicked();

private:
    Ui::PictureDialog *ui;
    QString jsonDrawString;
    QString windowTitleStr;
};

#endif // PICTUREDIALOG_H
