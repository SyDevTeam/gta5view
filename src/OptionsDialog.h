/******************************************************************************
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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QSize>
#include <QList>
#include <QDialog>
#include <QSettings>
#include <QTreeWidgetItem>
#include "ProfileDatabase.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(ProfileDatabase *profileDB, QWidget *parent = 0);
    void commitProfiles(const QStringList &profiles, const QString &game);
    ~OptionsDialog();

private slots:
    void on_cmdOK_clicked();
    void on_cmdExploreFolder_clicked();
    void on_cmdExploreFolder_RDR2_clicked();
    void on_cbDefaultStyle_toggled(bool checked);
    void on_cbDefaultFont_toggled(bool checked);
    void on_cmdCopyStatsID_clicked();
    void on_cbFont_currentFontChanged(const QFont &font);
    void on_cmdFont_clicked();

signals:
    void settingsApplied(int contentMode, bool languageChanged);

private:
    ProfileDatabase *profileDB;
    Ui::OptionsDialog *ui;
    QList<QTreeWidgetItem*> playerItems;
    Qt::AspectRatioMode aspectRatio;
    QString currentAreaLanguage;
    QString currentLanguage;
    QString currentCFolder;
    QString currentCFolderR;
    QString defaultProfile;
    QString defaultGame;
    QString percentString;
    bool withoutPlayers;
    bool currentFFolder;
    bool currentFFolderR;
    int contentMode;
    void setupTreeWidget();
    void setupLanguageBox(QSettings *settings);
    void setupRadioButtons(QSettings *settings);
    void setupDefaultProfile(QSettings *settings);
    void setupCustomGameFolder(QSettings *settings);
    void setupInterfaceSettings(QSettings *settings);
    void setupStatisticsSettings(QSettings *settings);
    void setupSnapmaticPictureViewer(QSettings *settings);
    void setupWindowsGameSettings();
    void applySettings();
};

#endif // OPTIONSDIALOG_H
