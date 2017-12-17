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

#ifndef IMAGEEDITORDIALOG_H
#define IMAGEEDITORDIALOG_H

#include "SnapmaticPicture.h"
#include <QDialog>

namespace Ui {
class ImageEditorDialog;
}

class ImageEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageEditorDialog(SnapmaticPicture *picture, QString profileName, QWidget *parent = 0);
    ~ImageEditorDialog();

private slots:
    void on_cmdClose_clicked();
    void on_cmdReplace_clicked();
    void on_cmdSave_clicked();
    void on_cmdQuestion_clicked();

private:
    SnapmaticPicture *smpic;
    QString profileName;
    Ui::ImageEditorDialog *ui;
    int snapmaticResolutionLW;
    int snapmaticResolutionLH;
    bool imageIsChanged;
    QImage pictureCache;
};

#endif // IMAGEEDITORDIALOG_H
