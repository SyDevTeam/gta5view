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

#include "ProfileInterface.h"
#include "ui_ProfileInterface.h"
#include "SidebarGenerator.h"
#include "SnapmaticWidget.h"
#include "DatabaseThread.h"
#include "SavegameWidget.h"
#include "StandardPaths.h"
#include "ProfileLoader.h"
#include <QSpacerItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QFileInfo>
#include <QPalette>
#include <QRegExp>
#include <QDebug>
#include <QColor>
#include <QTimer>
#include <QFile>
#include <QUrl>
#include <QDir>

ProfileInterface::ProfileInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent) :
    QWidget(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB),
    ui(new Ui::ProfileInterface)
{
    ui->setupUi(this);
    ui->cmdImport->setEnabled(false);
    ui->cmdCloseProfile->setEnabled(false);
    loadingStr = ui->labProfileLoading->text();
    selectedWidgts = 0;
    profileFolder = "";
    profileLoader = 0;
    saSpacerItem = 0;

    QPalette palette;
    QColor baseColor = palette.base().color();
    ui->saProfile->setStyleSheet(QString("QWidget#saProfileContent{background-color: rgb(%1, %2, %3)}").arg(QString::number(baseColor.red()),QString::number(baseColor.green()),QString::number(baseColor.blue())));
}

ProfileInterface::~ProfileInterface()
{
    foreach(ProfileWidget *widget, widgets.keys())
    {
        widgets.remove(widget);
        widget->deleteLater();
        delete widget;
    }
    foreach(SavegameData *savegame, savegames)
    {
        savegames.removeAll(savegame);
        savegame->deleteLater();
        delete savegame;
    }
    foreach(SnapmaticPicture *picture, pictures)
    {
        pictures.removeAll(picture);
        picture->deleteLater();
        delete picture;
    }
    profileLoader->deleteLater();
    delete profileLoader;
    delete ui;
}

void ProfileInterface::setProfileFolder(QString folder, QString profile)
{
    profileFolder = folder;
    profileName = profile;
}

void ProfileInterface::setupProfileInterface()
{
    ui->labProfileLoading->setText(tr("Loading..."));
    profileLoader = new ProfileLoader(profileFolder, crewDB);
    QObject::connect(profileLoader, SIGNAL(savegameLoaded(SavegameData*, QString)), this, SLOT(savegameLoaded(SavegameData*, QString)));
    QObject::connect(profileLoader, SIGNAL(pictureLoaded(SnapmaticPicture*, QString)), this, SLOT(pictureLoaded(SnapmaticPicture*, QString)));
    QObject::connect(profileLoader, SIGNAL(loadingProgress(int,int)), this, SLOT(loadingProgress(int,int)));
    QObject::connect(profileLoader, SIGNAL(finished()), this, SLOT(profileLoaded_p()));
    profileLoader->start();
}

void ProfileInterface::savegameLoaded(SavegameData *savegame, QString savegamePath)
{
    SavegameWidget *sgdWidget = new SavegameWidget();
    sgdWidget->setSavegameData(savegame, savegamePath);
    ui->vlSavegame->addWidget(sgdWidget);
    widgets[sgdWidget] = "SavegameWidget";
    savegames.append(savegame);
    if (selectedWidgts != 0) { sgdWidget->setSelectionMode(true); }
    QObject::connect(sgdWidget, SIGNAL(savegameDeleted()), this, SLOT(savegameDeleted()));
    QObject::connect(sgdWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(sgdWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
}

void ProfileInterface::pictureLoaded(SnapmaticPicture *picture, QString picturePath)
{
    SnapmaticWidget *picWidget = new SnapmaticWidget(profileDB, threadDB);
    picWidget->setSnapmaticPicture(picture, picturePath);
    ui->vlSnapmatic->addWidget(picWidget);
    widgets[picWidget] = "SnapmaticWidget";
    pictures.append(picture);
    if (selectedWidgts != 0) { picWidget->setSelectionMode(true); }
    QObject::connect(picWidget, SIGNAL(pictureDeleted()), this, SLOT(pictureDeleted()));
    QObject::connect(picWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(picWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
}

void ProfileInterface::loadingProgress(int value, int maximum)
{
    ui->pbPictureLoading->setMaximum(maximum);
    ui->pbPictureLoading->setValue(value);
    ui->labProfileLoading->setText(loadingStr.arg(QString::number(value), QString::number(maximum)));
}

void ProfileInterface::profileLoaded_p()
{
    saSpacerItem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->saProfileContent->layout()->addItem(saSpacerItem);
    ui->swProfile->setCurrentWidget(ui->pageProfile);
    ui->cmdCloseProfile->setEnabled(true);
    ui->cmdImport->setEnabled(true);
    emit profileLoaded();
}

void ProfileInterface::savegameDeleted()
{
    SavegameWidget *sgdWidget = (SavegameWidget*)sender();
    SavegameData *savegame = sgdWidget->getSavegame();
    if (sgdWidget->isSelected()) { sgdWidget->setSelected(false); }
    sgdWidget->close();
    savegames.removeAll(savegame);
    delete savegame;
}

void ProfileInterface::pictureDeleted()
{
    SnapmaticWidget *picWidget = (SnapmaticWidget*)sender();
    SnapmaticPicture *picture = picWidget->getPicture();
    if (picWidget->isSelected()) { picWidget->setSelected(false); }
    picWidget->close();
    pictures.removeAll(picture);
    delete picture;
}

void ProfileInterface::on_cmdCloseProfile_clicked()
{
    emit profileClosed();
}

void ProfileInterface::on_cmdImport_clicked()
{
    QSettings settings("Syping", "gta5sync");
    settings.beginGroup("FileDialogs");

fileDialogPreOpen:
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog.setWindowTitle(tr("Import copy"));
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);

    QStringList filters;
    filters << tr("All profile files (SGTA* PGTA*)");
    filters << tr("Savegames files (SGTA*)");
    filters << tr("Snapmatic pictures (PGTA*)");
    filters << tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.restoreState(settings.value("ImportCopy","").toByteArray());

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            QFileInfo selectedFileInfo(selectedFile);
            QString selectedFileName = selectedFileInfo.fileName();
            if (QFile::exists(selectedFile))
            {
                if (selectedFileName.left(4) == "PGTA")
                {
                    SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
                    if (picture->readingPicture())
                    {
                        importSnapmaticPicture(picture, selectedFile);
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Import copy"), tr("Failed to read Snapmatic picture"));
                        picture->deleteLater();
                        delete picture;
                        goto fileDialogPreOpen;
                    }
                }
                else if (selectedFileName.left(4) == "SGTA")
                {
                    SavegameData *savegame = new SavegameData(selectedFile);
                    if (savegame->readingSavegame())
                    {
                        importSavegameData(savegame, selectedFile);
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Import copy"), tr("Failed to read Savegame file"));
                        savegame->deleteLater();
                        delete savegame;
                        goto fileDialogPreOpen;
                    }
                }
                else
                {
                    SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
                    SavegameData *savegame = new SavegameData(selectedFile);
                    if (picture->readingPicture())
                    {
                        importSnapmaticPicture(picture, selectedFile);
                        savegame->deleteLater();
                        delete savegame;
                    }
                    else if (savegame->readingSavegame())
                    {
                        importSavegameData(savegame, selectedFile);
                        picture->deleteLater();
                        delete picture;
                    }
                    else
                    {
                        savegame->deleteLater();
                        picture->deleteLater();
                        delete savegame;
                        delete picture;
                        QMessageBox::warning(this, tr("Import copy"), tr("Can't import %1 because of not valid file format").arg("\""+selectedFileName+"\""));
                        goto fileDialogPreOpen;
                    }
                }
            }
            else
            {
                QMessageBox::warning(this, tr("Import copy"), tr("No valid file is selected"));
                goto fileDialogPreOpen;
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Import copy"), tr("No valid file is selected"));
            goto fileDialogPreOpen;
        }
    }

    settings.setValue("ImportCopy", fileDialog.saveState());
    settings.endGroup();
}

bool ProfileInterface::importSnapmaticPicture(SnapmaticPicture *picture, QString picPath)
{
    QFileInfo picFileInfo(picPath);
    QString picFileName = picFileInfo.fileName();
    if (picFileName.left(4) != "PGTA")
    {
        QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Snapmatic picture because the file not begin with PGTA"));
        return false;
    }
    else if (QFile::copy(picPath, profileFolder + "/" + picFileName))
    {
        pictureLoaded(picture, profileFolder + "/" + picFileName);
        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Snapmatic picture because the copy failed"));
        return false;
    }
}

bool ProfileInterface::importSavegameData(SavegameData *savegame, QString sgdPath)
{
    QString sgdFileName;
    bool foundFree = 0;
    int currentSgd = 0;

    while (currentSgd < 15 && !foundFree)
    {
        QString sgdNumber = QString::number(currentSgd);
        if (sgdNumber.length() == 1)
        {
            sgdNumber.insert(0, "0");
        }
        sgdFileName = "SGTA500" + sgdNumber;

        if (!QFile::exists(profileFolder + "/" + sgdFileName))
        {
            foundFree = true;
        }
        currentSgd++;
    }

    if (foundFree)
    {
        if (QFile::copy(sgdPath, profileFolder + "/" + sgdFileName))
        {
            savegameLoaded(savegame, profileFolder + "/" + sgdFileName);
            return true;
        }
        else
        {
            QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Savegame file because the copy failed"));
            return false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Savegame file because no free Savegame slot left"));
        return false;
    }
}

void ProfileInterface::profileWidgetSelected()
{
    if (selectedWidgts == 0)
    {
        foreach(ProfileWidget *widget, widgets.keys())
        {
            widget->setSelectionMode(true);
        }
    }
    selectedWidgts++;
}

void ProfileInterface::profileWidgetDeselected()
{
    if (selectedWidgts == 1)
    {
        int scrollBarValue = ui->saProfile->verticalScrollBar()->value();
        foreach(ProfileWidget *widget, widgets.keys())
        {
            widget->setSelectionMode(false);
        }
        ui->saProfile->verticalScrollBar()->setValue(scrollBarValue);
    }
    selectedWidgts--;
}

void ProfileInterface::selectAllWidgets()
{
    foreach(ProfileWidget *widget, widgets.keys())
    {
        widget->setSelected(true);
    }
}

void ProfileInterface::deselectAllWidgets()
{
    foreach(ProfileWidget *widget, widgets.keys())
    {
        widget->setSelected(false);
    }
}
