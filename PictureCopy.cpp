/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping
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

#include "PictureCopy.h"
#include "PictureDialog.h"
#include "SidebarGenerator.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>

PictureCopy::PictureCopy()
{

}

void PictureCopy::copyPicture(QWidget *parent, QString picPath)
{
    QSettings settings("Syping", "gta5sync");
    settings.beginGroup("FileDialogs");

fileDialogPreSave:
    QFileInfo sgdFileInfo(picPath);
    QFileDialog fileDialog(parent);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix("");
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(PictureDialog::tr("Export picture for Import..."));
    fileDialog.setLabelText(QFileDialog::Accept, PictureDialog::tr("&Export"));

    QStringList filters;
    filters << PictureDialog::tr("Snapmatic pictures (PGTA*)");
    filters << PictureDialog::tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.restoreState(settings.value("CopyPicture","").toByteArray());
    fileDialog.selectFile(sgdFileInfo.fileName());

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);

            if (QFile::exists(selectedFile))
            {
                if (QMessageBox::Yes == QMessageBox::warning(parent, PictureDialog::tr("Export picture for Import"), PictureDialog::tr("Overwrite %1 with current Snapmatic picture?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                {
                    if (!QFile::remove(selectedFile))
                    {
                        QMessageBox::warning(parent, PictureDialog::tr("Export picture for Import"), PictureDialog::tr("Failed to overwrite %1 with current Snapmatic picture").arg("\""+selectedFile+"\""));
                        goto fileDialogPreSave;
                    }
                }
                else
                {
                    goto fileDialogPreSave;
                }
            }

            bool isCopied = QFile::copy(picPath, selectedFile);
            if (!isCopied)
            {
                QMessageBox::warning(parent, PictureDialog::tr("Export picture for Import"), PictureDialog::tr("Failed to copy current Snapmatic picture"));
                goto fileDialogPreSave;
            }
        }
        else
        {
            QMessageBox::warning(parent, PictureDialog::tr("Export picture for Import"), PictureDialog::tr("No valid file is selected"));
            goto fileDialogPreSave;
        }
    }

    settings.setValue("CopyPicture", fileDialog.saveState());
    settings.endGroup();
}
