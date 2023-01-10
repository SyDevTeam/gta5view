/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2017-2021 Syping
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
#include <QMenu>

namespace Ui {
class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QString profileName, QWidget *parent = 0);
    ~ImportDialog();
    QImage image();
    QString getImageTitle();
    void setImage(QImage *image);
    void lockSettings(bool lock);
    void enableOverwriteMode();
    bool isImportAgreed();
    bool isUnlimitedBuffer();
    bool areSettingsLocked();

private slots:
    void processImage();
    void reworkImage();
    void cropPicture();
    void importNewPicture();
    void loadImportSettings();
    void saveImportSettings();
    void on_cbIgnore_toggled(bool checked);
    void on_cbAvatar_toggled(bool checked);
    void on_cmdCancel_clicked();
    void on_cmdOK_clicked();
    void on_labPicture_labelPainted();
    void on_cmdColourChange_clicked();
    void on_cmdBackgroundChange_clicked();
    void on_cmdBackgroundWipe_clicked();
    void on_cbStretch_toggled(bool checked);
    void on_cbForceAvatarColour_toggled(bool checked);
    void on_cbWatermark_toggled(bool checked);
    void on_cbBorderless_toggled(bool checked);
    void on_cbImportAsIs_toggled(bool checked);
    void on_cbResolution_currentIndexChanged(int index);

private:
    QString profileName;
    Ui::ImportDialog *ui;
    QImage avatarAreaImage;
    QString backgroundPath;
    QString imageTitle;
    QImage backImage;
    QImage workImage;
    QImage origImage;
    QImage newImage;
    QColor selectedColour;
    QMenu optionsMenu;
    QSize snapmaticResolution;
    bool insideAvatarZone;
    bool watermarkPicture;
    bool watermarkAvatar;
    bool watermarkBlock;
    bool settingsLocked;
    bool importAgreed;
    int snapmaticResolutionLW;
    int snapmaticResolutionLH;
    void processWatermark(QPainter *snapmaticPainter);
    void processSettings(QString settingsProfile, bool setDefault = false);
    void saveSettings(QString settingsProfile);
};

#endif // IMPORTDIALOG_H
