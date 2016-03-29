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

#include "SavegameWidget.h"
#include "ui_SavegameWidget.h"
#include "StandardPaths.h"
#include "SavegameData.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QFile>
#include <QUrl>

SavegameWidget::SavegameWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SavegameWidget)
{
    ui->setupUi(this);
    sgdPath = "";
    sgdStr = "";
    sgdata = 0;
}

SavegameWidget::~SavegameWidget()
{
    delete ui;
}

void SavegameWidget::setSavegameData(SavegameData *savegame, QString savegamePath)
{
    ui->labSavegameStr->setText(savegame->getSavegameStr());
    sgdStr = savegame->getSavegameStr();
    sgdPath = savegamePath;
    sgdata = savegame;
}

void SavegameWidget::on_cmdDelete_clicked()
{
    int uchoice = QMessageBox::question(this, tr("Delete savegame"), tr("Are you sure to delete %1 from your savegames?").arg("\""+sgdStr+"\""), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
    if (uchoice == QMessageBox::Yes)
    {
        if (!QFile::exists(sgdPath))
        {
            emit savegameDeleted();
        }
        else if(QFile::remove(sgdPath))
        {
            emit savegameDeleted();
        }
        else
        {
            QMessageBox::warning(this, tr("Delete savegame"), tr("Failed at deleting %1 from your savegames").arg("\""+sgdStr+"\""));
        }
    }
}

void SavegameWidget::on_cmdCopy_clicked()
{
    QSettings settings("Syping", "gta5sync");
    settings.beginGroup("FileDialogs");

fileDialogPreSave:
    QFileInfo sgdFileInfo(sgdPath);
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix("");
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(tr("Copy savegame"));

    QStringList filters;
    filters << tr("Savegame files (SGTA*)");
    filters << tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = fileDialog.sidebarUrls();
    QDir dir;

    // Get Documents + Desktop Location
    QString documentsLocation = StandardPaths::documentsLocation();
    QString desktopLocation = StandardPaths::desktopLocation();

    // Add Desktop Location to Sidebar
    dir.setPath(desktopLocation);
    if (dir.exists())
    {
        sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
    }

    // Add Documents + GTA V Location to Sidebar
    dir.setPath(documentsLocation);
    if (dir.exists())
    {
        sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
        if (dir.cd("Rockstar Games/GTA V"))
        {
            sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
        }
    }

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.restoreState(settings.value("CopySavegame","").toByteArray());
    fileDialog.selectFile(sgdFileInfo.fileName());

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);

            if (QFile::exists(selectedFile))
            {
                if (QMessageBox::Yes == QMessageBox::warning(this, tr("Copy savegame"), tr("Overwrite %1 with current savegame?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                {
                    if (!QFile::remove(selectedFile))
                    {
                        QMessageBox::warning(this, tr("Copy savegame"), tr("Failed to overwrite %1 with current savegame").arg("\""+selectedFile+"\""));
                        goto fileDialogPreSave;
                    }
                }
                else
                {
                    goto fileDialogPreSave;
                }
            }

            bool isCopied = QFile::copy(sgdPath, selectedFile);
            if (!isCopied)
            {
                QMessageBox::warning(this, tr("Copy savegame"), tr("Failed to copy current savegame"));
                goto fileDialogPreSave;
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Copy savegame"), tr("No valid file is selected"));
            goto fileDialogPreSave;
        }
    }

    settings.setValue("CopySavegame", fileDialog.saveState());
    settings.endGroup();
}
