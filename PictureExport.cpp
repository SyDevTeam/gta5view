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

#include "PictureExport.h"
#include "PictureDialog.h"
#include "SidebarGenerator.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>

PictureExport::PictureExport()
{

}

void PictureExport::exportPicture(QWidget *parent, SnapmaticPicture *picture)
{
    QSettings settings("Syping", "gta5sync");
    settings.beginGroup("FileDialogs");

fileDialogPreSave:
    QFileDialog fileDialog(parent);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix("suffix");
    fileDialog.setWindowTitle(PictureDialog::tr("Export picture"));
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);

    QStringList filters;
    filters << PictureDialog::tr("JPEG picture (*.jpg)");
    filters << PictureDialog::tr("Portable Network Graphics (*.png)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.restoreState(settings.value("ExportPicture","").toByteArray());

    if (picture != 0)
    {
        QString newPictureFileName = getPictureFileName(picture);
        fileDialog.selectFile(newPictureFileName);
    }

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString saveFileFormat;
            QString selectedFile = selectedFiles.at(0);

            if (selectedFile.right(4) == ".jpg")
            {
                saveFileFormat = "JPEG";
            }
            else if (selectedFile.right(4) == ".jpeg")
            {
                saveFileFormat = "JPEG";
            }
            else if (selectedFile.right(4) == ".png")
            {
                saveFileFormat = "PNG";
            }
            else if (selectedFile.right(7) == ".suffix")
            {
                if (fileDialog.selectedNameFilter() == "JPEG picture (*.jpg)")
                {
                    selectedFile.replace(".suffix", ".jpg");
                }
                else if (fileDialog.selectedNameFilter() == "Portable Network Graphics (*.png)")
                {
                    selectedFile.replace(".suffix", ".png");
                }
                else
                {
                    selectedFile.replace(".suffix", ".jpg");
                }
            }

            if (QFile::exists(selectedFile))
            {
                if (QMessageBox::Yes == QMessageBox::warning(parent, PictureDialog::tr("Export picture"), PictureDialog::tr("Overwrite %1 with current Snapmatic picture?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                {
                    if (!QFile::remove(selectedFile))
                    {
                        QMessageBox::warning(parent, PictureDialog::tr("Export picture"), PictureDialog::tr("Failed to overwrite %1 with current Snapmatic picture").arg("\""+selectedFile+"\""));
                        goto fileDialogPreSave;
                    }
                }
                else
                {
                    goto fileDialogPreSave;
                }
            }

            bool isSaved = picture->getPicture().save(selectedFile, saveFileFormat.toStdString().c_str(), 100);

            if (!isSaved)
            {
                QMessageBox::warning(parent, PictureDialog::tr("Export picture"), PictureDialog::tr("Failed to export current Snapmatic picture"));
                goto fileDialogPreSave;
            }
        }
        else
        {
            QMessageBox::warning(parent, PictureDialog::tr("Export picture"), PictureDialog::tr("No valid file is selected"));
            goto fileDialogPreSave;
        }
    }

    settings.setValue("ExportPicture", fileDialog.saveState());
    settings.endGroup();
}

QString PictureExport::getPictureFileName(SnapmaticPicture *picture)
{
    QString newPictureFileName;
    QString pictureStr = picture->getPictureStr();
    QStringList pictureStrList = pictureStr.split(" - ");
    if (pictureStrList.length() <= 2)
    {
        QString dtStr = pictureStrList.at(1);
        QStringList dtStrList = dtStr.split(" ");
        if (dtStrList.length() <= 2)
        {
            QString dayStr;
            QString yearStr;
            QString monthStr;
            QString dateStr = dtStrList.at(0);
            QString timeStr = dtStrList.at(1);
            timeStr.replace(":","");
            QStringList dateStrList = dateStr.split("/");
            if (dateStrList.length() <= 3)
            {
                dayStr = dateStrList.at(1);
                yearStr = dateStrList.at(2);
                monthStr = dateStrList.at(0);
            }
            QString cmpPicTitl = picture->getPictureTitl();
            cmpPicTitl.replace(" ", "_");
            newPictureFileName = yearStr + monthStr + dayStr + timeStr + "_" + cmpPicTitl +  ".jpg";
        }
    }
    return newPictureFileName;
}
