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

#include "config.h"
#include "PictureCopy.h"
#include "PictureDialog.h"
#include "StandardPaths.h"
#include "SidebarGenerator.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>

PictureCopy::PictureCopy()
{

}

void PictureCopy::copyPicture(QWidget *parent, QString picPath, SnapmaticPicture *picture)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");
    settings.beginGroup("PictureCopy");

    QString adjustedPicPath = picPath;
    if (adjustedPicPath.right(7) == ".hidden") // for the hidden file system
    {
        adjustedPicPath.remove(adjustedPicPath.length() - 7, 7);
    }

fileDialogPreSave:
    QFileInfo sgdFileInfo(adjustedPicPath);
    QFileDialog fileDialog(parent);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix(".rem");
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(PictureDialog::tr("Export as GTA Snapmatic..."));
    fileDialog.setLabelText(QFileDialog::Accept, PictureDialog::tr("&Export"));

    QStringList filters;
    filters << PictureDialog::tr("GTA V Export (*.g5e)");
    filters << PictureDialog::tr("GTA V Raw Export (*.auto)");
    filters << PictureDialog::tr("Snapmatic pictures (PGTA*)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value("Directory", StandardPaths::documentsLocation()).toString());
    fileDialog.selectFile(QString(picture->getExportPictureFileName() + ".g5e"));
    fileDialog.restoreGeometry(settings.value(parent->objectName() + "+Geomtery", "").toByteArray());


    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);

            if (QFile::exists(selectedFile))
            {
                if (QMessageBox::Yes == QMessageBox::warning(parent, PictureDialog::tr("Export as GTA Snapmatic"), PictureDialog::tr("Overwrite %1 with current Snapmatic picture?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                {
                    if (!QFile::remove(selectedFile))
                    {
                        QMessageBox::warning(parent, PictureDialog::tr("Export as GTA Snapmatic"), PictureDialog::tr("Failed to overwrite %1 with current Snapmatic picture").arg("\""+selectedFile+"\""));
                        goto fileDialogPreSave;
                    }
                }
                else
                {
                    goto fileDialogPreSave;
                }
            }

            if (selectedFile.right(4) == ".g5e")
            {
                bool isExported = picture->exportPicture(selectedFile, true);
                if (!isExported)
                {
                    QMessageBox::warning(parent, PictureDialog::tr("Export as GTA Snapmatic"), PictureDialog::tr("Failed to export current Snapmatic picture"));
                    goto fileDialogPreSave;
                }
            }
            else
            {
                bool isAutoExt = false;
                if (selectedFile.right(5) == ".auto")
                {
                    isAutoExt = true;
                    QString dirPath = QFileInfo(selectedFile).dir().path();
                    QString stockFileName = sgdFileInfo.fileName();
                    selectedFile = dirPath + "/" + stockFileName;
                }
                else if (selectedFile.right(4) == ".rem")
                {
                    selectedFile.remove(".rem");
                }
                bool isCopied = picture->exportPicture(selectedFile, false);
                if (!isCopied)
                {
                    QMessageBox::warning(parent, PictureDialog::tr("Export as GTA Snapmatic"), PictureDialog::tr("Failed to export current Snapmatic picture"));
                    goto fileDialogPreSave;
                }
                else
                {
                    if (isAutoExt) QMessageBox::information(parent, PictureDialog::tr("Export as GTA Snapmatic"), PictureDialog::tr("Exported Snapmatic to \"%1\" because of using the .auto extension.").arg(selectedFile));
                }
            }
        }
        else
        {
            QMessageBox::warning(parent, PictureDialog::tr("Export as GTA Snapmatic"), PictureDialog::tr("No valid file is selected"));
            goto fileDialogPreSave;
        }
    }

    settings.setValue(parent->objectName() + "+Geometry", fileDialog.saveGeometry());
    settings.setValue("Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
}
