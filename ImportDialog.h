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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = 0);
    ~ImportDialog();
    QImage image();
    QString getImageTitle();
    void setImage(const QImage &image);
    bool isImportAgreed();

private slots:
    void processImage();
    void on_cbIgnore_toggled(bool checked);
    void on_cbAvatar_toggled(bool checked);
    void on_cmdCancel_clicked();
    void on_cmdOK_clicked();
    void on_labPicture_labelPainted();
    void on_cmdColourChange_clicked();

private:
    Ui::ImportDialog *ui;
    QImage avatarAreaImage;
    QString imageTitle;
    QImage workImage;
    QImage newImage;
    QColor selectedColour;
    bool insideAvatarZone;
    bool importAgreed;
    int snapmaticResolutionLW;
    int snapmaticResolutionLH;
};

#endif // IMPORTDIALOG_H
