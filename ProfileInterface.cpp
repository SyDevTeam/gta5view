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
#include "PictureDialog.h"
#include "PictureExport.h"
#include "StandardPaths.h"
#include "ProfileLoader.h"
#include "ExportThread.h"
#include "config.h"
#include <QProgressDialog>
#include <QProgressBar>
#include <QInputDialog>
#include <QPushButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QEventLoop>
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
    enabledPicStr = tr("Enabled pictures: %1 of %2");
    selectedWidgts = 0;
    profileFolder = "";
    profileLoader = 0;
    saSpacerItem = 0;

    QPalette palette;
    QColor baseColor = palette.base().color();
    ui->labVersion->setText(ui->labVersion->text().arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER));
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
    savegameLoaded_f(savegame, savegamePath, false);
}

void ProfileInterface::savegameLoaded_f(SavegameData *savegame, QString savegamePath, bool inserted)
{
    SavegameWidget *sgdWidget = new SavegameWidget(this);
    sgdWidget->setSavegameData(savegame, savegamePath);
    sgdWidget->setContentMode(contentMode);
    widgets[sgdWidget] = "SGD" + QFileInfo(savegamePath).fileName();
    savegames.append(savegame);
    if (selectedWidgts != 0 || contentMode == 2) { sgdWidget->setSelectionMode(true); }
    QObject::connect(sgdWidget, SIGNAL(savegameDeleted()), this, SLOT(savegameDeleted()));
    QObject::connect(sgdWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(sgdWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
    QObject::connect(sgdWidget, SIGNAL(allWidgetsSelected()), this, SLOT(selectAllWidgets()));
    QObject::connect(sgdWidget, SIGNAL(allWidgetsDeselected()), this, SLOT(deselectAllWidgets()));
    if (inserted) { insertSavegameIPI(sgdWidget); }
}

void ProfileInterface::pictureLoaded(SnapmaticPicture *picture, QString picturePath)
{
    pictureLoaded_f(picture, picturePath, false);
}

void ProfileInterface::pictureLoaded_f(SnapmaticPicture *picture, QString picturePath, bool inserted)
{
    SnapmaticWidget *picWidget = new SnapmaticWidget(profileDB, crewDB, threadDB, this);
    picWidget->setSnapmaticPicture(picture, picturePath);
    picWidget->setContentMode(contentMode);
    widgets[picWidget] = "PIC" + picture->getPictureSortStr();
    pictures.append(picture);
    if (selectedWidgts != 0 || contentMode == 2) { picWidget->setSelectionMode(true); }
    QObject::connect(picWidget, SIGNAL(pictureDeleted()), this, SLOT(pictureDeleted()));
    QObject::connect(picWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(picWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
    QObject::connect(picWidget, SIGNAL(allWidgetsSelected()), this, SLOT(selectAllWidgets()));
    QObject::connect(picWidget, SIGNAL(allWidgetsDeselected()), this, SLOT(deselectAllWidgets()));
    QObject::connect(picWidget, SIGNAL(nextPictureRequested(QWidget*)), this, SLOT(dialogNextPictureRequested(QWidget*)));
    QObject::connect(picWidget, SIGNAL(previousPictureRequested(QWidget*)), this, SLOT(dialogPreviousPictureRequested(QWidget*)));
    if (inserted) { insertSnapmaticIPI(picWidget); }
}

void ProfileInterface::loadingProgress(int value, int maximum)
{
    ui->pbPictureLoading->setMaximum(maximum);
    ui->pbPictureLoading->setValue(value);
    ui->labProfileLoading->setText(loadingStr.arg(QString::number(value), QString::number(maximum)));
}

void ProfileInterface::insertSnapmaticIPI(QWidget *widget)
{
    ProfileWidget *proWidget = (ProfileWidget*)widget;
    if (widgets.contains(proWidget))
    {
        QString widgetKey = widgets[proWidget];
        QStringList widgetsKeyList = widgets.values();
        QStringList pictureKeyList = widgetsKeyList.filter("PIC", Qt::CaseSensitive);
#if QT_VERSION >= 0x050600
        qSort(pictureKeyList.rbegin(), pictureKeyList.rend());
#else
        qSort(pictureKeyList.begin(), pictureKeyList.end(), qGreater<QString>());
#endif
        int picIndex = pictureKeyList.indexOf(QRegExp(widgetKey));
        ui->vlSnapmatic->insertWidget(picIndex, proWidget);

        qApp->processEvents();
        ui->saProfile->ensureWidgetVisible(proWidget, 0, 0);
    }
}

void ProfileInterface::insertSavegameIPI(QWidget *widget)
{
    ProfileWidget *proWidget = (ProfileWidget*)widget;
    if (widgets.contains(proWidget))
    {
        QString widgetKey = widgets[proWidget];
        QStringList widgetsKeyList = widgets.values();
        QStringList savegameKeyList = widgetsKeyList.filter("SGD", Qt::CaseSensitive);
        qSort(savegameKeyList.begin(), savegameKeyList.end());
        int sgdIndex = savegameKeyList.indexOf(QRegExp(widgetKey));
        ui->vlSavegame->insertWidget(sgdIndex, proWidget);

        qApp->processEvents();
        ui->saProfile->ensureWidgetVisible(proWidget, 0, 0);
    }
}

void ProfileInterface::dialogNextPictureRequested(QWidget *dialog)
{
    PictureDialog *picDialog = (PictureDialog*)dialog;
    ProfileWidget *proWidget = (ProfileWidget*)sender();
    if (widgets.contains(proWidget))
    {
        QString widgetKey = widgets[proWidget];
        QStringList widgetsKeyList = widgets.values();
        QStringList pictureKeyList = widgetsKeyList.filter("PIC", Qt::CaseSensitive);
#if QT_VERSION >= 0x050600
        qSort(pictureKeyList.rbegin(), pictureKeyList.rend());
#else
        qSort(pictureKeyList.begin(), pictureKeyList.end(), qGreater<QString>());
#endif
        int picIndex;
        if (picDialog->isIndexed())
        {
            picIndex = picDialog->getIndex();
        }
        else
        {
            picIndex = pictureKeyList.indexOf(QRegExp(widgetKey));
        }
        picIndex++;
        if (pictureKeyList.length() > picIndex)
        {
            QString newWidgetKey = pictureKeyList.at(picIndex);
            SnapmaticWidget *picWidget = (SnapmaticWidget*)widgets.key(newWidgetKey);
            //picDialog->setMaximumHeight(QWIDGETSIZE_MAX);
            picDialog->setSnapmaticPicture(picWidget->getPicture(), picIndex);
            //picDialog->setMaximumHeight(picDialog->height());
        }
    }
}

void ProfileInterface::dialogPreviousPictureRequested(QWidget *dialog)
{
    PictureDialog *picDialog = (PictureDialog*)dialog;
    ProfileWidget *proWidget = (ProfileWidget*)sender();
    if (widgets.contains(proWidget))
    {
        QString widgetKey = widgets[proWidget];
        QStringList widgetsKeyList = widgets.values();
        QStringList pictureKeyList = widgetsKeyList.filter("PIC", Qt::CaseSensitive);
#if QT_VERSION >= 0x050600
        qSort(pictureKeyList.rbegin(), pictureKeyList.rend());
#else
        qSort(pictureKeyList.begin(), pictureKeyList.end(), qGreater<QString>());
#endif
        int picIndex;
        if (picDialog->isIndexed())
        {
            picIndex = picDialog->getIndex();
        }
        else
        {
            picIndex = pictureKeyList.indexOf(QRegExp(widgetKey));
        }
        if (picIndex > 0)
        {
            picIndex--;
            QString newWidgetKey = pictureKeyList.at(picIndex );
            SnapmaticWidget *picWidget = (SnapmaticWidget*)widgets.key(newWidgetKey);
            //picDialog->setMaximumHeight(QWIDGETSIZE_MAX);
            picDialog->setSnapmaticPicture(picWidget->getPicture(), picIndex);
            //picDialog->setMaximumHeight(picDialog->height());
        }
    }
}

void ProfileInterface::sortingProfileInterface()
{
    ui->vlSavegame->setEnabled(false);
    ui->vlSnapmatic->setEnabled(false);

    QStringList widgetsKeyList = widgets.values();
    qSort(widgetsKeyList.begin(), widgetsKeyList.end());

    foreach(QString widgetKey, widgetsKeyList)
    {
        ProfileWidget *widget = widgets.key(widgetKey);
        if (widget->getWidgetType() == "SnapmaticWidget")
        {
            ui->vlSnapmatic->insertWidget(0, widget);
        }
        else if (widget->getWidgetType() == "SavegameWidget")
        {
            ui->vlSavegame->addWidget(widget);
        }
    }

    ui->vlSavegame->setEnabled(true);
    ui->vlSnapmatic->setEnabled(true);

    qApp->processEvents();
}

void ProfileInterface::profileLoaded_p()
{
    sortingProfileInterface();
    saSpacerItem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->saProfileContent->layout()->addItem(saSpacerItem);
    ui->swProfile->setCurrentWidget(ui->pageProfile);
    ui->cmdCloseProfile->setEnabled(true);
    ui->cmdImport->setEnabled(true);
    emit profileLoaded();
}

void ProfileInterface::savegameDeleted()
{
    savegameDeleted_f((SavegameWidget*)sender());
}

void ProfileInterface::savegameDeleted_f(QWidget *sgdWidget_)
{
    SavegameWidget *sgdWidget = (SavegameWidget*)sgdWidget_;
    SavegameData *savegame = sgdWidget->getSavegame();
    if (sgdWidget->isSelected()) { sgdWidget->setSelected(false); }
    widgets.remove(sgdWidget);
    sgdWidget->close();
    sgdWidget->deleteLater();
    savegames.removeAll(savegame);
    delete savegame;
}

void ProfileInterface::pictureDeleted()
{
    pictureDeleted_f((SnapmaticWidget*)sender());
}

void ProfileInterface::pictureDeleted_f(QWidget *picWidget_)
{
    SnapmaticWidget *picWidget = (SnapmaticWidget*)picWidget_;
    SnapmaticPicture *picture = picWidget->getPicture();
    if (picWidget->isSelected()) { picWidget->setSelected(false); }
    widgets.remove(picWidget);
    picWidget->close();
    picWidget->deleteLater();
    pictures.removeAll(picture);
    delete picture;
}

void ProfileInterface::on_cmdCloseProfile_clicked()
{
    emit profileClosed();
}

void ProfileInterface::on_cmdImport_clicked()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");
    settings.beginGroup("ImportCopy");

fileDialogPreOpen:
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(tr("Import..."));
    fileDialog.setLabelText(QFileDialog::Accept, tr("Import"));

    QStringList filters;
    filters << tr("All profile files (*.g5e SGTA* PGTA*)");
    filters << tr("GTA V Export (*.g5e)");
    filters << tr("Savegames files (SGTA*)");
    filters << tr("Snapmatic pictures (PGTA*)");
    filters << tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value(profileName + "+Directory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value(profileName + "+Geometry", "").toByteArray());

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            if (!importFile(selectedFile, true)) goto fileDialogPreOpen;
        }
        else if (selectedFiles.length() > 1)
        {
            QString errorStr;
            QStringList failedFiles;
            foreach(const QString &selectedFile, selectedFiles)
            {
                if (!importFile(selectedFile, false))
                {
                    failedFiles << QFileInfo(selectedFile).fileName();
                }
            }
            foreach (const QString &curErrorStr, failedFiles)
            {
                errorStr.append(", " + curErrorStr);
            }
            if (errorStr != "")
            {
                errorStr.remove(0, 2);
                QMessageBox::warning(this, tr("Import"), tr("Import failed with...\n\n%1").arg(errorStr));
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Import"), tr("No valid file is selected"));
            goto fileDialogPreOpen;
        }
    }

    settings.setValue(profileName + "+Geometry", fileDialog.saveGeometry());
    settings.setValue(profileName + "+Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}

bool ProfileInterface::importFile(QString selectedFile, bool warn)
{
    QString selectedFileName = QFileInfo(selectedFile).fileName();
    if (QFile::exists(selectedFile))
    {
        if (selectedFileName.left(4) == "PGTA" || selectedFileName.right(4) == ".g5e")
        {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            if (picture->readingPicture())
            {
                bool success = importSnapmaticPicture(picture, selectedFile, warn);
                if (!success) delete picture;
                return success;
            }
            else
            {
                if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to read Snapmatic picture"));
                picture->deleteLater();
                delete picture;
                return false;
            }
        }
        else if (selectedFileName.left(4) == "SGTA")
        {
            SavegameData *savegame = new SavegameData(selectedFile);
            if (savegame->readingSavegame())
            {
                bool success = importSavegameData(savegame, selectedFile, warn);
                if (!success) delete savegame;
                return success;
            }
            else
            {
                if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to read Savegame file"));
                savegame->deleteLater();
                delete savegame;
                return false;
            }
        }
        else
        {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            SavegameData *savegame = new SavegameData(selectedFile);
            if (picture->readingPicture())
            {
                bool success = importSnapmaticPicture(picture, selectedFile, warn);
                delete savegame;
                if (!success) delete picture;
                return success;
            }
            else if (savegame->readingSavegame())
            {
                bool success = importSavegameData(savegame, selectedFile, warn);
                delete picture;
                if (!success) delete savegame;
                return success;
            }
            else
            {
                savegame->deleteLater();
                picture->deleteLater();
                delete savegame;
                delete picture;
                if (warn) QMessageBox::warning(this, tr("Import"), tr("Can't import %1 because of not valid file format").arg("\""+selectedFileName+"\""));
                return false;
            }
        }
    }
    if (warn) QMessageBox::warning(this, tr("Import"), tr("No valid file is selected"));
    return false;
}

bool ProfileInterface::importSnapmaticPicture(SnapmaticPicture *picture, QString picPath, bool warn)
{
    QFileInfo picFileInfo(picPath);
    QString picFileName = picFileInfo.fileName();
    QString adjustedFileName = picFileName;
    if (adjustedFileName.right(4) == ".g5e")
    {
        adjustedFileName = picture->getPictureFileName();
    }
    if (adjustedFileName.right(7) == ".hidden") // for the hidden file system
    {
        adjustedFileName.remove(adjustedFileName.length() - 7, 7);
    }
    if (adjustedFileName.right(4) == ".bak") // for the backup file system
    {
        adjustedFileName.remove(adjustedFileName.length() - 4, 4);
    }
    if (picFileName.left(4) != "PGTA" && picFileName.right(4) != ".g5e")
    {
        if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to import the Snapmatic picture, file not begin with PGTA or end with .g5e"));
        return false;
    }
    else if (QFile::exists(profileFolder + QDir::separator() + adjustedFileName) || QFile::exists(profileFolder + QDir::separator() + adjustedFileName + ".hidden"))
    {
        if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to import the Snapmatic picture, the picture is already in the game"));
        return false;
    }
    else if (picture->exportPicture(profileFolder + QDir::separator() + adjustedFileName, false))
    {
        picture->setPicFileName(profileFolder + QDir::separator() + adjustedFileName);
        pictureLoaded_f(picture, profileFolder + QDir::separator() + adjustedFileName, true);
        return true;
    }
    else
    {
        if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to import the Snapmatic picture, can't copy the file into profile"));
        return false;
    }
}

bool ProfileInterface::importSavegameData(SavegameData *savegame, QString sgdPath, bool warn)
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

        if (!QFile::exists(profileFolder + QDir::separator() + sgdFileName))
        {
            foundFree = true;
        }
        currentSgd++;
    }

    if (foundFree)
    {
        if (QFile::copy(sgdPath, profileFolder + QDir::separator() + sgdFileName))
        {
            savegame->setSavegameFileName(profileFolder + QDir::separator() + sgdFileName);
            savegameLoaded_f(savegame, profileFolder + QDir::separator() + sgdFileName, true);
            return true;
        }
        else
        {
            if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to import the Savegame, can't copy the file into profile"));
            return false;
        }
    }
    else
    {
        if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to import the Savegame, no Savegame slot is left"));
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
            if (contentMode != 2)
            {
                widget->setSelectionMode(false);
            }
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

void ProfileInterface::exportSelected()
{
    if (selectedWidgts != 0)
    {
        int exportCount = 0;
        int exportPictures = 0;
        int exportSavegames = 0;
        bool pictureCopyEnabled = false;
        bool pictureExportEnabled = false;

        QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
        settings.beginGroup("FileDialogs");
        settings.beginGroup("ExportDirectory");
        QString exportDirectory = QFileDialog::getExistingDirectory(this, tr("Export selected"), settings.value(profileName, profileFolder).toString());
        if (exportDirectory != "")
        {
            settings.setValue(profileName, exportDirectory);
            foreach (ProfileWidget *widget, widgets.keys())
            {
                if (widget->isSelected())
                {
                    if (widget->getWidgetType() == "SnapmaticWidget")
                    {
                        exportPictures++;
                    }
                    else if (widget->getWidgetType() == "SavegameWidget")
                    {
                        exportSavegames++;
                    }
                }
            }

            if (exportPictures != 0)
            {
                QInputDialog inputDialog;
                QStringList inputDialogItems;
                inputDialogItems << tr("JPG pictures and GTA Snapmatic");
                inputDialogItems << tr("JPG pictures only");
                inputDialogItems << tr("GTA Snapmatic only");

                QString ExportPreSpan;
                QString ExportPostSpan;
#ifdef GTA5SYNC_WIN
                ExportPreSpan = "<span style=\"color: #003399; font-size: 12pt\">";
                ExportPostSpan = "</span>";
#else
                ExportPreSpan = "<span style=\"font-weight: bold\">";
                ExportPostSpan = "</span>";
#endif

                bool itemSelected = false;
                QString selectedItem = inputDialog.getItem(this, tr("Export selected"), tr("%1Export Snapmatic pictures%2<br><br>JPG pictures make it possible to open the picture with a Image Viewer<br>GTA Snapmatic make it possible to import the picture into the game<br><br>Export as:").arg(ExportPreSpan, ExportPostSpan), inputDialogItems, 0, false, &itemSelected, inputDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
                if (itemSelected)
                {
                    if (selectedItem == tr("JPG pictures and GTA Snapmatic"))
                    {
                        pictureExportEnabled = true;
                        pictureCopyEnabled = true;
                    }
                    else if (selectedItem == tr("JPG pictures only"))
                    {
                        pictureExportEnabled = true;
                    }
                    else if (selectedItem == tr("GTA Snapmatic only"))
                    {
                        pictureCopyEnabled = true;
                    }
                    else
                    {
                        pictureExportEnabled = true;
                        pictureCopyEnabled = true;
                    }
                }
                else
                {
                    pictureExportEnabled = true;
                    pictureCopyEnabled = true;
                }
            }

            // Counting the exports together
            exportCount = exportCount + exportSavegames;
            if (pictureExportEnabled && pictureCopyEnabled)
            {
                int exportPictures2 = exportPictures * 2;
                exportCount = exportCount + exportPictures2;
            }
            else
            {
                exportCount = exportCount + exportPictures;
            }

            QProgressDialog pbDialog(this);
            pbDialog.setWindowFlags(pbDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
            pbDialog.setWindowTitle(tr("Export selected..."));
            pbDialog.setLabelText(tr("Initializing export..."));
            pbDialog.setRange(0, exportCount);

            QList<QPushButton*> pbBtn = pbDialog.findChildren<QPushButton*>();
            pbBtn.at(0)->setDisabled(true);

            QList<QProgressBar*> pbBar = pbDialog.findChildren<QProgressBar*>();
            pbBar.at(0)->setTextVisible(false);

            ExportThread *exportThread = new ExportThread(widgets, exportDirectory, pictureCopyEnabled, pictureExportEnabled, exportCount);
            QObject::connect(exportThread, SIGNAL(exportStringUpdate(QString)), &pbDialog, SLOT(setLabelText(QString)));
            QObject::connect(exportThread, SIGNAL(exportProgressUpdate(int)), &pbDialog, SLOT(setValue(int)));
            QObject::connect(exportThread, SIGNAL(exportFinished()), &pbDialog, SLOT(close()));
            exportThread->start();

            pbDialog.exec();
            QStringList getFailedSavegames = exportThread->getFailedSavegames();
            QStringList getFailedCopyPictures = exportThread->getFailedCopyPictures();
            QStringList getFailedExportPictures = exportThread->getFailedExportPictures();

            QString errorStr;
            QStringList errorList;
            errorList << getFailedExportPictures;
            errorList << getFailedCopyPictures;
            errorList << getFailedSavegames;

            foreach (const QString &curErrorStr, errorList)
            {
                errorStr.append(", " + curErrorStr);
            }
            if (errorStr != "")
            {
                errorStr.remove(0, 2);
                QMessageBox::warning(this, tr("Export selected"), tr("Export failed with...\n\n%1").arg(errorStr));
            }

            if (exportThread->isFinished())
            {
                exportThread->deleteLater();
                delete exportThread;
            }
            else
            {
                QEventLoop threadFinishLoop;
                QObject::connect(exportThread, SIGNAL(finished()), &threadFinishLoop, SLOT(quit()));
                threadFinishLoop.exec();
                exportThread->deleteLater();
                delete exportThread;
            }
        }
        settings.endGroup();
        settings.endGroup();
    }
    else
    {
        QMessageBox::information(this, tr("Export selected"), tr("No Snapmatic pictures or Savegames files are selected"));
    }
}

void ProfileInterface::deleteSelected()
{
    if (selectedWidgts != 0)
    {
        if (QMessageBox::Yes == QMessageBox::warning(this, tr("Remove selected"), tr("You really want remove the selected Snapmatic picutres and Savegame files?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
        {
            foreach (ProfileWidget *widget, widgets.keys())
            {
                if (widget->isSelected())
                {
                    if (widget->getWidgetType() == "SnapmaticWidget")
                    {
                        SnapmaticWidget *picWidget = (SnapmaticWidget*)widget;
                        QString fileName = picWidget->getPicturePath();
                        if (!QFile::exists(fileName) || QFile::remove(fileName))
                        {
                            pictureDeleted_f(picWidget);
                        }
                    }
                    else if (widget->getWidgetType() == "SavegameWidget")
                    {
                        SavegameWidget *sgdWidget = (SavegameWidget*)widget;
                        SavegameData *savegame = sgdWidget->getSavegame();
                        QString fileName = savegame->getSavegameFileName();
                        if (!QFile::exists(fileName) || QFile::remove(fileName))
                        {
                            savegameDeleted_f(sgdWidget);
                        }
                    }
                }
            }
            if (selectedWidgts != 0)
            {
                QMessageBox::warning(this, tr("Remove selected"), tr("Failed at remove the complete selected Snapmatic pictures and/or Savegame files"));
            }
        }
    }
    else
    {
        QMessageBox::information(this, tr("Remove selected"), tr("No Snapmatic pictures or Savegames files are selected"));
    }
}

void ProfileInterface::importFiles()
{
    on_cmdImport_clicked();
}

void ProfileInterface::settingsApplied(int _contentMode, QString language)
{
    Q_UNUSED(language)
    contentMode = _contentMode;

    if (contentMode == 2)
    {
        foreach(ProfileWidget *widget, widgets.keys())
        {
            widget->setSelectionMode(true);
            widget->setContentMode(contentMode);
        }
    }
    else
    {
        foreach(ProfileWidget *widget, widgets.keys())
        {
            if (selectedWidgts == 0)
            {
                widget->setSelectionMode(false);
            }
            widget->setContentMode(contentMode);
        }
    }
}

void ProfileInterface::enableSelected()
{
    int fails = 0;
    foreach (ProfileWidget *widget, widgets.keys())
    {
        if (widget->isSelected())
        {
            if (widget->getWidgetType() == "SnapmaticWidget")
            {
                SnapmaticWidget *snapmaticWidget = (SnapmaticWidget*)widget;
                if (!snapmaticWidget->makePictureVisible())
                {
                    fails++;
                }
            }
        }
    }
}

void ProfileInterface::disableSelected()
{
    int fails = 0;
    foreach (ProfileWidget *widget, widgets.keys())
    {
        if (widget->isSelected())
        {
            if (widget->getWidgetType() == "SnapmaticWidget")
            {
                SnapmaticWidget *snapmaticWidget = (SnapmaticWidget*)widget;
                if (!snapmaticWidget->makePictureHidden())
                {
                    fails++;
                }
            }
        }
    }
}

int ProfileInterface::selectedWidgets()
{
    return selectedWidgts;
}
