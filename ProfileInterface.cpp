/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
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
#include "ImportDialog.h"
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
#include <QPainter>
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

    if (QIcon::hasThemeIcon("dialog-close"))
    {
        ui->cmdCloseProfile->setIcon(QIcon::fromTheme("dialog-close"));
    }
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
    QObject::connect(profileLoader, SIGNAL(savegameLoaded(SavegameData*, QString)), this, SLOT(savegameLoaded_event(SavegameData*, QString)));
    QObject::connect(profileLoader, SIGNAL(pictureLoaded(SnapmaticPicture*)), this, SLOT(pictureLoaded_event(SnapmaticPicture*)));
    QObject::connect(profileLoader, SIGNAL(loadingProgress(int,int)), this, SLOT(loadingProgress(int,int)));
    QObject::connect(profileLoader, SIGNAL(finished()), this, SLOT(profileLoaded_p()));
    profileLoader->start();
}

void ProfileInterface::savegameLoaded_event(SavegameData *savegame, QString savegamePath)
{
    savegameLoaded(savegame, savegamePath, false);
}

void ProfileInterface::savegameLoaded(SavegameData *savegame, QString savegamePath, bool inserted)
{
    SavegameWidget *sgdWidget = new SavegameWidget(this);
    sgdWidget->setSavegameData(savegame, savegamePath);
    sgdWidget->setContentMode(contentMode);
    widgets[sgdWidget] = "SGD" + QFileInfo(savegamePath).fileName();
    savegames.append(savegame);
    if (selectedWidgts != 0 || contentMode == 2) { sgdWidget->setSelectionMode(true); }
    QObject::connect(sgdWidget, SIGNAL(savegameDeleted()), this, SLOT(savegameDeleted_event()));
    QObject::connect(sgdWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(sgdWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
    QObject::connect(sgdWidget, SIGNAL(allWidgetsSelected()), this, SLOT(selectAllWidgets()));
    QObject::connect(sgdWidget, SIGNAL(allWidgetsDeselected()), this, SLOT(deselectAllWidgets()));
    QObject::connect(sgdWidget, SIGNAL(contextMenuTriggered(QContextMenuEvent*)), this, SLOT(contextMenuTriggeredSGD(QContextMenuEvent*)));
    if (inserted) { insertSavegameIPI(sgdWidget); }
}

void ProfileInterface::pictureLoaded_event(SnapmaticPicture *picture)
{
    pictureLoaded(picture, false);
}

void ProfileInterface::pictureLoaded(SnapmaticPicture *picture, bool inserted)
{
    SnapmaticWidget *picWidget = new SnapmaticWidget(profileDB, crewDB, threadDB, this);
    picWidget->setSnapmaticPicture(picture);
    picWidget->setContentMode(contentMode);
    widgets[picWidget] = "PIC" + picture->getPictureSortStr();
    pictures.append(picture);
    if (selectedWidgts != 0 || contentMode == 2) { picWidget->setSelectionMode(true); }
    QObject::connect(picWidget, SIGNAL(pictureDeleted()), this, SLOT(pictureDeleted_event()));
    QObject::connect(picWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(picWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
    QObject::connect(picWidget, SIGNAL(allWidgetsSelected()), this, SLOT(selectAllWidgets()));
    QObject::connect(picWidget, SIGNAL(allWidgetsDeselected()), this, SLOT(deselectAllWidgets()));
    QObject::connect(picWidget, SIGNAL(nextPictureRequested(QWidget*)), this, SLOT(dialogNextPictureRequested(QWidget*)));
    QObject::connect(picWidget, SIGNAL(previousPictureRequested(QWidget*)), this, SLOT(dialogPreviousPictureRequested(QWidget*)));
    QObject::connect(picWidget, SIGNAL(contextMenuTriggered(QContextMenuEvent*)), this, SLOT(contextMenuTriggeredPIC(QContextMenuEvent*)));
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

void ProfileInterface::savegameDeleted_event()
{
    savegameDeleted((SavegameWidget*)sender());
}

void ProfileInterface::savegameDeleted(SavegameWidget *sgdWidget)
{
    SavegameData *savegame = sgdWidget->getSavegame();
    if (sgdWidget->isSelected()) { sgdWidget->setSelected(false); }
    widgets.remove(sgdWidget);
    sgdWidget->close();
    sgdWidget->deleteLater();
    savegames.removeAll(savegame);
    delete savegame;
}

void ProfileInterface::pictureDeleted_event()
{
    pictureDeleted((SnapmaticWidget*)sender());
}

void ProfileInterface::pictureDeleted(SnapmaticWidget *picWidget)
{
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
    filters << tr("Importable files (*.g5e *.jpg *.png SGTA* PGTA*)");
    filters << tr("GTA V Export (*.g5e)");
    filters << tr("Savegames files (SGTA*)");
    filters << tr("Snapmatic pictures (PGTA*)");
    filters << tr("All image files (*.jpg *.png)");
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
            if (!importFile(selectedFile, true, 0)) goto fileDialogPreOpen;
        }
        else if (selectedFiles.length() > 1)
        {
            int maximumId = selectedFiles.length();
            int overallId = 1;
            int currentId = 0;
            QString errorStr;
            QStringList failedFiles;

            // Progress dialog
            QProgressDialog pbDialog(this);
            pbDialog.setWindowFlags(pbDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
            pbDialog.setWindowTitle(tr("Import..."));
            pbDialog.setLabelText(tr("Import file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));
            pbDialog.setRange(1, maximumId);
            pbDialog.setValue(1);
            pbDialog.setModal(true);
            QList<QPushButton*> pbBtn = pbDialog.findChildren<QPushButton*>();
            pbBtn.at(0)->setDisabled(true);
            QList<QProgressBar*> pbBar = pbDialog.findChildren<QProgressBar*>();
            pbBar.at(0)->setTextVisible(false);
            pbDialog.show();

            QTime t;
            t.start();
            foreach(const QString &selectedFile, selectedFiles)
            {
                pbDialog.setValue(overallId);
                pbDialog.setLabelText(tr("Import file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));
                if (currentId == 10)
                {
                    // Break until two seconds are over (this prevent import failures)
                    int elapsedTime = t.elapsed();
                    if (elapsedTime > 2000)
                    {
                    }
                    else if (elapsedTime < 0)
                    {
                        QEventLoop loop;
                        QTimer::singleShot(2000, &loop, SLOT(quit()));
                        loop.exec();
                    }
                    else
                    {
                        QEventLoop loop;
                        QTimer::singleShot(2000 - elapsedTime, &loop, SLOT(quit()));
                        loop.exec();
                    }
                    currentId = 0;
                    t.restart();
                }
                if (!importFile(selectedFile, false, currentId))
                {
                    failedFiles << QFileInfo(selectedFile).fileName();
                }
                overallId++;
                currentId++;
            }
            pbDialog.close();
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

bool ProfileInterface::importFile(QString selectedFile, bool notMultiple, int currentId)
{
    QString selectedFileName = QFileInfo(selectedFile).fileName();
    if (QFile::exists(selectedFile))
    {
        if (selectedFileName.left(4) == "PGTA" || selectedFileName.right(4) == ".g5e")
        {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            if (picture->readingPicture())
            {
                bool success = importSnapmaticPicture(picture, notMultiple);
                if (!success) delete picture;
                return success;
            }
            else
            {
                if (notMultiple) QMessageBox::warning(this, tr("Import"), tr("Failed to read Snapmatic picture"));
                delete picture;
                return false;
            }
        }
        else if (selectedFileName.left(4) == "SGTA")
        {
            SavegameData *savegame = new SavegameData(selectedFile);
            if (savegame->readingSavegame())
            {
                bool success = importSavegameData(savegame, selectedFile, notMultiple);
                if (!success) delete savegame;
                return success;
            }
            else
            {
                if (notMultiple) QMessageBox::warning(this, tr("Import"), tr("Failed to read Savegame file"));
                delete savegame;
                return false;
            }
        }
        else if(selectedFileName.right(4) == ".jpg" || selectedFileName.right(4) == ".png")
        {
            SnapmaticPicture *picture = new SnapmaticPicture(":/template/template.g5e");
            if (picture->readingPicture(true, false))
            {
                if (!notMultiple)
                {
                    QImage snapmaticImage;
                    QString customImageTitle;
                    QPixmap snapmaticPixmap(960, 536);
                    snapmaticPixmap.fill(Qt::black);
                    QPainter snapmaticPainter(&snapmaticPixmap);
                    if (!snapmaticImage.load(selectedFile))
                    {
                        delete picture;
                        return false;
                    }
                    if (snapmaticImage.height() == snapmaticImage.width())
                    {
                        // Avatar mode
                        int diffWidth = 0;
                        int diffHeight = 0;
                        snapmaticImage = snapmaticImage.scaled(470, 470, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        if (snapmaticImage.width() > snapmaticImage.height())
                        {
                            diffHeight = 470 - snapmaticImage.height();
                            diffHeight = diffHeight / 2;
                        }
                        else if (snapmaticImage.width() < snapmaticImage.height())
                        {
                            diffWidth = 470 - snapmaticImage.width();
                            diffWidth = diffWidth / 2;
                        }
                        snapmaticPainter.drawImage(145 + diffWidth, 66 + diffHeight, snapmaticImage);
                        customImageTitle = "Custom Avatar";
                    }
                    else
                    {
                        // Picture mode
                        int diffWidth = 0;
                        int diffHeight = 0;
                        snapmaticImage = snapmaticImage.scaled(960, 536, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        if (snapmaticImage.width() != 960)
                        {
                            diffWidth = 960 - snapmaticImage.width();
                            diffWidth = diffWidth / 2;
                        }
                        else if (snapmaticImage.height() != 536)
                        {
                            diffHeight = 536 - snapmaticImage.height();
                            diffHeight = diffHeight / 2;
                        }
                        snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, snapmaticImage);
                        customImageTitle = "Custom Picture";
                    }
                    snapmaticPainter.end();
                    if (!picture->setImage(snapmaticPixmap.toImage()))
                    {
                        delete picture;
                        return false;
                    }
                    SnapmaticProperties spJson = picture->getSnapmaticProperties();
                    spJson.uid = QString(QTime::currentTime().toString("HHmmss") +
                                         QString::number(currentId) +
                                         QString::number(QDate::currentDate().dayOfYear())).toInt();
                    spJson.createdDateTime = QDateTime::currentDateTime();
                    spJson.createdTimestamp = spJson.createdDateTime.toTime_t();
                    picture->setSnapmaticProperties(spJson);
                    picture->setPicFileName(QString("PGTA5%1").arg(QString::number(spJson.uid)));
                    picture->setPictureTitle(customImageTitle);
                    picture->updateStrings();
                    bool success = importSnapmaticPicture(picture, notMultiple);
                    if (!success) delete picture;
                    return success;
                }
                else
                {
                    bool success = false;
                    QImage snapmaticImage;
                    if (!snapmaticImage.load(selectedFile))
                    {
                        delete picture;
                        return false;
                    }
                    ImportDialog *importDialog = new ImportDialog(this);
                    importDialog->setWindowFlags(importDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
                    importDialog->setImage(snapmaticImage);
                    importDialog->setModal(true);
                    importDialog->show();
                    importDialog->exec();
                    if (importDialog->isDoImport())
                    {
                        if (picture->setImage(importDialog->image()))
                        {
                            SnapmaticProperties spJson = picture->getSnapmaticProperties();
                            spJson.uid = QString(QTime::currentTime().toString("HHmmss") +
                                                 QString::number(currentId) +
                                                 QString::number(QDate::currentDate().dayOfYear())).toInt();
                            spJson.createdDateTime = QDateTime::currentDateTime();
                            spJson.createdTimestamp = spJson.createdDateTime.toTime_t();
                            picture->setSnapmaticProperties(spJson);
                            picture->setPicFileName(QString("PGTA5%1").arg(QString::number(spJson.uid)));
                            picture->setPictureTitle(importDialog->getImageTitle());
                            picture->updateStrings();
                            success = importSnapmaticPicture(picture, notMultiple);
                        }
                    }
                    else
                    {
                        delete picture;
                        success = true;
                    }
                    delete importDialog;
                    if (!success) delete picture;
                    return success;
                }
            }
            else
            {
                delete picture;
                return false;
            }
        }
        else
        {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            SavegameData *savegame = new SavegameData(selectedFile);
            if (picture->readingPicture())
            {
                bool success = importSnapmaticPicture(picture, notMultiple);
                delete savegame;
                if (!success) delete picture;
                return success;
            }
            else if (savegame->readingSavegame())
            {
                bool success = importSavegameData(savegame, selectedFile, notMultiple);
                delete picture;
                if (!success) delete savegame;
                return success;
            }
            else
            {
                delete savegame;
                delete picture;
                if (notMultiple) QMessageBox::warning(this, tr("Import"), tr("Can't import %1 because of not valid file format").arg("\""+selectedFileName+"\""));
                return false;
            }
        }
    }
    if (notMultiple) QMessageBox::warning(this, tr("Import"), tr("No valid file is selected"));
    return false;
}

bool ProfileInterface::importSnapmaticPicture(SnapmaticPicture *picture, bool warn)
{
    QString picFileName = picture->getPictureFileName();
    QString adjustedFileName = picFileName;
    if (adjustedFileName.right(7) == ".hidden") // for the hidden file system
    {
        adjustedFileName.remove(adjustedFileName.length() - 7, 7);
    }
    if (adjustedFileName.right(4) == ".bak") // for the backup file system
    {
        adjustedFileName.remove(adjustedFileName.length() - 4, 4);
    }
    if (picFileName.left(4) != "PGTA")
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
        picture->setPicFilePath(profileFolder + QDir::separator() + adjustedFileName);
        pictureLoaded(picture, true);
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
            savegameLoaded(savegame, profileFolder + QDir::separator() + sgdFileName, true);
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
                            pictureDeleted(picWidget);
                        }
                    }
                    else if (widget->getWidgetType() == "SavegameWidget")
                    {
                        SavegameWidget *sgdWidget = (SavegameWidget*)widget;
                        SavegameData *savegame = sgdWidget->getSavegame();
                        QString fileName = savegame->getSavegameFileName();
                        if (!QFile::exists(fileName) || QFile::remove(fileName))
                        {
                            savegameDeleted(sgdWidget);
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

void ProfileInterface::contextMenuTriggeredPIC(QContextMenuEvent *ev)
{
    SnapmaticWidget *picWidget = (SnapmaticWidget*)sender();
    QMenu contextMenu(picWidget);
    QMenu editMenu(SnapmaticWidget::tr("Edi&t"), picWidget);
    if (picWidget->isHidden())
    {
        editMenu.addAction(SnapmaticWidget::tr("Show &In-game"), picWidget, SLOT(makePictureVisibleSlot()));
    }
    else
    {
        editMenu.addAction(SnapmaticWidget::tr("Hide &In-game"), picWidget, SLOT(makePictureHiddenSlot()));
    }
    editMenu.addAction(SnapmaticWidget::tr("&Edit Properties..."), picWidget, SLOT(editSnapmaticProperties()));
    QMenu exportMenu(SnapmaticWidget::tr("&Export"), this);
    exportMenu.addAction(SnapmaticWidget::tr("Export as &JPG picture..."), picWidget, SLOT(on_cmdExport_clicked()));
    exportMenu.addAction(SnapmaticWidget::tr("Export as &GTA Snapmatic..."), picWidget, SLOT(on_cmdCopy_clicked()));
    contextMenu.addAction(SnapmaticWidget::tr("&View"), picWidget, SLOT(on_cmdView_clicked()));
    contextMenu.addMenu(&editMenu);
    contextMenu.addMenu(&exportMenu);
    contextMenu.addAction(SnapmaticWidget::tr("&Remove"), picWidget, SLOT(on_cmdDelete_clicked()));
    if (picWidget->isSelected())
    {
        contextMenu.addSeparator();
        if (!picWidget->isSelected()) { contextMenu.addAction(SnapmaticWidget::tr("&Select"), picWidget, SLOT(pictureSelected())); }
        if (picWidget->isSelected()) { contextMenu.addAction(SnapmaticWidget::tr("&Deselect"), picWidget, SLOT(pictureSelected())); }
        contextMenu.addAction(SnapmaticWidget::tr("Select &All"), picWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
        if (selectedWidgets() != 0)
        {
            contextMenu.addAction(SnapmaticWidget::tr("&Deselect All"), picWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
        }
    }
    else
    {
        contextMenu.addSeparator();
        contextMenu.addAction(SnapmaticWidget::tr("&Select"), picWidget, SLOT(pictureSelected()));
        contextMenu.addAction(SnapmaticWidget::tr("Select &All"), picWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
    }
    contextMenu.exec(ev->globalPos());
}

void ProfileInterface::contextMenuTriggeredSGD(QContextMenuEvent *ev)
{
    SavegameWidget *sgdWidget = (SavegameWidget*)sender();
    QMenu contextMenu(sgdWidget);
    contextMenu.addAction(SavegameWidget::tr("&View"), sgdWidget, SLOT(on_cmdView_clicked()));
    contextMenu.addAction(SavegameWidget::tr("&Export"), sgdWidget, SLOT(on_cmdCopy_clicked()));
    contextMenu.addAction(SavegameWidget::tr("&Remove"), sgdWidget, SLOT(on_cmdDelete_clicked()));
    if (sgdWidget->isSelected())
    {
        contextMenu.addSeparator();
        if (!sgdWidget->isSelected()) { contextMenu.addAction(SavegameWidget::tr("&Select"), this, SLOT(savegameSelected())); }
        if (sgdWidget->isSelected()) { contextMenu.addAction(SavegameWidget::tr("&Deselect"), this, SLOT(savegameSelected())); }
        contextMenu.addAction(SavegameWidget::tr("Select &All"), sgdWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
        if (selectedWidgets() != 0)
        {
            contextMenu.addAction(SavegameWidget::tr("&Deselect All"), sgdWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
        }
    }
    else
    {
        contextMenu.addSeparator();
        contextMenu.addAction(SavegameWidget::tr("&Select"), sgdWidget, SLOT(savegameSelected()));
        contextMenu.addAction(SavegameWidget::tr("Select &All"), sgdWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
    }
    contextMenu.exec(ev->globalPos());
}
