/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2023 Syping
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

#include "SavegameWidget.h"
#include "StandardPaths.h"
#include "SavegameCopy.h"
#include "config.h"
#include <QStringBuilder>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>

SavegameCopy::SavegameCopy()
{
}

void SavegameCopy::copySavegame(QWidget *parent, QString sgdPath)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);

    settings.beginGroup("FileDialogs");
    bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
    settings.beginGroup("SavegameCopy");

fileDialogPreSave: //Work?
    QFileInfo sgdFileInfo(sgdPath);
    QFileDialog fileDialog(parent);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, dontUseNativeDialog);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix("");
    fileDialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    fileDialog.setWindowTitle(SavegameWidget::tr(("Export Savegame...")));
    fileDialog.setLabelText(QFileDialog::Accept, SavegameWidget::tr("Export"));

    QStringList filters;
    const QString fileName = sgdFileInfo.fileName();
    if (fileName.startsWith("SGTA5"))
        filters << SavegameWidget::tr("GTA V Savegames files (%1)").arg("SGTA5*");
    else if (fileName.startsWith("SRDR3"))
        filters << SavegameWidget::tr("RDR 2 Savegames files (%1)").arg("SRDR3*");
    filters << SavegameWidget::tr("All files (%1)").arg("**");
    fileDialog.setNameFilters(filters);

    fileDialog.setDirectory(settings.value("Directory", StandardPaths::picturesLocation()).toString());
    fileDialog.restoreGeometry(settings.value(parent->objectName() % "+Geometry", "").toByteArray());
    fileDialog.selectFile(sgdFileInfo.fileName());

    if (fileDialog.exec()) {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1) {
            QString selectedFile = selectedFiles.at(0);

            if (QFile::exists(selectedFile)) {
                if (QMessageBox::Yes == QMessageBox::warning(parent, SavegameWidget::tr("Export Savegame"), SavegameWidget::tr("Overwrite %1 with current Savegame?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
                    if (!QFile::remove(selectedFile)) {
                        QMessageBox::warning(parent, SavegameWidget::tr("Export Savegame"), SavegameWidget::tr("Failed to overwrite %1 with current Savegame").arg("\""+selectedFile+"\""));
                        goto fileDialogPreSave; //Work?
                    }
                }
                else {
                    goto fileDialogPreSave; //Work?
                }
            }

            bool isCopied = QFile::copy(sgdPath, selectedFile);
            if (!isCopied) {
                QMessageBox::warning(parent, SavegameWidget::tr("Export Savegame"), SavegameWidget::tr("Failed to export current Savegame"));
                goto fileDialogPreSave; //Work?
            }
        }
        else {
            QMessageBox::warning(parent, SavegameWidget::tr("Export Savegame"), SavegameWidget::tr("No valid file is selected"));
            goto fileDialogPreSave; //Work?
        }
    }

    settings.setValue(parent->objectName() % "+Geometry", fileDialog.saveGeometry());
    settings.setValue("Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}
