/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2021 Syping
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
#include "PlayerListDialog.h"
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
#include "UiModLabel.h"
#include "pcg_basic.h"
#include "wrapper.h"
#include "AppEnv.h"
#include "config.h"
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QNetworkRequest>
#include <QStringBuilder>
#include <QNetworkReply>
#include <QImageReader>
#include <QProgressBar>
#include <QInputDialog>
#include <QPushButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QEventLoop>
#include <QScrollBar>
#include <QClipboard>
#include <QFileInfo>
#include <QPainter>
#include <QAction>
#include <QDebug>
#include <QColor>
#include <QTimer>
#include <QStyle>
#include <QFile>
#include <QUrl>
#include <QDir>

#include <cstdint>
#include <random>
#include <ctime>

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#include <QJsonDocument>
#include <QJsonObject>
#endif

#define importTimeFormat "HHmmss"
#define findRetryLimit 500

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
    QString appVersion = GTA5SYNC_APPVER;
#ifndef GTA5SYNC_BUILDTYPE_REL
#ifdef GTA5SYNC_COMMIT
    if (!appVersion.contains("-")) { appVersion = appVersion % "-" % GTA5SYNC_COMMIT; }
#endif
#endif
    ui->labVersion->setText(QString("%1 %2").arg(GTA5SYNC_APPSTR, appVersion));
    ui->saProfileContent->setFilesDropEnabled(true);
    ui->saProfileContent->setImageDropEnabled(true);

    // Set Icon for Close Button
    if (QIcon::hasThemeIcon("dialog-close")) {
        ui->cmdCloseProfile->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close")) {
        ui->cmdCloseProfile->setIcon(QIcon::fromTheme("gtk-close"));
    }

    // Set Icon for Import Button
    if (QIcon::hasThemeIcon("document-import")) {
        ui->cmdImport->setIcon(QIcon::fromTheme("document-import"));
    }
    else if (QIcon::hasThemeIcon("document-open")) {
        ui->cmdImport->setIcon(QIcon::fromTheme("document-open"));
    }

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
#ifndef Q_OS_MAC
    ui->hlButtons->setSpacing(6 * screenRatio);
    ui->hlButtons->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
#else
    if (QApplication::style()->objectName() == "macintosh") {
        ui->hlButtons->setSpacing(6 * screenRatio);
        ui->hlButtons->setContentsMargins(9 * screenRatio, 15 * screenRatio, 15 * screenRatio, 17 * screenRatio);
    }
    else {
        ui->hlButtons->setSpacing(6 * screenRatio);
        ui->hlButtons->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
    }
#endif

    // Seed RNG
    pcg32_srandom_r(&rng, time(NULL), (intptr_t)&rng);

#if QT_VERSION >= 0x050000
    // Register Metatypes
    qRegisterMetaType<QVector<QString>>();
#endif

    setMouseTracking(true);
    installEventFilter(this);
}

ProfileInterface::~ProfileInterface()
{
    for (const QString &widgetStr : qAsConst(widgets)) {
        ProfileWidget *widget = widgets.key(widgetStr, nullptr);
        if (widget != nullptr) {
            widget->removeEventFilter(this);
            widget->disconnect();
            delete widget;
        }
    }
    widgets.clear();

    for (SavegameData *savegame : qAsConst(savegames)) {
        delete savegame;
    }
    savegames.clear();

    for (SnapmaticPicture *picture : qAsConst(pictures)) {
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
    fixedPictures.clear();
    ui->labProfileLoading->setText(tr("Loading..."));
    profileLoader = new ProfileLoader(profileFolder, crewDB);
#if QT_VERSION >= 0x050000
    QObject::connect(profileLoader, SIGNAL(directoryScanned(QVector<QString>,QVector<QString>)), this, SLOT(directoryScanned(QVector<QString>,QVector<QString>)));
#endif
    QObject::connect(profileLoader, SIGNAL(savegameLoaded(SavegameData*, QString)), this, SLOT(savegameLoaded_event(SavegameData*, QString)));
    QObject::connect(profileLoader, SIGNAL(pictureLoaded(SnapmaticPicture*)), this, SLOT(pictureLoaded_event(SnapmaticPicture*)));
    QObject::connect(profileLoader, SIGNAL(pictureFixed(SnapmaticPicture*)), this, SLOT(pictureFixed_event(SnapmaticPicture*)));
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
    if (selectedWidgts != 0 || contentMode == 2)
        sgdWidget->setSelectionMode(true);
    QObject::connect(sgdWidget, SIGNAL(savegameDeleted()), this, SLOT(savegameDeleted_event()));
    QObject::connect(sgdWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(sgdWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
    QObject::connect(sgdWidget, SIGNAL(allWidgetsSelected()), this, SLOT(selectAllWidgets()));
    QObject::connect(sgdWidget, SIGNAL(allWidgetsDeselected()), this, SLOT(deselectAllWidgets()));
    QObject::connect(sgdWidget, SIGNAL(contextMenuTriggered(QContextMenuEvent*)), this, SLOT(contextMenuTriggeredSGD(QContextMenuEvent*)));
    if (inserted)
        insertSavegameIPI(sgdWidget);
}

void ProfileInterface::pictureLoaded_event(SnapmaticPicture *picture)
{
    pictureLoaded(picture, false);
}

void ProfileInterface::pictureFixed_event(SnapmaticPicture *picture)
{
    QString fixedPicture = picture->getPictureStr() % " (" % picture->getPictureTitl() % ")";
    fixedPictures << fixedPicture;
}

void ProfileInterface::pictureLoaded(SnapmaticPicture *picture, bool inserted)
{
    SnapmaticWidget *picWidget = new SnapmaticWidget(profileDB, crewDB, threadDB, profileName, this);
    picWidget->setSnapmaticPicture(picture);
    picWidget->setContentMode(contentMode);
    picWidget->setMouseTracking(true);
    picWidget->installEventFilter(this);
    widgets[picWidget] = "PIC" % picture->getPictureSortStr();
    pictures += picture;
    if (selectedWidgts != 0 || contentMode == 2)
        picWidget->setSelectionMode(true);
    QObject::connect(picWidget, SIGNAL(pictureDeleted()), this, SLOT(pictureDeleted_event()));
    QObject::connect(picWidget, SIGNAL(widgetSelected()), this, SLOT(profileWidgetSelected()));
    QObject::connect(picWidget, SIGNAL(widgetDeselected()), this, SLOT(profileWidgetDeselected()));
    QObject::connect(picWidget, SIGNAL(allWidgetsSelected()), this, SLOT(selectAllWidgets()));
    QObject::connect(picWidget, SIGNAL(allWidgetsDeselected()), this, SLOT(deselectAllWidgets()));
    QObject::connect(picWidget, SIGNAL(nextPictureRequested(QWidget*)), this, SLOT(dialogNextPictureRequested(QWidget*)));
    QObject::connect(picWidget, SIGNAL(previousPictureRequested(QWidget*)), this, SLOT(dialogPreviousPictureRequested(QWidget*)));
    QObject::connect(picWidget, SIGNAL(contextMenuTriggered(QContextMenuEvent*)), this, SLOT(contextMenuTriggeredPIC(QContextMenuEvent*)));
    if (inserted)
        insertSnapmaticIPI(picWidget);
}

void ProfileInterface::loadingProgress(int value, int maximum)
{
    ui->pbPictureLoading->setMaximum(maximum);
    ui->pbPictureLoading->setValue(value);
    ui->labProfileLoading->setText(loadingStr.arg(QString::number(value), QString::number(maximum)));
}

#if QT_VERSION >= 0x050000
void ProfileInterface::directoryChanged(const QString &path)
{
    Q_UNUSED(path)
    QDir dir(profileFolder);
    QVector<QString> t_savegameFiles;
    QVector<QString> t_snapmaticPics;
    QVector<QString> n_savegameFiles;
    QVector<QString> n_snapmaticPics;
    const QStringList files = dir.entryList(QDir::Files);
    for (const QString &fileName : files) {
        if (fileName.startsWith("SGTA5") && !fileName.endsWith(".bak")) {
            t_savegameFiles << fileName;
            if (!savegameFiles.contains(fileName)) {
                n_savegameFiles << fileName;
            }
        }
        if (fileName.startsWith("PGTA5") && !fileName.endsWith(".bak")) {
            t_snapmaticPics << fileName;
            if (!snapmaticPics.contains(fileName)) {
                n_snapmaticPics << fileName;
            }
        }
    }
    savegameFiles = t_savegameFiles;
    snapmaticPics = t_snapmaticPics;

    if (!n_savegameFiles.isEmpty() || !n_snapmaticPics.isEmpty()) {
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();

        for (const QString &fileName : qAsConst(n_savegameFiles)) {
            const QString filePath = profileFolder % "/" % fileName;
            SavegameData *savegame = new SavegameData(filePath);
            if (savegame->readingSavegame())
                savegameLoaded(savegame, filePath, true);
            else
                delete savegame;
        }
        for (const QString &fileName : qAsConst(n_snapmaticPics)) {
            const QString filePath = profileFolder % "/" % fileName;
            SnapmaticPicture *picture = new SnapmaticPicture(filePath);
            if (picture->readingPicture(true))
                pictureLoaded(picture, true);
            else
                delete picture;
        }
    }
}

void ProfileInterface::directoryScanned(QVector<QString> savegameFiles_s, QVector<QString> snapmaticPics_s)
{
    savegameFiles = savegameFiles_s;
    snapmaticPics = snapmaticPics_s;
    fileSystemWatcher.addPath(profileFolder);
    QObject::connect(&fileSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));
}
#endif

void ProfileInterface::insertSnapmaticIPI(QWidget *widget)
{
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(widget);
    const QString widgetKey = widgets.value(proWidget, QString());
    if (!widgetKey.isNull()) {
        QStringList widgetsKeyList = widgets.values();
        QStringList pictureKeyList = widgetsKeyList.filter("PIC", Qt::CaseSensitive);
#if QT_VERSION >= 0x050600
        std::sort(pictureKeyList.rbegin(), pictureKeyList.rend());
#else
        qSort(pictureKeyList.begin(), pictureKeyList.end(), qGreater<QString>());
#endif
        int picIndex = pictureKeyList.indexOf(widgetKey);
        ui->vlSnapmatic->insertWidget(picIndex, proWidget);

        QApplication::processEvents();
        ui->saProfile->ensureWidgetVisible(proWidget, 0, 0);
    }
}

void ProfileInterface::insertSavegameIPI(QWidget *widget)
{
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(widget);
    const QString widgetKey = widgets.value(proWidget, QString());
    if (!widgetKey.isNull()) {
        QStringList widgetsKeyList = widgets.values();
        QStringList savegameKeyList = widgetsKeyList.filter("SGD", Qt::CaseSensitive);
#if QT_VERSION >= 0x050600
        std::sort(savegameKeyList.begin(), savegameKeyList.end());
#else
        qSort(savegameKeyList.begin(), savegameKeyList.end());
#endif
        int sgdIndex = savegameKeyList.indexOf(widgetKey);
        ui->vlSavegame->insertWidget(sgdIndex, proWidget);

        QApplication::processEvents();
        ui->saProfile->ensureWidgetVisible(proWidget, 0, 0);
    }
}

void ProfileInterface::dialogNextPictureRequested(QWidget *dialog)
{
    PictureDialog *picDialog = qobject_cast<PictureDialog*>(dialog);
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(sender());
    const QString widgetKey = widgets.value(proWidget, QString());
    if (!widgetKey.isNull()) {
        QStringList widgetsKeyList = widgets.values();
        QStringList pictureKeyList = widgetsKeyList.filter("PIC", Qt::CaseSensitive);
#if QT_VERSION >= 0x050600
        std::sort(pictureKeyList.rbegin(), pictureKeyList.rend());
#else
        qSort(pictureKeyList.begin(), pictureKeyList.end(), qGreater<QString>());
#endif
        int picIndex;
        if (picDialog->isIndexed()) {
            picIndex = picDialog->getIndex();
        }
        else {
            picIndex = pictureKeyList.indexOf(widgetKey);
        }
        picIndex++;
        if (pictureKeyList.length() > picIndex) {
            const QString newWidgetKey = pictureKeyList.at(picIndex);
            SnapmaticWidget *picWidget = static_cast<SnapmaticWidget*>(widgets.key(newWidgetKey));
            picDialog->setSnapmaticPicture(picWidget->getPicture(), picIndex);
        }
    }
}

void ProfileInterface::dialogPreviousPictureRequested(QWidget *dialog)
{
    PictureDialog *picDialog = qobject_cast<PictureDialog*>(dialog);
    ProfileWidget *proWidget = qobject_cast<ProfileWidget*>(sender());
    const QString widgetKey = widgets.value(proWidget, QString());
    if (!widgetKey.isNull()) {
        QStringList widgetsKeyList = widgets.values();
        QStringList pictureKeyList = widgetsKeyList.filter("PIC", Qt::CaseSensitive);
#if QT_VERSION >= 0x050600
        std::sort(pictureKeyList.rbegin(), pictureKeyList.rend());
#else
        qSort(pictureKeyList.begin(), pictureKeyList.end(), qGreater<QString>());
#endif
        int picIndex;
        if (picDialog->isIndexed()) {
            picIndex = picDialog->getIndex();
        }
        else {
            picIndex = pictureKeyList.indexOf(widgetKey);
        }
        if (picIndex > 0) {
            picIndex--;
            const QString newWidgetKey = pictureKeyList.at(picIndex);
            SnapmaticWidget *picWidget = static_cast<SnapmaticWidget*>(widgets.key(newWidgetKey));
            picDialog->setSnapmaticPicture(picWidget->getPicture(), picIndex);
        }
    }
}

void ProfileInterface::sortingProfileInterface()
{
    ui->vlSavegame->setEnabled(false);
    ui->vlSnapmatic->setEnabled(false);

    QStringList widgetsKeyList = widgets.values();

#if QT_VERSION >= 0x050600
    std::sort(widgetsKeyList.begin(), widgetsKeyList.end());
#else
    qSort(widgetsKeyList.begin(), widgetsKeyList.end());
#endif

    for (const QString &widgetKey : qAsConst(widgetsKeyList)) {
        ProfileWidget *widget = widgets.key(widgetKey);
        if (widget->getWidgetType() == "SnapmaticWidget") {
            ui->vlSnapmatic->insertWidget(0, widget);
        }
        else if (widget->getWidgetType() == "SavegameWidget") {
            ui->vlSavegame->addWidget(widget);
        }
    }

    ui->vlSavegame->setEnabled(true);
    ui->vlSnapmatic->setEnabled(true);

    QApplication::processEvents();
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

    if (!fixedPictures.isEmpty()) {
        int fixedInt = 0;
        QString fixedStr;
        for (const QString &fixedPicture : qAsConst(fixedPictures)) {
            if (fixedInt != 0) { fixedStr += "<br>"; }
            fixedStr += fixedPicture;
            fixedInt++;
        }
        QMessageBox::information(this, tr("Snapmatic Loader"), tr("<h4>Following Snapmatic Pictures got repaired</h4>%1").arg(fixedStr));
    }
}

void ProfileInterface::savegameDeleted_event()
{
    savegameDeleted(qobject_cast<SavegameWidget*>(sender()), true);
}

void ProfileInterface::savegameDeleted(SavegameWidget *sgdWidget, bool isRemoteEmited)
{
    SavegameData *savegame = sgdWidget->getSavegame();
    if (sgdWidget->isSelected())
        sgdWidget->setSelected(false);
    widgets.remove(sgdWidget);

    sgdWidget->disconnect();
    sgdWidget->removeEventFilter(this);
    if (sgdWidget == previousWidget) {
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
    if (picWidget->isSelected())
        picWidget->setSelected(false);
    widgets.remove(picWidget);

    picWidget->disconnect();
    picWidget->removeEventFilter(this);
    if (picWidget == previousWidget) {
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
    fileDialog.setLabelText(QFileDialog::Accept, tr("Import..."));

    // Getting readable Image formats
    QString imageFormatsStr = " ";
    for (const QByteArray &imageFormat : QImageReader::supportedImageFormats()) {
        imageFormatsStr += QString("*.") % QString::fromUtf8(imageFormat).toLower() % " ";
    }
    QString importableFormatsStr = QString("*.g5e SGTA* PGTA*");
    if (!imageFormatsStr.trimmed().isEmpty()) {
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

    if (fileDialog.exec()) {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1) {
            QString selectedFile = selectedFiles.at(0);
            QDateTime importDateTime = QDateTime::currentDateTime();
            if (!importFile(selectedFile, importDateTime, true)) goto fileDialogPreOpen; //Work?
        }
        else if (selectedFiles.length() > 1) {
            importFilesProgress(selectedFiles);
        }
        else {
            QMessageBox::warning(this, tr("Import..."), tr("No valid file is selected"));
            goto fileDialogPreOpen; //Work?
        }
    }

    settings.setValue(profileName % "+Geometry", fileDialog.saveGeometry());
    settings.setValue(profileName % "+Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}

bool ProfileInterface::importFilesProgress(QStringList selectedFiles)
{
    int maximumId = selectedFiles.length();
    int overallId = 0;
    QString errorStr;
    QStringList failed;

    // Progress dialog
    QProgressDialog pbDialog(this);
    pbDialog.setWindowFlags(pbDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
    pbDialog.setWindowTitle(tr("Import..."));
    pbDialog.setLabelText(tr("Import file %1 of %2 files").arg(QString::number(1), QString::number(maximumId)));
    pbDialog.setRange(1, maximumId);
    pbDialog.setValue(1);
    pbDialog.setModal(true);
    QList<QPushButton*> pbBtn = pbDialog.findChildren<QPushButton*>();
    pbBtn.at(0)->setDisabled(true);
    QList<QProgressBar*> pbBar = pbDialog.findChildren<QProgressBar*>();
    pbBar.at(0)->setTextVisible(false);
    pbDialog.setAutoClose(false);
    pbDialog.show();

    // THREADING HERE PLEASE
    QDateTime importDateTime = QDateTime::currentDateTime();
    for (const QString &selectedFile : selectedFiles) {
        overallId++;
        pbDialog.setValue(overallId);
        pbDialog.setLabelText(tr("Import file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));
        importDateTime = QDateTime::currentDateTime();
        if (!importFile(selectedFile, importDateTime, false)) {
            failed << QFileInfo(selectedFile).fileName();
        }
    }

    pbDialog.close();
    for (const QString &curErrorStr : qAsConst(failed)) {
        errorStr += ", " % curErrorStr;
    }
    if (errorStr != "") {
        errorStr.remove(0, 2);
        QMessageBox::warning(this, tr("Import..."), tr("Import failed with...\n\n%1").arg(errorStr));
        return false;
    }
    return true;
}

bool ProfileInterface::importFile(QString selectedFile, QDateTime importDateTime, bool notMultiple)
{
    QString selectedFileName = QFileInfo(selectedFile).fileName();
    if (QFile::exists(selectedFile)) {
        if ((selectedFileName.left(4) == "PGTA" && !selectedFileName.contains('.')) || selectedFileName.right(4) == ".g5e") {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            if (picture->readingPicture(true)) {
                bool success = importSnapmaticPicture(picture, notMultiple);
                if (!success)
                    delete picture;
#ifdef GTA5SYNC_TELEMETRY
                if (success && notMultiple) {
                    QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
                    telemetrySettings.beginGroup("Telemetry");
                    bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
                    telemetrySettings.endGroup();
                    if (pushUsageData && Telemetry->canPush()) {
                        QJsonDocument jsonDocument;
                        QJsonObject jsonObject;
                        jsonObject["Type"] = "ImportSuccess";
                        jsonObject["ImportSize"] = QString::number(picture->getContentMaxLength());
#if QT_VERSION >= 0x060000
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                        jsonObject["ImportType"] = "Snapmatic";
                        jsonDocument.setObject(jsonObject);
                        Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
                    }
                }
#endif
                return success;
            }
            else {
                if (notMultiple)
                    QMessageBox::warning(this, tr("Import..."), tr("Failed to read Snapmatic picture"));
                delete picture;
                return false;
            }
        }
        else if (selectedFileName.left(4) == "SGTA") {
            SavegameData *savegame = new SavegameData(selectedFile);
            if (savegame->readingSavegame()) {
                bool success = importSavegameData(savegame, selectedFile, notMultiple);
                if (!success)
                    delete savegame;
#ifdef GTA5SYNC_TELEMETRY
                if (success && notMultiple) {
                    QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
                    telemetrySettings.beginGroup("Telemetry");
                    bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
                    telemetrySettings.endGroup();
                    if (pushUsageData && Telemetry->canPush()) {
                        QJsonDocument jsonDocument;
                        QJsonObject jsonObject;
                        jsonObject["Type"] = "ImportSuccess";
#if QT_VERSION >= 0x060000
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                        jsonObject["ImportType"] = "Savegame";
                        jsonDocument.setObject(jsonObject);
                        Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
                    }
                }
#endif
                return success;
            }
            else {
                if (notMultiple) QMessageBox::warning(this, tr("Import..."), tr("Failed to read Savegame file"));
                delete savegame;
                return false;
            }
        }
        else if (isSupportedImageFile(selectedFileName)) {
            SnapmaticPicture *picture = new SnapmaticPicture(":/template/template.g5e");
            if (picture->readingPicture(false)) {
                if (!notMultiple) {
                    QFile snapmaticFile(selectedFile);
                    if (!snapmaticFile.open(QFile::ReadOnly)) {
                        delete picture;
                        return false;
                    }
                    QImage snapmaticImage;
                    QImageReader snapmaticImageReader;
                    snapmaticImageReader.setDecideFormatFromContent(true);
                    snapmaticImageReader.setDevice(&snapmaticFile);
                    if (!snapmaticImageReader.read(&snapmaticImage)) {
                        delete picture;
                        return false;
                    }
                    QString customImageTitle;
                    QPixmap snapmaticPixmap(960, 536);
                    snapmaticPixmap.fill(Qt::black);
                    QPainter snapmaticPainter(&snapmaticPixmap);
                    if (snapmaticImage.height() == snapmaticImage.width()) {
                        // Avatar mode
                        int diffWidth = 0;
                        int diffHeight = 0;
                        snapmaticImage = snapmaticImage.scaled(470, 470, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        if (snapmaticImage.width() > snapmaticImage.height()) {
                            diffHeight = 470 - snapmaticImage.height();
                            diffHeight = diffHeight / 2;
                        }
                        else if (snapmaticImage.width() < snapmaticImage.height()) {
                            diffWidth = 470 - snapmaticImage.width();
                            diffWidth = diffWidth / 2;
                        }
                        snapmaticPainter.drawImage(145 + diffWidth, 66 + diffHeight, snapmaticImage);
                        customImageTitle = ImportDialog::tr("Custom Avatar", "Custom Avatar Description in SC, don't use Special Character!");
                    }
                    else {
                        // Picture mode
                        int diffWidth = 0;
                        int diffHeight = 0;
                        snapmaticImage = snapmaticImage.scaled(960, 536, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        if (snapmaticImage.width() != 960) {
                            diffWidth = 960 - snapmaticImage.width();
                            diffWidth = diffWidth / 2;
                        }
                        else if (snapmaticImage.height() != 536) {
                            diffHeight = 536 - snapmaticImage.height();
                            diffHeight = diffHeight / 2;
                        }
                        snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, snapmaticImage);
                        customImageTitle = ImportDialog::tr("Custom Picture", "Custom Picture Description in SC, don't use Special Character!");
                    }
                    snapmaticPainter.end();
                    if (!picture->setImage(snapmaticPixmap.toImage())) {
                        delete picture;
                        return false;
                    }
                    SnapmaticProperties spJson = picture->getSnapmaticProperties();
                    spJson.uid = getRandomUid();
                    bool fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                    bool fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".bak");
                    bool fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                    int cEnough = 0;
                    while ((fExists || fExistsBackup || fExistsHidden) && cEnough < findRetryLimit) {
                        spJson.uid = getRandomUid();
                        fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                        fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".bak");
                        fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                        cEnough++;
                    }
                    spJson.createdDateTime = importDateTime;
#if QT_VERSION >= 0x060000
                    quint64 timestamp = spJson.createdDateTime.toSecsSinceEpoch();
                    if (timestamp > UINT32_MAX) {
                        timestamp = UINT32_MAX;
                    }
                    spJson.createdTimestamp = (quint32)timestamp;
#else
                    spJson.createdTimestamp = spJson.createdDateTime.toTime_t();
#endif
                    picture->setSnapmaticProperties(spJson);
                    const QString picFileName = QString("PGTA5%1").arg(QString::number(spJson.uid));
                    picture->setPicFileName(picFileName);
                    picture->setPictureTitle(customImageTitle);
                    picture->updateStrings();
                    bool success = importSnapmaticPicture(picture, notMultiple);
                    if (!success)
                        delete picture;
                    return success;
                }
                else {
                    bool success = false;
                    QFile snapmaticFile(selectedFile);
                    if (!snapmaticFile.open(QFile::ReadOnly)) {
                        QMessageBox::warning(this, tr("Import..."), tr("Can't import %1 because file can't be open").arg("\""+selectedFileName+"\""));
                        delete picture;
                        return false;
                    }
                    QImage *snapmaticImage = new QImage();
                    QImageReader snapmaticImageReader;
                    snapmaticImageReader.setDecideFormatFromContent(true);
                    snapmaticImageReader.setDevice(&snapmaticFile);
                    if (!snapmaticImageReader.read(snapmaticImage)) {
                        QMessageBox::warning(this, tr("Import..."), tr("Can't import %1 because file can't be parsed properly").arg("\""+selectedFileName+"\""));
                        delete snapmaticImage;
                        delete picture;
                        return false;
                    }
                    ImportDialog *importDialog = new ImportDialog(profileName, this);
                    importDialog->setImage(snapmaticImage);
                    importDialog->setModal(true);
                    importDialog->show();
                    importDialog->exec();
                    if (importDialog->isImportAgreed()) {
                        if (picture->setImage(importDialog->image(), importDialog->isUnlimitedBuffer())) {
                            SnapmaticProperties spJson = picture->getSnapmaticProperties();
                            spJson.uid = getRandomUid();
                            bool fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                            bool fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".bak");
                            bool fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                            int cEnough = 0;
                            while ((fExists || fExistsBackup || fExistsHidden) && cEnough < findRetryLimit) {
                                spJson.uid = getRandomUid();
                                fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                                fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".bak");
                                fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                                cEnough++;
                            }
                            spJson.createdDateTime = importDateTime;
#if QT_VERSION >= 0x060000
                            quint64 timestamp = spJson.createdDateTime.toSecsSinceEpoch();
                            if (timestamp > UINT32_MAX) {
                                timestamp = UINT32_MAX;
                            }
                            spJson.createdTimestamp = (quint32)timestamp;
#else
                            spJson.createdTimestamp = spJson.createdDateTime.toTime_t();
#endif
                            picture->setSnapmaticProperties(spJson);
                            const QString picFileName = QString("PGTA5%1").arg(QString::number(spJson.uid));
                            picture->setPicFileName(picFileName);
                            picture->setPictureTitle(importDialog->getImageTitle());
                            picture->updateStrings();
                            success = importSnapmaticPicture(picture, notMultiple);
#ifdef GTA5SYNC_TELEMETRY
                            if (success) {
                                QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
                                telemetrySettings.beginGroup("Telemetry");
                                bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
                                telemetrySettings.endGroup();
                                if (pushUsageData && Telemetry->canPush()) {
                                    QJsonDocument jsonDocument;
                                    QJsonObject jsonObject;
                                    jsonObject["Type"] = "ImportSuccess";
                                    jsonObject["ExtraFlag"] = "Dialog";
                                    jsonObject["ImportSize"] = QString::number(picture->getContentMaxLength());
#if QT_VERSION >= 0x060000
                                    jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                                    jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                                    jsonObject["ImportType"] = "Image";
                                    jsonDocument.setObject(jsonObject);
                                    Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
                                }
                            }
#endif
                        }
                    }
                    else {
                        delete picture;
                        success = true;
                    }
                    delete importDialog;
                    if (!success)
                        delete picture;
                    return success;
                }
            }
            else {
                delete picture;
                return false;
            }
        }
        else {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            SavegameData *savegame = new SavegameData(selectedFile);
            if (picture->readingPicture()) {
                bool success = importSnapmaticPicture(picture, notMultiple);
                delete savegame;
                if (!success)
                    delete picture;
#ifdef GTA5SYNC_TELEMETRY
                if (success && notMultiple) {
                    QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
                    telemetrySettings.beginGroup("Telemetry");
                    bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
                    telemetrySettings.endGroup();
                    if (pushUsageData && Telemetry->canPush()) {
                        QJsonDocument jsonDocument;
                        QJsonObject jsonObject;
                        jsonObject["Type"] = "ImportSuccess";
                        jsonObject["ImportSize"] = QString::number(picture->getContentMaxLength());
#if QT_VERSION >= 0x060000
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                        jsonObject["ImportType"] = "Snapmatic";
                        jsonDocument.setObject(jsonObject);
                        Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
                    }
                }
#endif
                return success;
            }
            else if (savegame->readingSavegame()) {
                bool success = importSavegameData(savegame, selectedFile, notMultiple);
                delete picture;
                if (!success)
                    delete savegame;
#ifdef GTA5SYNC_TELEMETRY
                if (success && notMultiple) {
                    QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
                    telemetrySettings.beginGroup("Telemetry");
                    bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
                    telemetrySettings.endGroup();
                    if (pushUsageData && Telemetry->canPush()) {
                        QJsonDocument jsonDocument;
                        QJsonObject jsonObject;
                        jsonObject["Type"] = "ImportSuccess";
#if QT_VERSION >= 0x060000
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                        jsonObject["ImportTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                        jsonObject["ImportType"] = "Savegame";
                        jsonDocument.setObject(jsonObject);
                        Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
                    }
                }
#endif
                return success;
            }
            else {
#ifdef GTA5SYNC_DEBUG
                qDebug() << "ImportError SnapmaticPicture" << picture->getLastStep();
                qDebug() << "ImportError SavegameData" << savegame->getLastStep();
#endif
                delete picture;
                delete savegame;
                if (notMultiple) QMessageBox::warning(this, tr("Import..."), tr("Can't import %1 because file format can't be detected").arg("\""+selectedFileName+"\""));
                return false;
            }
        }
    }
    if (notMultiple)
        QMessageBox::warning(this, tr("Import..."), tr("No valid file is selected"));
    return false;
}

bool ProfileInterface::importUrls(const QMimeData *mimeData)
{
    QStringList pathList;

    for (const QUrl &currentUrl : mimeData->urls()) {
        if (currentUrl.isLocalFile())
            pathList += currentUrl.toLocalFile();
    }

    if (pathList.length() == 1) {
        QString selectedFile = pathList.at(0);
        return importFile(selectedFile, QDateTime::currentDateTime(), true);
    }
    else if (pathList.length() > 1) {
        return importFilesProgress(pathList);
    }
    return false;
}

bool ProfileInterface::importRemote(QUrl remoteUrl)
{
    bool retValue = false;
    QDialog urlPasteDialog(this);
#if QT_VERSION >= 0x050000
    urlPasteDialog.setObjectName(QStringLiteral("UrlPasteDialog"));
#else
    urlPasteDialog.setObjectName(QString::fromUtf8("UrlPasteDialog"));
#endif
    urlPasteDialog.setWindowFlags(urlPasteDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
    urlPasteDialog.setWindowTitle(tr("Import..."));
    urlPasteDialog.setModal(true);
    QVBoxLayout urlPasteLayout(&urlPasteDialog);
#if QT_VERSION >= 0x050000
    urlPasteLayout.setObjectName(QStringLiteral("UrlPasteLayout"));
#else
    urlPasteLayout.setObjectName(QString::fromUtf8("UrlPasteLayout"));
#endif
    urlPasteDialog.setLayout(&urlPasteLayout);
    UiModLabel urlPasteLabel(&urlPasteDialog);
#if QT_VERSION >= 0x050000
    urlPasteLabel.setObjectName(QStringLiteral("UrlPasteLabel"));
#else
    urlPasteLabel.setObjectName(QString::fromUtf8("UrlPasteLabel"));
#endif

    urlPasteLabel.setText(tr("Prepare Content for Import..."));
    urlPasteLayout.addWidget(&urlPasteLabel);
    urlPasteDialog.setFixedSize(urlPasteDialog.sizeHint());
    urlPasteDialog.show();

    QNetworkAccessManager *netManager = new QNetworkAccessManager();
    QNetworkRequest netRequest(remoteUrl);
    netRequest.setRawHeader("User-Agent", AppEnv::getUserAgent());
    netRequest.setRawHeader("Accept", "text/html");
    netRequest.setRawHeader("Accept-Charset", "utf-8");
    netRequest.setRawHeader("Accept-Language", "en-US,en;q=0.9");
    netRequest.setRawHeader("Connection", "keep-alive");
    QNetworkReply *netReply = netManager->get(netRequest);
    QEventLoop *downloadLoop = new QEventLoop();
    QObject::connect(netReply, SIGNAL(finished()), downloadLoop, SLOT(quit()));
    QTimer::singleShot(30000, downloadLoop, SLOT(quit()));
    downloadLoop->exec();
    downloadLoop->disconnect();
    delete downloadLoop;

    urlPasteDialog.close();

    if (netReply->isFinished()) {
        QImage *snapmaticImage = new QImage();
        QImageReader snapmaticImageReader;
        snapmaticImageReader.setDecideFormatFromContent(true);
        snapmaticImageReader.setDevice(netReply);
        if (snapmaticImageReader.read(snapmaticImage)) {
            retValue = importImage(snapmaticImage, QDateTime::currentDateTime());
        }
        else {
            delete snapmaticImage;
        }
    }
    else {
        netReply->abort();
    }
    delete netReply;
    delete netManager;
    return retValue;
}

bool ProfileInterface::importImage(QImage *snapmaticImage, QDateTime importDateTime)
{
    SnapmaticPicture *picture = new SnapmaticPicture(":/template/template.g5e");
    if (picture->readingPicture(false)) {
        bool success = false;
        ImportDialog *importDialog = new ImportDialog(profileName, this);
        importDialog->setImage(snapmaticImage);
        importDialog->setModal(true);
        importDialog->show();
        importDialog->exec();
        if (importDialog->isImportAgreed()) {
            if (picture->setImage(importDialog->image(), importDialog->isUnlimitedBuffer())) {
                SnapmaticProperties spJson = picture->getSnapmaticProperties();
                spJson.uid = getRandomUid();
                bool fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                bool fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".bak");
                bool fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                int cEnough = 0;
                while ((fExists || fExistsBackup || fExistsHidden) && cEnough < findRetryLimit) {
                    spJson.uid = getRandomUid();
                    fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid));
                    fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".bak");
                    fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(spJson.uid) % ".hidden");
                    cEnough++;
                }
                spJson.createdDateTime = importDateTime;
#if QT_VERSION >= 0x060000
                quint64 timestamp = spJson.createdDateTime.toSecsSinceEpoch();
                if (timestamp > UINT32_MAX) {
                    timestamp = UINT32_MAX;
                }
                spJson.createdTimestamp = (quint32)timestamp;
#else
                spJson.createdTimestamp = spJson.createdDateTime.toTime_t();
#endif
                picture->setSnapmaticProperties(spJson);
                const QString picFileName = QString("PGTA5%1").arg(QString::number(spJson.uid));
                picture->setPicFileName(picFileName);
                picture->setPictureTitle(importDialog->getImageTitle());
                picture->updateStrings();
                success = importSnapmaticPicture(picture, true);
            }
        }
        else {
            delete picture;
            success = true;
        }
        delete importDialog;
        if (!success)
            delete picture;
        return success;
    }
    else {
        delete picture;
        return false;
    }
}

bool ProfileInterface::importSnapmaticPicture(SnapmaticPicture *picture, bool warn)
{
    QString picFileName = picture->getPictureFileName();
    QString adjustedFileName = picture->getOriginalPictureFileName();
    if (!picFileName.startsWith("PGTA5")) {
        if (warn)
            QMessageBox::warning(this, tr("Import..."), tr("Failed to import the Snapmatic picture, file not begin with PGTA or end with .g5e"));
        return false;
    }
    else if (QFile::exists(profileFolder % "/" % adjustedFileName) || QFile::exists(profileFolder % "/" % adjustedFileName % ".hidden")) {
        SnapmaticProperties snapmaticProperties = picture->getSnapmaticProperties();
        if (warn) {
            int uchoice = QMessageBox::question(this, tr("Import..."), tr("A Snapmatic picture already exists with the uid %1, you want assign your import a new uid and timestamp?").arg(QString::number(snapmaticProperties.uid)), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (uchoice == QMessageBox::Yes) {
                // Update Snapmatic uid
                snapmaticProperties.uid = getRandomUid();
                snapmaticProperties.createdDateTime = QDateTime::currentDateTime();
#if QT_VERSION >= 0x060000
                quint64 timestamp = snapmaticProperties.createdDateTime.toSecsSinceEpoch();
                if (timestamp > UINT32_MAX) {
                    timestamp = UINT32_MAX;
                }
                snapmaticProperties.createdTimestamp = (quint32)timestamp;
#else
                snapmaticProperties.createdTimestamp = snapmaticProperties.createdDateTime.toTime_t();
#endif
                bool fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid));
                bool fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".bak");
                bool fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".hidden");
                int cEnough = 0;
                while ((fExists || fExistsBackup || fExistsHidden) && cEnough < findRetryLimit) {
                    snapmaticProperties.uid = getRandomUid();
                    fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid));
                    fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".bak");
                    fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".hidden");
                    cEnough++;
                }
                if (fExists || fExistsBackup || fExistsHidden) {
                    // That should never happen
                    return false;
                }
                if (!picture->setSnapmaticProperties(snapmaticProperties)) {
                    // That should never happen
                    return false;
                }
                picture->updateStrings();
                picFileName = picture->getPictureFileName();
                adjustedFileName = picture->getOriginalPictureFileName();
            }
            else {
                return false;
            }
        }
        else {
            // Update Snapmatic uid
            snapmaticProperties.uid = getRandomUid();
            snapmaticProperties.createdDateTime = QDateTime::currentDateTime();
#if QT_VERSION >= 0x060000
            quint64 timestamp = snapmaticProperties.createdDateTime.toSecsSinceEpoch();
            if (timestamp > UINT32_MAX) {
                timestamp = UINT32_MAX;
            }
            snapmaticProperties.createdTimestamp = (quint32)timestamp;
#else
            snapmaticProperties.createdTimestamp = snapmaticProperties.createdDateTime.toTime_t();
#endif
            bool fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid));
            bool fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".bak");
            bool fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".hidden");
            int cEnough = 0;
            while ((fExists || fExistsBackup || fExistsHidden) && cEnough < findRetryLimit) {
                snapmaticProperties.uid = getRandomUid();
                fExists = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid));
                fExistsBackup = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".bak");
                fExistsHidden = QFile::exists(profileFolder % "/PGTA5" % QString::number(snapmaticProperties.uid) % ".hidden");
                cEnough++;
            }
            if (fExists || fExistsBackup || fExistsHidden) {
                // That should never happen
                return false;
            }
            if (!picture->setSnapmaticProperties(snapmaticProperties)) {
                // That should never happen
                return false;
            }
            picture->updateStrings();
            picFileName = picture->getPictureFileName();
            adjustedFileName = picture->getOriginalPictureFileName();
        }
    }
    if (picture->exportPicture(profileFolder % "/" % adjustedFileName, SnapmaticFormat::PGTA_Format)) {
        picture->setSnapmaticFormat(SnapmaticFormat::PGTA_Format);
        picture->setPicFilePath(profileFolder % "/" % adjustedFileName);
#if QT_VERSION >= 0x050000
        snapmaticPics << picture->getPictureFileName();
#endif
        pictureLoaded(picture, true);
        return true;
    }
    else {
        if (warn)
            QMessageBox::warning(this, tr("Import..."), tr("Failed to import the Snapmatic picture, can't copy the file into profile"));
        return false;
    }
}

bool ProfileInterface::importSavegameData(SavegameData *savegame, QString sgdPath, bool warn)
{
    QString sgdFileName;
    bool foundFree = 0;
    int currentSgd = 0;

    while (currentSgd < 15 && !foundFree) {
        QString sgdNumber = QString::number(currentSgd);
        if (sgdNumber.length() == 1) {
            sgdNumber.insert(0, "0");
        }
        sgdFileName = "SGTA500" % sgdNumber;

        if (!QFile::exists(profileFolder % "/" % sgdFileName)) {
            foundFree = true;
        }
        currentSgd++;
    }

    if (foundFree) {
        const QString newSgdPath = profileFolder % "/" % sgdFileName;
        if (QFile::copy(sgdPath, newSgdPath)) {
            savegame->setSavegameFileName(newSgdPath);
#if QT_VERSION >= 0x050000
            savegameFiles << newSgdPath;
#endif
            savegameLoaded(savegame, newSgdPath, true);
            return true;
        }
        else {
            if (warn)
                QMessageBox::warning(this, tr("Import..."), tr("Failed to import the Savegame, can't copy the file into profile"));
            return false;
        }
    }
    else {
        if (warn)
            QMessageBox::warning(this, tr("Import..."), tr("Failed to import the Savegame, no Savegame slot is left"));
        return false;
    }
}

void ProfileInterface::profileWidgetSelected()
{
    if (selectedWidgts == 0) {
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr)
                widget->setSelectionMode(true);
        }
    }
    selectedWidgts++;
}

void ProfileInterface::profileWidgetDeselected()
{
    if (selectedWidgts == 1) {
        int scrollBarValue = ui->saProfile->verticalScrollBar()->value();
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr && contentMode != 2) {
                widget->setSelectionMode(false);
            }
        }
        ui->saProfile->verticalScrollBar()->setValue(scrollBarValue);
    }
    selectedWidgts--;
}

void ProfileInterface::selectAllWidgets()
{
    for (const QString &widgetStr : qAsConst(widgets)) {
        ProfileWidget *widget = widgets.key(widgetStr, nullptr);
        if (widget != nullptr)
            widget->setSelected(true);
    }
}

void ProfileInterface::deselectAllWidgets()
{
    for (const QString &widgetStr : qAsConst(widgets)) {
        ProfileWidget *widget = widgets.key(widgetStr, nullptr);
        if (widget != nullptr)
            widget->setSelected(false);
    }
}

void ProfileInterface::exportSelected()
{
    if (selectedWidgts != 0) {
        int exportCount = 0;
        int exportPictures = 0;
        int exportSavegames = 0;
        bool pictureCopyEnabled = false;
        bool pictureExportEnabled = false;

        QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
        settings.beginGroup("FileDialogs");
        //bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
        settings.beginGroup("ExportDirectory");
        QString exportDirectory = QFileDialog::getExistingDirectory(this, tr("Export selected..."), settings.value(profileName, profileFolder).toString());
        if (exportDirectory != "") {
            settings.setValue(profileName, exportDirectory);
            for (const QString &widgetStr : qAsConst(widgets)) {
                ProfileWidget *widget = widgets.key(widgetStr, nullptr);
                if (widget != nullptr) {
                    if (widget->isSelected()) {
                        if (widget->getWidgetType() == "SnapmaticWidget") {
                            exportPictures++;
                        }
                        else if (widget->getWidgetType() == "SavegameWidget") {
                            exportSavegames++;
                        }
                    }
                }
            }

            if (exportPictures != 0) {
                QInputDialog inputDialog;
                QStringList inputDialogItems;
                inputDialogItems << tr("JPG pictures and GTA Snapmatic");
                inputDialogItems << tr("JPG pictures only");
                inputDialogItems << tr("GTA Snapmatic only");

                QString ExportPreSpan;
                QString ExportPostSpan;
#ifdef Q_OS_WIN
                ExportPreSpan = "<span style=\"color:#003399;font-size:12pt\">";
                ExportPostSpan = "</span>";
#else
                ExportPreSpan = "<span style=\"font-weight:bold\">";
                ExportPostSpan = "</span>";
#endif

                bool itemSelected = false;
                QString selectedItem = inputDialog.getItem(this, tr("Export selected..."), tr("%1Export Snapmatic pictures%2<br><br>JPG pictures make it possible to open the picture with a Image Viewer<br>GTA Snapmatic make it possible to import the picture into the game<br><br>Export as:").arg(ExportPreSpan, ExportPostSpan), inputDialogItems, 0, false, &itemSelected, inputDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
                if (itemSelected) {
                    if (selectedItem == tr("JPG pictures and GTA Snapmatic")) {
                        pictureExportEnabled = true;
                        pictureCopyEnabled = true;
                    }
                    else if (selectedItem == tr("JPG pictures only")) {
                        pictureExportEnabled = true;
                    }
                    else if (selectedItem == tr("GTA Snapmatic only")) {
                        pictureCopyEnabled = true;
                    }
                    else {
                        pictureExportEnabled = true;
                        pictureCopyEnabled = true;
                    }
                }
                else {
                    // Don't export anymore when any Cancel button got clicked
                    settings.endGroup();
                    settings.endGroup();
                    return;
                }
            }

            // Counting the exports together
            exportCount = exportCount + exportSavegames;
            if (pictureExportEnabled && pictureCopyEnabled) {
                int exportPictures2 = exportPictures * 2;
                exportCount = exportCount + exportPictures2;
            }
            else {
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

            pbDialog.setAutoClose(false);
            pbDialog.exec();
            QStringList getFailedSavegames = exportThread->getFailedSavegames();
            QStringList getFailedCopyPictures = exportThread->getFailedCopyPictures();
            QStringList getFailedExportPictures = exportThread->getFailedExportPictures();

            QString errorStr;
            QStringList errorList;
            errorList << getFailedExportPictures;
            errorList << getFailedCopyPictures;
            errorList << getFailedSavegames;

            for (const QString &curErrorStr : qAsConst(errorList)) {
                errorStr += ", " % curErrorStr;
            }
            if (errorStr != "") {
                errorStr.remove(0, 2);
                QMessageBox::warning(this, tr("Export selected..."), tr("Export failed with...\n\n%1").arg(errorStr));
            }

            if (exportThread->isFinished()) {
                delete exportThread;
            }
            else {
                QEventLoop threadFinishLoop;
                QObject::connect(exportThread, SIGNAL(finished()), &threadFinishLoop, SLOT(quit()));
                threadFinishLoop.exec();
                delete exportThread;
            }
        }
        settings.endGroup();
        settings.endGroup();
    }
    else {
        QMessageBox::information(this, tr("Export selected..."), tr("No Snapmatic pictures or Savegames files are selected"));
    }
}

void ProfileInterface::deleteSelectedL(bool isRemoteEmited)
{
    if (selectedWidgts != 0) {
        if (QMessageBox::Yes == QMessageBox::warning(this, tr("Remove selected"), tr("You really want remove the selected Snapmatic picutres and Savegame files?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
            for (const QString &widgetStr : qAsConst(widgets)) {
                ProfileWidget *widget = widgets.key(widgetStr, nullptr);
                if (widget != nullptr) {
                    if (widget->isSelected()) {
                        if (widget->getWidgetType() == "SnapmaticWidget") {
                            SnapmaticWidget *picWidget = qobject_cast<SnapmaticWidget*>(widget);
                            if (picWidget->getPicture()->deletePictureFile()) {
                                pictureDeleted(picWidget, isRemoteEmited);
                            }
                        }
                        else if (widget->getWidgetType() == "SavegameWidget") {
                            SavegameWidget *sgdWidget = qobject_cast<SavegameWidget*>(widget);
                            SavegameData *savegame = sgdWidget->getSavegame();
                            QString fileName = savegame->getSavegameFileName();
                            if (!QFile::exists(fileName) || QFile::remove(fileName)) {
                                savegameDeleted(sgdWidget, isRemoteEmited);
                            }
                        }
                    }
                }
            }
            if (selectedWidgts != 0) {
                QMessageBox::warning(this, tr("Remove selected"), tr("Failed to remove all selected Snapmatic pictures and/or Savegame files"));
            }
        }
    }
    else {
        QMessageBox::information(this, tr("Remove selected"), tr("No Snapmatic pictures or Savegames files are selected"));
    }
}

void ProfileInterface::deleteSelected()
{
    deleteSelectedL(false);
}

void ProfileInterface::deleteSelectedR()
{
    deleteSelectedL(true);
}

void ProfileInterface::massToolQualify()
{
    massTool(MassTool::Qualify);
}

void ProfileInterface::massToolPlayers()
{
    massTool(MassTool::Players);
}

void ProfileInterface::massToolCrew()
{
    massTool(MassTool::Crew);
}

void ProfileInterface::massToolTitle()
{
    massTool(MassTool::Title);
}

void ProfileInterface::importFiles()
{
    on_cmdImport_clicked();
}

void ProfileInterface::settingsApplied(int _contentMode, bool languageChanged)
{
    if (languageChanged)
        retranslateUi();
    contentMode = _contentMode;
    if (contentMode == 2) {
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr) {
                widget->setSelectionMode(true);
                widget->setContentMode(contentMode);
                if (languageChanged)
                    widget->retranslate();
            }
        }
    }
    else {
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr) {
                if (selectedWidgts == 0) {
                    widget->setSelectionMode(false);
                }
                widget->setContentMode(contentMode);
                if (languageChanged)
                    widget->retranslate();
            }
        }
    }
#ifdef Q_OS_MAC
    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    if (QApplication::style()->objectName() == "macintosh") {
        ui->hlButtons->setSpacing(6 * screenRatio);
        ui->hlButtons->setContentsMargins(9 * screenRatio, 15 * screenRatio, 15 * screenRatio, 17 * screenRatio);
    }
    else {
        ui->hlButtons->setSpacing(6 * screenRatio);
        ui->hlButtons->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
    }
#endif
}

void ProfileInterface::enableSelected()
{
    QList<SnapmaticWidget*> snapmaticWidgets;
    for (const QString &widgetStr : qAsConst(widgets)) {
        ProfileWidget *widget = widgets.key(widgetStr, nullptr);
        if (widget != nullptr) {
            if (widget->isSelected()) {
                if (widget->getWidgetType() == "SnapmaticWidget") {
                    SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
                    snapmaticWidgets += snapmaticWidget;
                }
            }
        }
    }
    if (snapmaticWidgets.isEmpty()) {
        QMessageBox::information(this, QApplication::translate("UserInterface", "Show In-game"), QApplication::translate("ProfileInterface", "No Snapmatic pictures are selected"));
        return;
    }
    QStringList fails;
    for (SnapmaticWidget *widget : qAsConst(snapmaticWidgets)) {
        SnapmaticPicture *picture = widget->getPicture();
        if (!widget->makePictureVisible()) {
            fails << QString("%1 [%2]").arg(picture->getPictureTitle(), picture->getPictureString());
        }
    }
    if (!fails.isEmpty()) {
        QMessageBox::warning(this, QApplication::translate("UserInterface", "Show In-game"), QApplication::translate("ProfileInterface", "%1 failed with...\n\n%2", "Action failed with...").arg(QApplication::translate("UserInterface", "Show In-game"), fails.join(", ")));
    }
}

void ProfileInterface::disableSelected()
{
    QList<SnapmaticWidget*> snapmaticWidgets;
    for (const QString &widgetStr : qAsConst(widgets)) {
        ProfileWidget *widget = widgets.key(widgetStr, nullptr);
        if (widget != nullptr) {
            if (widget->isSelected()) {
                if (widget->getWidgetType() == "SnapmaticWidget") {
                    SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
                    snapmaticWidgets += snapmaticWidget;
                }
            }
        }
    }
    if (snapmaticWidgets.isEmpty()) {
        QMessageBox::information(this, QApplication::translate("UserInterface", "Hide In-game"), QApplication::translate("ProfileInterface", "No Snapmatic pictures are selected"));
        return;
    }
    QStringList fails;
    for (SnapmaticWidget *widget : qAsConst(snapmaticWidgets)) {
        SnapmaticPicture *picture = widget->getPicture();
        if (!widget->makePictureHidden()) {
            fails << QString("%1 [%2]").arg(picture->getPictureTitle(), picture->getPictureString());
        }
    }
    if (!fails.isEmpty()) {
        QMessageBox::warning(this, QApplication::translate("UserInterface", "Hide In-game"), QApplication::translate("ProfileInterface", "%1 failed with...\n\n%2", "Action failed with...").arg(QApplication::translate("UserInterface", "Hide In-game"), fails.join(", ")));
    }
}

int ProfileInterface::selectedWidgets()
{
    return selectedWidgts;
}

void ProfileInterface::contextMenuTriggeredPIC(QContextMenuEvent *ev)
{
    SnapmaticWidget *picWidget = qobject_cast<SnapmaticWidget*>(sender());
    if (picWidget != previousWidget) {
        if (previousWidget != nullptr) {
            previousWidget->setStyleSheet(QLatin1String(""));
        }
        picWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color:palette(highlight)}QLabel#labPicStr{color:palette(highlighted-text)}"));
        previousWidget = picWidget;
    }
    QMenu contextMenu(picWidget);
    const int selectedCount = selectedWidgets();
    if (contentMode < 20 || selectedCount == 0) {
        QMenu editMenu(SnapmaticWidget::tr("Edi&t"), picWidget);
        if (picWidget->isHidden()) {
            editMenu.addAction(SnapmaticWidget::tr("Show &In-game"), picWidget, SLOT(makePictureVisibleSlot()));
        }
        else {
            editMenu.addAction(SnapmaticWidget::tr("Hide &In-game"), picWidget, SLOT(makePictureHiddenSlot()));
        }
        editMenu.addAction(PictureDialog::tr("&Edit Properties..."), picWidget, SLOT(editSnapmaticProperties()));
        editMenu.addAction(PictureDialog::tr("&Overwrite Image..."), picWidget, SLOT(editSnapmaticImage()));
        editMenu.addSeparator();
        editMenu.addAction(PictureDialog::tr("Open &Map Viewer..."), picWidget, SLOT(openMapViewer()));
        editMenu.addAction(PictureDialog::tr("Open &JSON Editor..."), picWidget, SLOT(editSnapmaticRawJson()));
        QMenu exportMenu(SnapmaticWidget::tr("&Export"), this);
        exportMenu.addAction(PictureDialog::tr("Export as &Picture..."), picWidget, SLOT(on_cmdExport_clicked()));
        exportMenu.addAction(PictureDialog::tr("Export as &Snapmatic..."), picWidget, SLOT(on_cmdCopy_clicked()));
        contextMenu.addAction(SnapmaticWidget::tr("&View"), picWidget, SLOT(on_cmdView_clicked()));
        contextMenu.addMenu(&editMenu);
        contextMenu.addMenu(&exportMenu);
        contextMenu.addAction(SnapmaticWidget::tr("&Remove"), picWidget, SLOT(on_cmdDelete_clicked()));
        contextMenu.addSeparator();
        if (!picWidget->isSelected())
            contextMenu.addAction(SnapmaticWidget::tr("&Select"), picWidget, SLOT(pictureSelected()));
        else {
            contextMenu.addAction(SnapmaticWidget::tr("&Deselect"), picWidget, SLOT(pictureSelected()));
        }
        if (selectedCount != widgets.count()) {
            contextMenu.addAction(SnapmaticWidget::tr("Select &All"), picWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
        }
        if (selectedCount != 0) {
            contextMenu.addAction(SnapmaticWidget::tr("&Deselect All"), picWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
        }
        contextMenuOpened = true;
        contextMenu.exec(ev->globalPos());
        contextMenuOpened = false;
        QTimer::singleShot(0, this, SLOT(hoverProfileWidgetCheck()));
    }
    else {
        QMenu editMenu(SnapmaticWidget::tr("Edi&t"), picWidget);
        editMenu.addAction(QApplication::translate("UserInterface", "&Qualify as Avatar"), this, SLOT(massToolQualify()), QKeySequence::fromString("Shift+Q"));
        editMenu.addAction(QApplication::translate("UserInterface", "Change &Players..."), this, SLOT(massToolPlayers()), QKeySequence::fromString("Shift+P"));
        editMenu.addAction(QApplication::translate("UserInterface", "Change &Crew..."), this, SLOT(massToolCrew()), QKeySequence::fromString("Shift+C"));
        editMenu.addAction(QApplication::translate("UserInterface", "Change &Title..."), this, SLOT(massToolTitle()), QKeySequence::fromString("Shift+T"));
        editMenu.addSeparator();
        editMenu.addAction(SnapmaticWidget::tr("Show &In-game"), this, SLOT(enableSelected()), QKeySequence::fromString("Shift+E"));
        editMenu.addAction(SnapmaticWidget::tr("Hide &In-game"), this, SLOT(disableSelected()), QKeySequence::fromString("Shift+D"));
        contextMenu.addMenu(&editMenu);
        contextMenu.addAction(SavegameWidget::tr("&Export"), this, SLOT(exportSelected()), QKeySequence::fromString("Ctrl+E"));
        contextMenu.addAction(SavegameWidget::tr("&Remove"), this, SLOT(deleteSelectedR()), QKeySequence::fromString("Ctrl+Del"));
        contextMenu.addSeparator();
        if (!picWidget->isSelected()) {
            contextMenu.addAction(SnapmaticWidget::tr("&Select"), picWidget, SLOT(pictureSelected()));
        }
        else {
            contextMenu.addAction(SnapmaticWidget::tr("&Deselect"), picWidget, SLOT(pictureSelected()));
        }
        if (selectedCount != widgets.count()) {
            contextMenu.addAction(SnapmaticWidget::tr("Select &All"), picWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
        }
        if (selectedCount != 0) {
            contextMenu.addAction(SnapmaticWidget::tr("&Deselect All"), picWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
        }
        contextMenuOpened = true;
        contextMenu.exec(ev->globalPos());
        contextMenuOpened = false;
        QTimer::singleShot(0, this, SLOT(hoverProfileWidgetCheck()));
    }
}

void ProfileInterface::contextMenuTriggeredSGD(QContextMenuEvent *ev)
{
    SavegameWidget *sgdWidget = qobject_cast<SavegameWidget*>(sender());
    if (sgdWidget != previousWidget) {
        if (previousWidget != nullptr) {
            previousWidget->setStyleSheet(QLatin1String(""));
        }
        sgdWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color:palette(highlight)}QLabel#labSavegameStr{color:palette(highlighted-text)}"));
        previousWidget = sgdWidget;
    }
    QMenu contextMenu(sgdWidget);
    const int selectedCount = selectedWidgets();
    if (contentMode < 20 || selectedCount == 0) {
        contextMenu.addAction(SavegameWidget::tr("&View"), sgdWidget, SLOT(on_cmdView_clicked()));
        contextMenu.addAction(SavegameWidget::tr("&Export"), sgdWidget, SLOT(on_cmdCopy_clicked()));
        contextMenu.addAction(SavegameWidget::tr("&Remove"), sgdWidget, SLOT(on_cmdDelete_clicked()));
        contextMenu.addSeparator();
        if (!sgdWidget->isSelected()) {
            contextMenu.addAction(SavegameWidget::tr("&Select"), sgdWidget, SLOT(savegameSelected()));
        }
        else {
            contextMenu.addAction(SavegameWidget::tr("&Deselect"), sgdWidget, SLOT(savegameSelected()));
        }
        if (selectedCount != widgets.count()) {
            contextMenu.addAction(SavegameWidget::tr("Select &All"), sgdWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
        }
        if (selectedCount != 0) {
            contextMenu.addAction(SavegameWidget::tr("&Deselect All"), sgdWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
        }
        contextMenuOpened = true;
        contextMenu.exec(ev->globalPos());
        contextMenuOpened = false;
        QTimer::singleShot(0, this, SLOT(hoverProfileWidgetCheck()));
    }
    else {
        QMenu editMenu(SnapmaticWidget::tr("Edi&t"), sgdWidget);
        editMenu.addAction(QApplication::translate("UserInterface", "&Qualify as Avatar"), this, SLOT(massToolQualify()), QKeySequence::fromString("Shift+Q"));
        editMenu.addAction(QApplication::translate("UserInterface", "Change &Players..."), this, SLOT(massToolPlayers()), QKeySequence::fromString("Shift+P"));
        editMenu.addAction(QApplication::translate("UserInterface", "Change &Crew..."), this, SLOT(massToolCrew()), QKeySequence::fromString("Shift+C"));
        editMenu.addAction(QApplication::translate("UserInterface", "Change &Title..."), this, SLOT(massToolTitle()), QKeySequence::fromString("Shift+T"));
        editMenu.addSeparator();
        editMenu.addAction(SnapmaticWidget::tr("Show &In-game"), this, SLOT(enableSelected()), QKeySequence::fromString("Shift+E"));
        editMenu.addAction(SnapmaticWidget::tr("Hide &In-game"), this, SLOT(disableSelected()), QKeySequence::fromString("Shift+D"));
        contextMenu.addMenu(&editMenu);
        contextMenu.addAction(SavegameWidget::tr("&Export"), this, SLOT(exportSelected()), QKeySequence::fromString("Ctrl+E"));
        contextMenu.addAction(SavegameWidget::tr("&Remove"), this, SLOT(deleteSelectedR()), QKeySequence::fromString("Ctrl+Del"));
        contextMenu.addSeparator();
        if (!sgdWidget->isSelected())
            contextMenu.addAction(SavegameWidget::tr("&Select"), sgdWidget, SLOT(savegameSelected()));
        else {
            contextMenu.addAction(SavegameWidget::tr("&Deselect"), sgdWidget, SLOT(savegameSelected()));
        }
        if (selectedCount != widgets.count()) {
            contextMenu.addAction(SavegameWidget::tr("Select &All"), sgdWidget, SLOT(selectAllWidgets()), QKeySequence::fromString("Ctrl+A"));
        }
        if (selectedCount != 0) {
            contextMenu.addAction(SavegameWidget::tr("&Deselect All"), sgdWidget, SLOT(deselectAllWidgets()), QKeySequence::fromString("Ctrl+D"));
        }
        contextMenuOpened = true;
        contextMenu.exec(ev->globalPos());
        contextMenuOpened = false;
        QTimer::singleShot(0, this, SLOT(hoverProfileWidgetCheck()));
    }
}

void ProfileInterface::on_saProfileContent_dropped(const QMimeData *mimeData)
{
    if (!mimeData)
        return;
    if (mimeData->hasImage()) {
        QImage *snapmaticImage = new QImage(qvariant_cast<QImage>(mimeData->imageData()));
        importImage(snapmaticImage, QDateTime::currentDateTime());
    }
    else if (mimeData->hasUrls()) {
        importUrls(mimeData);
    }
}

void ProfileInterface::retranslateUi()
{
    ui->retranslateUi(this);
    QString appVersion = GTA5SYNC_APPVER;
#ifndef GTA5SYNC_BUILDTYPE_REL
#ifdef GTA5SYNC_COMMIT
    if (!appVersion.contains("-"))
        appVersion = appVersion % "-" % GTA5SYNC_COMMIT;
#endif
#endif
    ui->labVersion->setText(QString("%1 %2").arg(GTA5SYNC_APPSTR, appVersion));
}

bool ProfileInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        if (isProfileLoaded) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
            case Qt::Key_V:
                if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && !QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    const QMimeData *clipboardData = QApplication::clipboard()->mimeData();
                    if (clipboardData->hasImage()) {
                        QImage *snapmaticImage = new QImage(qvariant_cast<QImage>(clipboardData->imageData()));
                        importImage(snapmaticImage, QDateTime::currentDateTime());
                    }
                    else if (clipboardData->hasUrls()) {
                        if (clipboardData->urls().length() >= 2) {
                            importUrls(clipboardData);
                        }
                        else if (clipboardData->urls().length() == 1) {
                            QUrl clipboardUrl = clipboardData->urls().at(0);
                            if (clipboardUrl.isLocalFile()) {
                                importFile(clipboardUrl.toLocalFile(), QDateTime::currentDateTime(), true);
                            }
                            else {
                                importRemote(clipboardUrl);
                            }
                        }
                    }
                    else if (clipboardData->hasText()) {
                        QUrl clipboardUrl = QUrl::fromUserInput(clipboardData->text());
                        if (clipboardUrl.isValid()) {
                            if (clipboardUrl.isLocalFile()) {
                                importFile(clipboardUrl.toLocalFile(), QDateTime::currentDateTime(), true);
                            }
                            else {
                                importRemote(clipboardUrl);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (event->type() == QEvent::MouseMove) {
        if ((watched->objectName() == "SavegameWidget" || watched->objectName() == "SnapmaticWidget") && isProfileLoaded) {
            ProfileWidget *pWidget = qobject_cast<ProfileWidget*>(watched);
            if (pWidget->underMouse()) {
                bool styleSheetChanged = false;
                if (pWidget->getWidgetType() == "SnapmaticWidget") {
                    if (pWidget != previousWidget) {
                        pWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color:palette(highlight)}QLabel#labPicStr{color:palette(highlighted-text)}"));
                        styleSheetChanged = true;
                    }
                }
                else if (pWidget->getWidgetType() == "SavegameWidget") {
                    if (pWidget != previousWidget) {
                        pWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color:palette(highlight)}QLabel#labSavegameStr{color:palette(highlighted-text)}"));
                        styleSheetChanged = true;
                    }
                }
                if (styleSheetChanged) {
                    if (previousWidget != nullptr) {
                        previousWidget->setStyleSheet(QLatin1String(""));
                    }
                    previousWidget = pWidget;
                }
            }
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        if ((watched->objectName() == "SavegameWidget" || watched->objectName() == "SnapmaticWidget") && isProfileLoaded) {
            ProfileWidget *pWidget = nullptr;
            for (auto it = widgets.constBegin(); it != widgets.constEnd(); it++) {
                ProfileWidget *widget = it.key();
                QPoint mousePos = widget->mapFromGlobal(QCursor::pos());
                if (widget->rect().contains(mousePos)) {
                    pWidget = widget;
                    break;
                }
            }
            if (pWidget != nullptr) {
                bool styleSheetChanged = false;
                if (pWidget->getWidgetType() == "SnapmaticWidget") {
                    if (pWidget != previousWidget) {
                        pWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color:palette(highlight)}QLabel#labPicStr{color:palette(highlighted-text)}"));
                        styleSheetChanged = true;
                    }
                }
                else if (pWidget->getWidgetType() == "SavegameWidget") {
                    if (pWidget != previousWidget) {
                        pWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color:palette(highlight)}QLabel#labSavegameStr{color:palette(highlighted-text)}"));
                        styleSheetChanged = true;
                    }
                }
                if (styleSheetChanged) {
                    if (previousWidget != nullptr) {
                        previousWidget->setStyleSheet(QLatin1String(""));
                    }
                    previousWidget = pWidget;
                }
            }
        }
    }
    else if (event->type() == QEvent::WindowDeactivate && isProfileLoaded) {
        if (previousWidget != nullptr && watched == previousWidget) {
            previousWidget->setStyleSheet(QLatin1String(""));
            previousWidget = nullptr;
        }
    }
    else if (event->type() == QEvent::Leave && isProfileLoaded && !contextMenuOpened) {
        if (watched->objectName() == "SavegameWidget" || watched->objectName() == "SnapmaticWidget") {
            ProfileWidget *pWidget = qobject_cast<ProfileWidget*>(watched);
            QPoint mousePos = pWidget->mapFromGlobal(QCursor::pos());
            if (!pWidget->geometry().contains(mousePos)) {
                if (previousWidget != nullptr) {
                    previousWidget->setStyleSheet(QLatin1String(""));
                    previousWidget = nullptr;
                }
            }
        }
        else if (watched->objectName() == "ProfileInterface") {
            if (previousWidget != nullptr) {
                previousWidget->setStyleSheet(QLatin1String(""));
                previousWidget = nullptr;
            }
        }
    }
    return false;
}

void ProfileInterface::hoverProfileWidgetCheck()
{
    ProfileWidget *pWidget = nullptr;
    for (auto it = widgets.constBegin(); it != widgets.constEnd(); it++) {
        ProfileWidget *widget = it.key();
        if (widget->underMouse()) {
            pWidget = widget;
            break;
        }
    }
    if (pWidget != nullptr) {
        bool styleSheetChanged = false;
        if (pWidget->getWidgetType() == "SnapmaticWidget") {
            if (pWidget != previousWidget) {
                pWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color:palette(highlight)}QLabel#labPicStr{color:palette(highlighted-text)}"));
                styleSheetChanged = true;
            }
        }
        else if (pWidget->getWidgetType() == "SavegameWidget") {
            if (pWidget != previousWidget) {
                pWidget->setStyleSheet(QString("QFrame#SavegameFrame{background-color:palette(highlight)}QLabel#labSavegameStr{color:palette(highlighted-text)}"));
                styleSheetChanged = true;
            }
        }
        if (styleSheetChanged) {
            if (previousWidget != nullptr) {
                previousWidget->setStyleSheet(QLatin1String(""));
            }
            previousWidget = pWidget;
        }
    }
    else {
        if (previousWidget != nullptr) {
            previousWidget->setStyleSheet(QLatin1String(""));
            previousWidget = nullptr;
        }
    }
}

void ProfileInterface::updatePalette()
{
    ui->saProfile->setStyleSheet(QString("QWidget#saProfileContent{background-color:palette(base)}"));
    if (previousWidget != nullptr) {
        if (previousWidget->getWidgetType() == "SnapmaticWidget") {
            previousWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color:palette(highlight)}QLabel#labPicStr{color:palette(highlighted-text)}"));
        }
        else if (previousWidget->getWidgetType() == "SavegameWidget") {
            previousWidget->setStyleSheet(QString("QFrame#SnapmaticFrame{background-color:palette(highlight)}QLabel#labPicStr{color:palette(highlighted-text)}"));
        }
    }
}

bool ProfileInterface::isSupportedImageFile(QString selectedFileName)
{
    for (const QByteArray &imageFormat : QImageReader::supportedImageFormats()) {
        QString imageFormatStr = QString(".") % QString::fromUtf8(imageFormat).toLower();
        if (selectedFileName.length() >= imageFormatStr.length() && selectedFileName.toLower().right(imageFormatStr.length()) == imageFormatStr) {
            return true;
        }
    }
    return false;
}

void ProfileInterface::massTool(MassTool tool)
{
    switch(tool) {
    case MassTool::Qualify: {
        QList<SnapmaticWidget*> snapmaticWidgets;
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr) {
                if (widget->isSelected()) {
                    if (widget->getWidgetType() == "SnapmaticWidget") {
                        SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
                        snapmaticWidgets += snapmaticWidget;
                    }
                }
            }
        }

        if (snapmaticWidgets.isEmpty()) {
            QMessageBox::information(this, tr("Qualify as Avatar"), tr("No Snapmatic pictures are selected"));
            return;
        }

        // Prepare Progress

        int maximumId = snapmaticWidgets.length();
        int overallId = 0;

        QProgressDialog pbDialog(this);
        pbDialog.setWindowFlags(pbDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
        pbDialog.setWindowTitle(tr("Patch selected..."));
        pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(1), QString::number(maximumId)));
        pbDialog.setRange(1, maximumId);
        pbDialog.setValue(1);
        pbDialog.setModal(true);
        QList<QPushButton*> pbBtn = pbDialog.findChildren<QPushButton*>();
        pbBtn.at(0)->setDisabled(true);
        QList<QProgressBar*> pbBar = pbDialog.findChildren<QProgressBar*>();
        pbBar.at(0)->setTextVisible(false);
        pbDialog.setAutoClose(false);
        pbDialog.show();

        // Begin Progress

        QStringList fails;
        for (SnapmaticWidget *snapmaticWidget : qAsConst(snapmaticWidgets)) {
            // Update Progress
            overallId++;
            pbDialog.setValue(overallId);
            pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));

            SnapmaticPicture *picture = snapmaticWidget->getPicture();

            SnapmaticProperties snapmaticProperties = picture->getSnapmaticProperties();
            snapmaticProperties.isSelfie = true;
            snapmaticProperties.isMug = false;
            snapmaticProperties.isFromRSEditor = false;
            snapmaticProperties.isFromDirector = false;
            snapmaticProperties.isMeme = false;

            QString currentFilePath = picture->getPictureFilePath();
            QString originalFilePath = picture->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName)) {
                QFile::copy(currentFilePath, backupFileName);
            }
            SnapmaticProperties fallbackProperties = picture->getSnapmaticProperties();
            picture->setSnapmaticProperties(snapmaticProperties);
            if (!picture->exportPicture(currentFilePath)) {
                picture->setSnapmaticProperties(fallbackProperties);
                fails << QString("%1 [%2]").arg(picture->getPictureTitle(), picture->getPictureString());
            }
            else {
                picture->emitUpdate();
                QApplication::processEvents();
            }
        }
        pbDialog.close();
        if (!fails.isEmpty()) {
            QMessageBox::warning(this, tr("Qualify as Avatar"), tr("%1 failed with...\n\n%2", "Action failed with...").arg(tr("Qualify", "%1 failed with..."), fails.join(", ")));
        }
    }
        break;
    case MassTool::Players: {
        QList<SnapmaticWidget*> snapmaticWidgets;
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr) {
                if (widget->isSelected()) {
                    if (widget->getWidgetType() == "SnapmaticWidget") {
                        SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
                        snapmaticWidgets += snapmaticWidget;
                    }
                }
            }
        }

        if (snapmaticWidgets.isEmpty()) {
            QMessageBox::information(this, tr("Change Players..."), tr("No Snapmatic pictures are selected"));
            return;
        }

        QStringList players;
        if (snapmaticWidgets.length() == 1) {
            players = snapmaticWidgets.at(0)->getPicture()->getSnapmaticProperties().playersList;
        }

        PlayerListDialog *playerListDialog = new PlayerListDialog(players, profileDB, this);
        playerListDialog->setModal(true);
        playerListDialog->show();
        playerListDialog->exec();
        if (!playerListDialog->isListUpdated())
            return;
        players = playerListDialog->getPlayerList();
        delete playerListDialog;

        // Prepare Progress

        int maximumId = snapmaticWidgets.length();
        int overallId = 0;

        QProgressDialog pbDialog(this);
        pbDialog.setWindowFlags(pbDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
        pbDialog.setWindowTitle(tr("Patch selected..."));
        pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(1), QString::number(maximumId)));
        pbDialog.setRange(1, maximumId);
        pbDialog.setValue(1);
        pbDialog.setModal(true);
        QList<QPushButton*> pbBtn = pbDialog.findChildren<QPushButton*>();
        pbBtn.at(0)->setDisabled(true);
        QList<QProgressBar*> pbBar = pbDialog.findChildren<QProgressBar*>();
        pbBar.at(0)->setTextVisible(false);
        pbDialog.setAutoClose(false);
        pbDialog.show();

        // Begin Progress

        QStringList fails;
        for (SnapmaticWidget *snapmaticWidget : qAsConst(snapmaticWidgets)) {
            // Update Progress
            overallId++;
            pbDialog.setValue(overallId);
            pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));

            SnapmaticPicture *picture = snapmaticWidget->getPicture();

            SnapmaticProperties snapmaticProperties = picture->getSnapmaticProperties();
            snapmaticProperties.playersList = players;

            QString currentFilePath = picture->getPictureFilePath();
            QString originalFilePath = picture->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName)) {
                QFile::copy(currentFilePath, backupFileName);
            }
            SnapmaticProperties fallbackProperties = picture->getSnapmaticProperties();
            picture->setSnapmaticProperties(snapmaticProperties);
            if (!picture->exportPicture(currentFilePath)) {
                picture->setSnapmaticProperties(fallbackProperties);
                fails << QString("%1 [%2]").arg(picture->getPictureTitle(), picture->getPictureString());
            }
            else {
                picture->emitUpdate();
                QApplication::processEvents();
            }
        }
        pbDialog.close();
        if (!fails.isEmpty()) {
            QMessageBox::warning(this, tr("Change Players..."), tr("%1 failed with...\n\n%2", "Action failed with...").arg(tr("Change Players", "%1 failed with..."), fails.join(", ")));
        }
    }
        break;
    case MassTool::Crew: {
        QList<SnapmaticWidget*> snapmaticWidgets;
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr) {
                if (widget->isSelected()) {
                    if (widget->getWidgetType() == "SnapmaticWidget") {
                        SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
                        snapmaticWidgets += snapmaticWidget;
                    }
                }
            }
        }

        if (snapmaticWidgets.isEmpty()) {
            QMessageBox::information(this, tr("Change Crew..."), tr("No Snapmatic pictures are selected"));
            return;
        }

        int crewID = 0;
        if (snapmaticWidgets.length() == 1) {
            crewID = snapmaticWidgets.at(0)->getPicture()->getSnapmaticProperties().crewID;
        }
        {
preSelectionCrewID:
            bool ok;
            int indexNum = 0;
            QStringList itemList;
            QStringList crewList = crewDB->getCrews();
            if (!crewList.contains(QLatin1String("0"))) {
                crewList += QLatin1String("0");
            }
            crewList.sort();
            for (QString crew : qAsConst(crewList)) {
                itemList += QString("%1 (%2)").arg(crew, crewDB->getCrewName(crew.toInt()));
            }
            if (crewList.contains(QString::number(crewID))) {
                indexNum = crewList.indexOf(QString::number(crewID));
            }
            QString newCrew = QInputDialog::getItem(this, QApplication::translate("SnapmaticEditor", "Snapmatic Crew"), QApplication::translate("SnapmaticEditor", "New Snapmatic crew:"), itemList, indexNum, true, &ok, windowFlags()^Qt::Dialog^Qt::WindowMinMaxButtonsHint);
            if (ok && !newCrew.isEmpty()) {
                if (newCrew.contains(" ")) newCrew = newCrew.split(" ").at(0);
                if (newCrew.length() > 10) return;
                for (const QChar &crewChar : qAsConst(newCrew)) {
                    if (!crewChar.isNumber()) {
                        QMessageBox::warning(this, tr("Change Crew..."), tr("Failed to enter a valid Snapmatic Crew ID"));
                        goto preSelectionCrewID;
                    }
                }
                if (!crewList.contains(newCrew)) {
                    crewDB->addCrew(crewID);
                }
                crewID = newCrew.toInt();
            }
            else {
                return;
            }
        }

        // Prepare Progress

        int maximumId = snapmaticWidgets.length();
        int overallId = 0;

        QProgressDialog pbDialog(this);
        pbDialog.setWindowFlags(pbDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
        pbDialog.setWindowTitle(tr("Patch selected..."));
        pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(1), QString::number(maximumId)));
        pbDialog.setRange(1, maximumId);
        pbDialog.setValue(1);
        pbDialog.setModal(true);
        QList<QPushButton*> pbBtn = pbDialog.findChildren<QPushButton*>();
        pbBtn.at(0)->setDisabled(true);
        QList<QProgressBar*> pbBar = pbDialog.findChildren<QProgressBar*>();
        pbBar.at(0)->setTextVisible(false);
        pbDialog.setAutoClose(false);
        pbDialog.show();

        // Begin Progress

        QStringList fails;
        for (SnapmaticWidget *snapmaticWidget : qAsConst(snapmaticWidgets)) {
            // Update Progress
            overallId++;
            pbDialog.setValue(overallId);
            pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));

            SnapmaticPicture *picture = snapmaticWidget->getPicture();

            SnapmaticProperties snapmaticProperties = picture->getSnapmaticProperties();
            snapmaticProperties.crewID = crewID;

            QString currentFilePath = picture->getPictureFilePath();
            QString originalFilePath = picture->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName)) {
                QFile::copy(currentFilePath, backupFileName);
            }
            SnapmaticProperties fallbackProperties = picture->getSnapmaticProperties();
            picture->setSnapmaticProperties(snapmaticProperties);
            if (!picture->exportPicture(currentFilePath)) {
                picture->setSnapmaticProperties(fallbackProperties);
                fails << QString("%1 [%2]").arg(picture->getPictureTitle(), picture->getPictureString());
            }
            else {
                picture->emitUpdate();
                QApplication::processEvents();
            }
        }
        pbDialog.close();
        if (!fails.isEmpty()) {
            QMessageBox::warning(this, tr("Change Crew..."), tr("%1 failed with...\n\n%2", "Action failed with...").arg(tr("Change Crew", "%1 failed with..."), fails.join(", ")));
        }
    }
        break;
    case MassTool::Title: {
        QList<SnapmaticWidget*> snapmaticWidgets;
        for (const QString &widgetStr : qAsConst(widgets)) {
            ProfileWidget *widget = widgets.key(widgetStr, nullptr);
            if (widget != nullptr) {
                if (widget->isSelected()) {
                    if (widget->getWidgetType() == "SnapmaticWidget") {
                        SnapmaticWidget *snapmaticWidget = qobject_cast<SnapmaticWidget*>(widget);
                        snapmaticWidgets += snapmaticWidget;
                    }
                }
            }
        }

        if (snapmaticWidgets.isEmpty()) {
            QMessageBox::information(this, tr("Change Title..."), tr("No Snapmatic pictures are selected"));
            return;
        }

        QString snapmaticTitle;
        if (snapmaticWidgets.length() == 1) {
            snapmaticTitle = snapmaticWidgets.at(0)->getPicture()->getPictureTitle();
        }
        {
preSelectionTitle:
            bool ok;
            QString newTitle = QInputDialog::getText(this, QApplication::translate("SnapmaticEditor", "Snapmatic Title"), QApplication::translate("SnapmaticEditor", "New Snapmatic title:"), QLineEdit::Normal, snapmaticTitle, &ok, windowFlags()^Qt::Dialog^Qt::WindowMinMaxButtonsHint);
            if (ok && !newTitle.isEmpty()) {
                if (!SnapmaticPicture::verifyTitle(newTitle)) {
                    QMessageBox::warning(this, tr("Change Title..."), tr("Failed to enter a valid Snapmatic title"));
                    goto preSelectionTitle;
                }
                snapmaticTitle = newTitle;
            }
            else {
                return;
            }
        }

        // Prepare Progress

        int maximumId = snapmaticWidgets.length();
        int overallId = 0;

        QProgressDialog pbDialog(this);
        pbDialog.setWindowFlags(pbDialog.windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowCloseButtonHint);
        pbDialog.setWindowTitle(tr("Patch selected..."));
        pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));
        pbDialog.setRange(1, maximumId);
        pbDialog.setValue(1);
        pbDialog.setModal(true);
        QList<QPushButton*> pbBtn = pbDialog.findChildren<QPushButton*>();
        pbBtn.at(0)->setDisabled(true);
        QList<QProgressBar*> pbBar = pbDialog.findChildren<QProgressBar*>();
        pbBar.at(0)->setTextVisible(false);
        pbDialog.setAutoClose(false);
        pbDialog.show();

        // Begin Progress

        QStringList fails;
        for (SnapmaticWidget *snapmaticWidget : qAsConst(snapmaticWidgets)) {
            // Update Progress
            overallId++;
            pbDialog.setValue(overallId);
            pbDialog.setLabelText(tr("Patch file %1 of %2 files").arg(QString::number(overallId), QString::number(maximumId)));

            SnapmaticPicture *picture = snapmaticWidget->getPicture();

            QString currentFilePath = picture->getPictureFilePath();
            QString originalFilePath = picture->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName)) {
                QFile::copy(currentFilePath, backupFileName);
            }
            QString fallbackTitle = picture->getPictureTitle();
            picture->setPictureTitle(snapmaticTitle);
            if (!picture->exportPicture(currentFilePath)) {
                picture->setPictureTitle(fallbackTitle);
                fails << QString("%1 [%2]").arg(picture->getPictureTitle(), picture->getPictureString());
            }
            else {
                picture->emitUpdate();
                QApplication::processEvents();
            }
        }
        pbDialog.close();
        if (!fails.isEmpty()) {
            QMessageBox::warning(this, tr("Change Title..."), tr("%1 failed with...\n\n%2", "Action failed with...").arg(tr("Change Title", "%1 failed with..."), fails.join(", ")));
        }
    }
        break;
    }
}

int ProfileInterface::getRandomUid()
{
    int random_int = pcg32_boundedrand_r(&rng, 2147483647);
    return random_int;
}
