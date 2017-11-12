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
#include "AppEnv.h"
#include "config.h"
#include <QProgressDialog>
#include <QStringBuilder>
#include <QImageReader>
#include <QProgressBar>
#include <QInputDialog>
#include <QPushButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QEventLoop>
#include <QScrollBar>
#include <QFileInfo>
#include <QPalette>
#include <QPainter>
#include <QRegExp>
#include <QAction>
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
    contextMenuOpened = false;
    isProfileLoaded = false;
    previousWidget = nullptr;
    profileLoader = nullptr;
    saSpacerItem = nullptr;

    updatePalette();
    ui->labVersion->setText(QString("%1 %2").arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER));
    ui->saProfileContent->setFilesMode(true);

    if (QIcon::hasThemeIcon("dialog-close"))
    {
        ui->cmdCloseProfile->setIcon(QIcon::fromTheme("dialog-close"));
    }

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
#ifndef Q_OS_MAC
    ui->hlButtons->setSpacing(6 * screenRatio);
    ui->hlButtons->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
#else
    ui->hlButtons->setSpacing(6 * screenRatio);
    ui->hlButtons->setContentsMargins(9 * screenRatio, 15 * screenRatio, 15 * screenRatio, 17 * screenRatio);
#endif

    setMouseTracking(true);
    installEventFilter(this);
}

ProfileInterface::~ProfileInterface()
{
    for (ProfileWidget *widget : widgets.keys())
    {
        widget->removeEventFilter(this);
        widget->disconnect();
        delete widget;
    }
    widgets.clear();

    for (SavegameData *savegame : savegames)
    {
        delete savegame;
    }
    savegames.clear();

    for (SnapmaticPicture *picture : pictures)
    {
        delete picture;
    }
    pictures.clear();

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
    sgdWidget->setMouseTracking(true);
    sgdWidget->installEventFilter(this);
    widgets[sgdWidget] = "SGD" % QFileInfo(savegamePath).fileName();
    savegames += savegame;
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
    picWidget->setMouseTracking(true);
    picWidget->installEventFilter(this);
    widgets[picWidget] = "PIC" % picture->getPictureSortStr();
    pictures += picture;
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
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(widget);
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
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(widget);
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
    PictureDialog *picDialog = qobject_cast<PictureDialog*>(dialog);
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(sender());
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
    PictureDialog *picDialog = qobject_cast<PictureDialog*>(dialog);
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(sender());
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

    for (QString widgetKey : widgetsKeyList)
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
    isProfileLoaded = true;
    emit profileLoaded();
}

void ProfileInterface::savegameDeleted_event()
{
    savegameDeleted(qobject_cast<SavegameWidget*>(sender()), true);
}

void ProfileInterface::savegameDeleted(SavegameWidget *sgdWidget, bool isRemoteEmited)
{
    SavegameData *savegame = sgdWidget->getSavegame();
    if (sgdWidget->isSelected()) { sgdWidget->setSelected(false); }
    widgets.remove(sgdWidget);

    sgdWidget->disconnect();
    sgdWidget->removeEventFilter(this);
    if (sgdWidget == previousWidget)
    {
        previousWidget = nullptr;
    }

    // Deleting when the widget did send a event cause a crash
    isRemoteEmited ? sgdWidget->deleteLater() : delete sgdWidget;

    savegames.removeAll(savegame);
    delete savegame;
}

void ProfileInterface::pictureDeleted_event()
{
    pictureDeleted(qobject_cast<SnapmaticWidget*>(sender()), true);
}

void ProfileInterface::pictureDeleted(SnapmaticWidget *picWidget, bool isRemoteEmited)
{
    SnapmaticPicture *picture = picWidget->getPicture();
    if (picWidget->isSelected()) { picWidget->setSelected(false); }
    widgets.remove(picWidget);

    picWidget->disconnect();
    picWidget->removeEventFilter(this);
    if (picWidget == previousWidget)
    {
        previousWidget = nullptr;
    }

    // Deleting when the widget did send a event cause a crash
    isRemoteEmited ? picWidget->deleteLater() : delete picWidget;

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
    bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
    settings.beginGroup("ImportCopy");

fileDialogPreOpen: //Work?
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, dontUseNativeDialog);
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(tr("Import..."));
    fileDialog.setLabelText(QFileDialog::Accept, tr("Import"));

    // Getting readable Image formats
    QString imageFormatsStr = " ";
    for (QByteArray imageFormat : QImageReader::supportedImageFormats())
    {
        imageFormatsStr += QString("*.") % QString::fromUtf8(imageFormat).toLower() % " ";
    }
    QString importableFormatsStr = QString("*.g5e SGTA* PGTA*");
    if (!imageFormatsStr.trimmed().isEmpty())
    {
        importableFormatsStr = QString("*.g5e%1SGTA* PGTA*").arg(imageFormatsStr);
    }

    QStringList filters;
    filters << tr("Importable files (%1)").arg(importableFormatsStr);
    filters << tr("GTA V Export (*.g5e)");
    filters << tr("Savegames files (SGTA*)");
    filters << tr("Snapmatic pictures (PGTA*)");
    filters << tr("All image files (%1)").arg(imageFormatsStr.trimmed());
    filters << tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value(profileName % "+Directory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value(profileName % "+Geometry", "").toByteArray());

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            if (!importFile(selectedFile, true)) goto fileDialogPreOpen; //Work?
        }
        else if (selectedFiles.length() > 1)
        {
            importFilesProgress(selectedFiles);
        }
        else
        {
            QMessageBox::warning(this, tr("Import"), tr("No valid file is selected"));
            goto fileDialogPreOpen; //Work?
        }
    }

    settings.setValue(profileName % "+Geometry", fileDialog.saveGeometry());
    settings.setValue(profileName % "+Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}

void ProfileInterface::importFilesProgress(QStringList selectedFiles)
{
    int maximumId = selectedFiles.length();
    int overallId = 1;
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
    for (QString selectedFile : selectedFiles)
    {
        pbDialog.setValue(overallId);
        pbDialog.setLabelText(tr("Import file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));
        if (!importFile(selectedFile, false))
        {
            failedFiles << QFileInfo(selectedFile).fileName();
        }
        overallId++;
    }
    pbDialog.close();
    for (QString curErrorStr : failedFiles)
    {
        errorStr += ", " % curErrorStr;
    }
    if (errorStr != "")
    {
        errorStr.remove(0, 2);
        QMessageBox::warning(this, tr("Import"), tr("Import failed with...\n\n%1").arg(errorStr));
    }
}

bool ProfileInterface::importFile(QString selectedFile, bool notMultiple)
{
    QString selectedFileName = QFileInfo(selectedFile).fileName();
    if (QFile::exists(selectedFile))
    {
        if (selectedFileName.left(4) == "PGTA" || selectedFileName.right(4) == ".g5e")
        {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            if (picture->readingPicture(true, true, true))
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
        else if(isSupportedImageFile(selectedFileName))
        {
            SnapmaticPicture *picture = new SnapmaticPicture(":/template/template.g5e");
            if (picture->readingPicture(true, false, true, false))
            {
                if (!notMultiple)
                {
                    QFile snapmaticFile(selectedFile);
                    if (!snapmaticFile.open(QFile::ReadOnly))
                    {
                        delete picture;
                        return false;
                    }
                    QImage snapmaticImage;
                    QImageReader snapmaticImageReader;
                    snapmaticImageReader.setDecideFormatFromContent(true);
                    snapmaticImageReader.setDevice(&snapmaticFile);
                    if (!snapmaticImageReader.read(&snapmaticImage))
                    {
                        delete picture;
                        return false;
                    }
                    QString customImageTitle;
                    QPixmap snapmaticPixmap(960, 536);
                    snapmaticPixmap.fill(Qt::black);
                    QPainter snapmaticPainter(&snapmaticPixmap);
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
                    QString currentTime = QTime::currentTime().toString("HHmmss");
                    SnapmaticProperties spJson = picture->getSnapmaticProperties();
                    spJson.uid = QString(currentTime %
                                         QString::number(QDate::currentDate().dayOfYear())).toInt();
                    bool fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                    bool fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                    int cEnough = 0;
                    while ((fExists || fExistsHidden) && cEnough < 5000)
                    {
                        currentTime = QString::number(currentTime.toInt() - 1);
                        spJson.uid = QString(currentTime %
                                             QString::number(QDate::currentDate().dayOfYear())).toInt();
                        fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                        fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                        cEnough++;
                    }
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
                    QFile snapmaticFile(selectedFile);
                    if (!snapmaticFile.open(QFile::ReadOnly))
                    {
                        delete picture;
                        return false;
                    }
                    QImage *importImage = new QImage();
                    QImageReader snapmaticImageReader;
                    snapmaticImageReader.setDecideFormatFromContent(true);
                    snapmaticImageReader.setDevice(&snapmaticFile);
                    if (!snapmaticImageReader.read(importImage))
                    {
                        QMessageBox::warning(this, tr("Import"), tr("Can't import %1 because file can't be parsed properly").arg("\""+selectedFileName+"\""));
                        delete picture;
                        return false;
                    }
                    ImportDialog *importDialog = new ImportDialog(this);
                    importDialog->setWindowFlags(importDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
                    importDialog->setImage(importImage);
                    importDialog->setModal(true);
                    importDialog->show();
                    importDialog->exec();
                    if (importDialog->isImportAgreed())
                    {
                        if (picture->setImage(importDialog->image()))
                        {
                            QString currentTime = QTime::currentTime().toString("HHmmss");
                            SnapmaticProperties spJson = picture->getSnapmaticProperties();
                            spJson.uid = QString(currentTime +
                                                 QString::number(QDate::currentDate().dayOfYear())).toInt();
                            bool fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                            bool fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                            int cEnough = 0;
                            while ((fExists || fExistsHidden) && cEnough < 25)
                            {
                                currentTime = QString::number(currentTime.toInt() - 1);
                                spJson.uid = QString(currentTime %
                                                     QString::number(QDate::currentDate().dayOfYear())).toInt();
                                fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                                fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                                cEnough++;
                            }
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
#ifdef GTA5SYNC_DEBUG
                qDebug() << "ImportError SnapmaticPicture" << picture->getLastStep();
                qDebug() << "ImportError SavegameData" << savegame->getLastStep();
#endif
                delete picture;
                delete savegame;
                if (notMultiple) QMessageBox::warning(this, tr("Import"), tr("Can't import %1 because file format can't be detected").arg("\""+selectedFileName+"\""));
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
    QString adjustedFileName = picture->getOriginalPictureFileName();
    if (picFileName.left(4) != "PGTA")
    {
        if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to import the Snapmatic picture, file not begin with PGTA or end with .g5e"));
        return false;
    }
    else if (QFile::exists(profileFolder % "/" % adjustedFileName) || QFile::exists(profileFolder % "/" % adjustedFileName % ".hidden"))
    {
        if (warn) QMessageBox::warning(this, tr("Import"), tr("Failed to import the Snapmatic picture, the picture is already in the game"));
        return false;
    }
    else if (picture->exportPicture(profileFolder % "/" % adjustedFileName, SnapmaticFormat::PGTA_Format))
    {
        picture->setPicFilePath(profileFolder % "/" % adjustedFileName);
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
        sgdFileName = "SGTA500" % sgdNumber;

        if (!QFile::exists(profileFolder % "/" % sgdFileName))
        {
            foundFree = true;
        }
        currentSgd++;
    }

    if (foundFree)
    {
        if (QFile::copy(sgdPath, profileFolder % "/" % sgdFileName))
        {
            savegame->setSavegameFileName(profileFolder % "/" % sgdFileName);
            savegameLoaded(savegame, profileFolder % "/" % sgdFileName, true);
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
        for (ProfileWidget *widget : widgets.keys())
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
        for (ProfileWidget *widget : widgets.keys())
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
    for (ProfileWidget *widget : widgets.keys())
    {
        widget->setSelected(true);
    }
}

void ProfileInterface::deselectAllWidgets()
{
    for (ProfileWidget *widget : widgets.keys())
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
        //bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
        settings.beginGroup("ExportDirectory");
        QString exportDirectory = QFileDialog::getExistingDirectory(this, tr("Export selected"), settings.value(profileName, profileFolder).toString());
        if (exportDirectory != "")
        {
            settings.setValue(profileName, exportDirectory);
            for (ProfileWidget *widget : widgets.keys())
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
                    // Don't export anymore when any Cancel button got clicked
                    settings.endGroup();
                    settings.endGroup();
                    return;
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
            pbDialog.setLabelText(tr("Initialising export..."));
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

            for (QString curErrorStr : errorList)
            {
                errorStr += ", " % curErrorStr;
            }
            if (errorStr != "")
            {
                errorStr.remove(0, 2);
                QMessageBox::warning(this, tr("Export selected"), tr("Export failed with...\n\n%1").arg(errorStr));
            }

            if (exportThread->isFinished())
            {
                delete exportThread;
            }
            else
            {
                QEventLoop threadFinishLoop;
                QObject::connect(exportThread, SIGNAL(finished()), &threadFinishLoop, SLOT(quit()));
                threadFinishLoop.exec();
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
            for (ProfileWidget *widget : widgets.keys())
            {
                if (widget->isSelected())
                {
                    if (widget->getWidgetType() == "SnapmaticWidget")
                    {
                        SnapmaticWidget *picWidget = qobject_cast<SnapmaticWidget*>(widget);
                        if (picWidget->getPicture()->deletePicFile())
                        {
                            pictureDeleted(picWidget);
                        }
                    }
                    else if (widget->getWidgetType() == "SavegameWidget")
                    {
                        SavegameWidget *sgdWidget = qobject_cast<SavegameWidget*>(widget);
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

void ProfileInterface::settingsApplied(int _contentMode, bool languageChanged)
{
    if (languageChanged) retranslateUi();
    contentMode = _contentMode;
    if (contentMode == 2)
    {
        for (ProfileWidget *widget : widgets.keys())
        {
            widget->setSelectionMode(true);
            widget->setContentMode(contentMode);
            if (languageChanged) widget->retranslate();
        }
    }
    else
    {
        for (ProfileWidget *widget : widgets.keys())
        {
            if (selectedWidgts == 0)
            {
                widget->setSelectionMode(false);
            }
            widget->setContentMode(contentMode);
            if (languageChanged) widget->retranslate();
        }
    }
}

void ProfileInterface::enableSelected()
{
    int fails = 0;
    for (ProfileWidget *widget : widgets.keys())
    {
        if (widget->isSelected())
        {
            if (widget->getWidgetType() == "SnapmaticWidget")
            {
                SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
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
    for (ProfileWidget *widget : widgets.keys())
    {
        if (widget->isSelected())
        {
            if (widget->getWidgetType() == "SnapmaticWidget")
            {
                SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
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
    SnapmaticWidget *picWidget = qobject_cast<SnapmaticWidget*>(sender());
    if (picWidget != previousWidget)
    {
        if (previousWidget != nullptr)
        {
            previousWidget->setStyleSheet(QLatin1String(""));
        }
        picWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color: rgb(%1, %2, %3)}QLabel#labPicStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
        previousWidget = picWidget;
    }
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
    editMenu.addAction(PictureDialog::tr("&Edit Properties..."), picWidget, SLOT(editSnapmaticProperties()));
    QMenu exportMenu(SnapmaticWidget::tr("&Export"), this);
    exportMenu.addAction(PictureDialog::tr("Export as &Picture..."), picWidget, SLOT(on_cmdExport_clicked()));
    exportMenu.addAction(PictureDialog::tr("Export as &Snapmatic..."), picWidget, SLOT(on_cmdCopy_clicked()));
    contextMenu.addAction(SnapmaticWidget::tr("&View"), picWidget, SLOT(on_cmdView_clicked()));
    contextMenu.addMenu(&editMenu);
    contextMenu.addMenu(&exportMenu);
    contextMenu.addAction(SnapmaticWidget::tr("&Remove"), picWidget, SLOT(on_cmdDelete_clicked()));
    contextMenu.addSeparator();
    if (!picWidget->isSelected()) { contextMenu.addAction(SnapmaticWidget::tr("&Select"), picWidget, SLOT(pictureSelected())); }
    if (picWidget->isSelected()) { contextMenu.addAction(SnapmaticWidget::tr("&Deselect"), picWidget, SLOT(pictureSelected())); }
    if (selectedWidgets() != widgets.count())
    {
        contextMenu.addAction(SnapmaticWidget::tr("Select &All"), picWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
    }
    if (selectedWidgets() != 0)
    {
        contextMenu.addAction(SnapmaticWidget::tr("&Deselect All"), picWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
    }
    contextMenuOpened = true;
    contextMenu.exec(ev->globalPos());
    contextMenuOpened = false;
    hoverProfileWidgetCheck();
}

void ProfileInterface::contextMenuTriggeredSGD(QContextMenuEvent *ev)
{
    SavegameWidget *sgdWidget = qobject_cast<SavegameWidget*>(sender());
    if (sgdWidget != previousWidget)
    {
        if (previousWidget != nullptr)
        {
            previousWidget->setStyleSheet(QLatin1String(""));
        }
        sgdWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color: rgb(%1, %2, %3)}QLabel#labSavegameStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
        previousWidget = sgdWidget;
    }
    QMenu contextMenu(sgdWidget);
    contextMenu.addAction(SavegameWidget::tr("&View"), sgdWidget, SLOT(on_cmdView_clicked()));
    contextMenu.addAction(SavegameWidget::tr("&Export"), sgdWidget, SLOT(on_cmdCopy_clicked()));
    contextMenu.addAction(SavegameWidget::tr("&Remove"), sgdWidget, SLOT(on_cmdDelete_clicked()));
    contextMenu.addSeparator();
    if (!sgdWidget->isSelected()) { contextMenu.addAction(SavegameWidget::tr("&Select"), sgdWidget, SLOT(savegameSelected())); }
    if (sgdWidget->isSelected()) { contextMenu.addAction(SavegameWidget::tr("&Deselect"), sgdWidget, SLOT(savegameSelected())); }
    if (selectedWidgets() != widgets.count())
    {
        contextMenu.addAction(SavegameWidget::tr("Select &All"), sgdWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
    }
    if (selectedWidgets() != 0)
    {
        contextMenu.addAction(SavegameWidget::tr("&Deselect All"), sgdWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
    }
    contextMenuOpened = true;
    contextMenu.exec(ev->globalPos());
    contextMenuOpened = false;
    hoverProfileWidgetCheck();
}

void ProfileInterface::on_saProfileContent_dropped(const QMimeData *mimeData)
{
    if (!mimeData) return;
    QStringList pathList;

    for (QUrl currentUrl : mimeData->urls())
    {
        if (currentUrl.isLocalFile())
        {
            pathList += currentUrl.toLocalFile();
        }
    }

    if (pathList.length() == 1)
    {
        QString selectedFile = pathList.at(0);
        importFile(selectedFile, true);
    }
    else if (pathList.length() > 1)
    {
        importFilesProgress(pathList);
    }
}

void ProfileInterface::retranslateUi()
{
    ui->retranslateUi(this);
    ui->labVersion->setText(QString("%1 %2").arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER));
}

bool ProfileInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        if ((watched->objectName() == "SavegameWidget" || watched->objectName() == "SnapmaticWidget") && isProfileLoaded)
        {
            ProfileWidget *pWidget = qobject_cast<ProfileWidget*>(watched);
            if (pWidget->underMouse())
            {
                bool styleSheetChanged = false;
                if (pWidget->getWidgetType() == "SnapmaticWidget")
                {
                    if (pWidget != previousWidget)
                    {
                        pWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color: rgb(%1, %2, %3)}QLabel#labPicStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
                        styleSheetChanged = true;
                    }
                }
                else if (pWidget->getWidgetType() == "SavegameWidget")
                {
                    if (pWidget != previousWidget)
                    {
                        pWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color: rgb(%1, %2, %3)}QLabel#labSavegameStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
                        styleSheetChanged = true;
                    }
                }
                if (styleSheetChanged)
                {
                    if (previousWidget != nullptr)
                    {
                        previousWidget->setStyleSheet(QLatin1String(""));
                    }
                    previousWidget = pWidget;
                }
            }
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::WindowActivate)
    {
        if ((watched->objectName() == "SavegameWidget" || watched->objectName() == "SnapmaticWidget") && isProfileLoaded)
        {
            ProfileWidget *pWidget = nullptr;
            for (ProfileWidget *widget : widgets.keys())
            {
                QPoint mousePos = widget->mapFromGlobal(QCursor::pos());
                if (widget->rect().contains(mousePos))
                {
                    pWidget = widget;
                    break;
                }
            }
            if (pWidget != nullptr)
            {
                bool styleSheetChanged = false;
                if (pWidget->getWidgetType() == "SnapmaticWidget")
                {
                    if (pWidget != previousWidget)
                    {
                        pWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color: rgb(%1, %2, %3)}QLabel#labPicStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
                        styleSheetChanged = true;
                    }
                }
                else if (pWidget->getWidgetType() == "SavegameWidget")
                {
                    if (pWidget != previousWidget)
                    {
                        pWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color: rgb(%1, %2, %3)}QLabel#labSavegameStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
                        styleSheetChanged = true;
                    }
                }
                if (styleSheetChanged)
                {
                    if (previousWidget != nullptr)
                    {
                        previousWidget->setStyleSheet(QLatin1String(""));
                    }
                    previousWidget = pWidget;
                }
            }
        }
    }
    else if (event->type() == QEvent::WindowDeactivate && isProfileLoaded)
    {
        if (previousWidget != nullptr)
        {
            previousWidget->setStyleSheet(QLatin1String(""));
            previousWidget = nullptr;
        }
    }
    else if (event->type() == QEvent::Leave && isProfileLoaded && !contextMenuOpened)
    {
        if (watched->objectName() == "SavegameWidget" || watched->objectName() == "SnapmaticWidget")
        {
            ProfileWidget *pWidget = qobject_cast<ProfileWidget*>(watched);
            QPoint mousePos = pWidget->mapFromGlobal(QCursor::pos());
            if (!pWidget->geometry().contains(mousePos))
            {
                if (previousWidget != nullptr)
                {
                    previousWidget->setStyleSheet(QLatin1String(""));
                    previousWidget = nullptr;
                }
            }
        }
    }
    return false;
}

void ProfileInterface::hoverProfileWidgetCheck()
{
    ProfileWidget *pWidget = nullptr;
    for (ProfileWidget *widget : widgets.keys())
    {
        if (widget->underMouse())
        {
            pWidget = widget;
            break;
        }
    }
    if (pWidget != nullptr)
    {
        bool styleSheetChanged = false;
        if (pWidget->getWidgetType() == "SnapmaticWidget")
        {
            if (pWidget != previousWidget)
            {
                pWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color: rgb(%1, %2, %3)}QLabel#labPicStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
                styleSheetChanged = true;
            }
        }
        else if (pWidget->getWidgetType() == "SavegameWidget")
        {
            if (pWidget != previousWidget)
            {
                pWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color: rgb(%1, %2, %3)}QLabel#labSavegameStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
                styleSheetChanged = true;
            }
        }
        if (styleSheetChanged)
        {
            if (previousWidget != nullptr)
            {
                previousWidget->setStyleSheet(QLatin1String(""));
            }
            previousWidget = pWidget;
        }
    }
    else
    {
        if (previousWidget != nullptr)
        {
            previousWidget->setStyleSheet(QLatin1String(""));
            previousWidget = nullptr;
        }
    }
}

void ProfileInterface::updatePalette()
{
    QPalette palette;
    QColor baseColor = palette.base().color();
    highlightBackColor = palette.highlight().color();
    highlightTextColor = palette.highlightedText().color();
    ui->saProfile->setStyleSheet(QString("QWidget#saProfileContent{background-color: rgb(%1, %2, %3)}").arg(QString::number(baseColor.red()), QString::number(baseColor.green()), QString::number(baseColor.blue())));
    if (previousWidget != nullptr)
    {
        if (previousWidget->getWidgetType() == "SnapmaticWidget")
        {
            previousWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color: rgb(%1, %2, %3)}QLabel#labPicStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
        }
        else if (previousWidget->getWidgetType() == "SavegameWidget")
        {
            previousWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color: rgb(%1, %2, %3)}QLabel#labSavegameStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
        }
    }
}

bool ProfileInterface::isSupportedImageFile(QString selectedFileName)
{
    for (QByteArray imageFormat : QImageReader::supportedImageFormats())
    {
        QString imageFormatStr = QString(".") % QString::fromUtf8(imageFormat).toLower();
        if (selectedFileName.length() >= imageFormatStr.length() && selectedFileName.toLower().right(imageFormatStr.length()) == imageFormatStr)
        {
            return true;
        }
    }
    return false;
}
