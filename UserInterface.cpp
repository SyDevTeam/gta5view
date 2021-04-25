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

#include "UserInterface.h"
#include "ui_UserInterface.h"
#include "ProfileInterface.h"
#include "SnapmaticPicture.h"
#include "SidebarGenerator.h"
#include "SavegameDialog.h"
#include "StandardPaths.h"
#include "OptionsDialog.h"
#include "PictureDialog.h"
#include "SavegameData.h"
#include "AboutDialog.h"
#include "IconLoader.h"
#include "AppEnv.h"
#include "config.h"
#include <QtGlobal>
#include <QStringBuilder>
#include <QStyleFactory>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMap>

#ifdef GTA5SYNC_DONATE
#ifdef GTA5SYNC_DONATE_ADDRESSES
#include <QFontDatabase>
#include <QSvgRenderer>
#include <QClipboard>
#include <QPainter>
#include "QrCode.h"
using namespace qrcodegen;
#endif
#endif

#ifdef GTA5SYNC_MOTD
UserInterface::UserInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, MessageThread *threadMessage, QWidget *parent) :
    QMainWindow(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB), threadMessage(threadMessage),
    ui(new Ui::UserInterface)
  #else
UserInterface::UserInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent) :
    QMainWindow(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB),
    ui(new Ui::UserInterface)
  #endif
{
    ui->setupUi(this);
    contentMode = 0;
    profileOpen = 0;
    profileUI = 0;
    ui->menuProfile->setEnabled(false);
    ui->actionSelect_profile->setEnabled(false);
    ui->actionAbout_gta5sync->setIcon(IconLoader::loadingAppIcon());
    ui->actionAbout_gta5sync->setText(tr("&About %1").arg(GTA5SYNC_APPSTR));
    ui->cmdClose->setToolTip(ui->cmdClose->toolTip().arg(GTA5SYNC_APPSTR));
    defaultWindowTitle = tr("%2 - %1").arg("%1", GTA5SYNC_APPSTR);

    setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
    QString appVersion = GTA5SYNC_APPVER;
#ifndef GTA5SYNC_BUILDTYPE_REL
#ifdef GTA5SYNC_COMMIT
    if (!appVersion.contains("-")) { appVersion = appVersion % "-" % GTA5SYNC_COMMIT; }
#endif
#endif
    ui->labVersion->setText(QString("%1 %2").arg(GTA5SYNC_APPSTR, appVersion));

    // Set Icon for Close Button
    if (QIcon::hasThemeIcon("dialog-close")) {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close")) {
        ui->cmdClose->setIcon(QIcon::fromTheme("gtk-close"));
    }

    // Set Icon for Reload Button
    if (QIcon::hasThemeIcon("view-refresh")) {
        ui->cmdReload->setIcon(QIcon::fromTheme("view-refresh"));
    }
    else if (QIcon::hasThemeIcon("reload")) {
        ui->cmdReload->setIcon(QIcon::fromTheme("reload"));
    }

    // Set Icon for Choose GTA V Folder Menu Item
    if (QIcon::hasThemeIcon("document-open-folder")) {
        ui->actionSelect_GTA_Folder->setIcon(QIcon::fromTheme("document-open-folder"));
    }
    else if (QIcon::hasThemeIcon("gtk-directory")) {
        ui->actionSelect_GTA_Folder->setIcon(QIcon::fromTheme("gtk-directory"));
    }

    // Set Icon for Open File Menu Item
    if (QIcon::hasThemeIcon("document-open")) {
        ui->actionOpen_File->setIcon(QIcon::fromTheme("document-open"));
    }

    // Set Icon for Close Profile Menu Item
    if (QIcon::hasThemeIcon("dialog-close")) {
        ui->actionSelect_profile->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close")) {
        ui->actionSelect_profile->setIcon(QIcon::fromTheme("gtk-close"));
    }

    // Set Icon for Exit Menu Item
    if (QIcon::hasThemeIcon("application-exit")) {
#ifndef Q_OS_MACOS // Setting icon for exit/quit lead to a crash in Mac OS X
        ui->actionExit->setIcon(QIcon::fromTheme("application-exit"));
#endif
    }

    // Set Icon for Preferences Menu Item
    if (QIcon::hasThemeIcon("preferences-system")) {
#ifndef Q_OS_MACOS // Setting icon for preferences/settings/options lead to a crash in Mac OS X
        ui->actionOptions->setIcon(QIcon::fromTheme("preferences-system"));
#endif
    }
    else if (QIcon::hasThemeIcon("configure")) {
#ifndef Q_OS_MACOS // Setting icon for preferences/settings/options lead to a crash in Mac OS X
        ui->actionOptions->setIcon(QIcon::fromTheme("configure"));
#endif
    }

    // Set Icon for Profile Import Menu Item
    if (QIcon::hasThemeIcon("document-import")) {
        ui->action_Import->setIcon(QIcon::fromTheme("document-import"));
    }
    else if (QIcon::hasThemeIcon("document-open")) {
        ui->action_Import->setIcon(QIcon::fromTheme("document-open"));
    }

    // Set Icon for Profile Export Menu Item
    if (QIcon::hasThemeIcon("document-export")) {
        ui->actionExport_selected->setIcon(QIcon::fromTheme("document-export"));
    }
    else if (QIcon::hasThemeIcon("document-save")) {
        ui->actionExport_selected->setIcon(QIcon::fromTheme("document-save"));
    }

    // Set Icon for Profile Remove Menu Item
    if (QIcon::hasThemeIcon("remove")) {
        ui->actionDelete_selected->setIcon(QIcon::fromTheme("remove"));
    }

#ifdef GTA5SYNC_DONATE
#ifdef GTA5SYNC_DONATE_ADDRESSES
    donateAction = new QAction(tr("&Donate"), this);
    if (QIcon::hasThemeIcon("help-donate")) {
        donateAction->setIcon(QIcon::fromTheme("help-donate"));
    }
    else if (QIcon::hasThemeIcon("taxes-finances")) {
        donateAction->setIcon(QIcon::fromTheme("taxes-finances"));
    }
    else {
        donateAction->setIcon(QIcon(":/img/donate.svgz"));
    }
    ui->menuHelp->insertAction(ui->actionAbout_gta5sync, donateAction);
    QObject::connect(donateAction, &QAction::triggered, this, [=](){
        QDialog *donateDialog = new QDialog(this);
        donateDialog->setWindowTitle(QString("%1 - %2").arg(GTA5SYNC_APPSTR, tr("Donate")));
#if QT_VERSION >= 0x050900
        donateDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#else
        donateDialog->setWindowFlags(donateDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
#endif
        QVBoxLayout *donateLayout = new QVBoxLayout;
        donateDialog->setLayout(donateLayout);
        QLabel *methodsLabel = new QLabel(QString("<b>%1</b>").arg(tr("Donation methods").toHtmlEscaped()), this);
        methodsLabel->setWordWrap(true);
        donateLayout->addWidget(methodsLabel);
        const QStringList addressList = QString::fromUtf8(GTA5SYNC_DONATE_ADDRESSES).split(',');
        for (const QString &address : addressList) {
            const QStringList addressList = address.split(':');
            if (addressList.length() == 2) {
                const QString currency = addressList.at(0);
                const QString address = addressList.at(1);
                QHBoxLayout *addressLayout = new QHBoxLayout;
                const QString iconPath = QString(":/donate/%1.svgz").arg(currency);
                if (QFile::exists(iconPath)) {
                    QLabel *currencyLabel = new QLabel(this);
                    currencyLabel->setFixedSize(32, 32);
                    currencyLabel->setScaledContents(true);
                    currencyLabel->setPixmap(QIcon(iconPath).pixmap(QSize(32, 32)));
                    addressLayout->addWidget(currencyLabel);
                }
                QLabel *currencyLabel = new QLabel(currency, this);
                currencyLabel->setTextFormat(Qt::PlainText);
                QFont currencyFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
                currencyFont.setWeight(QFont::Bold);
                currencyFont.setCapitalization(QFont::AllUppercase);
                currencyLabel->setFont(currencyFont);
                addressLayout->addWidget(currencyLabel);
                QLabel *addressLabel = new QLabel(address, this);
                addressLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                addressLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
                addressLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                addressLabel->setWordWrap(true);
                addressLayout->addWidget(addressLabel);
                QPushButton *viewAddressButton = new QPushButton(tr("View"), this);
                QObject::connect(viewAddressButton, &QPushButton::pressed, this, [=](){
                    QDialog *addressDialog = new QDialog(donateDialog);
                    QVBoxLayout *addressLayout = new QVBoxLayout;
                    addressDialog->setLayout(addressLayout);
                    QrCode qr = QrCode::encodeText(address.toUtf8().constData(), QrCode::Ecc::MEDIUM);
                    const std::string svgString = qr.toSvgString(0);
                    QSvgRenderer svgRenderer(QByteArray::fromRawData(svgString.c_str(), svgString.size()));
                    qreal screenRatio = AppEnv::screenRatio();
                    qreal screenRatioPR = AppEnv::screenRatioPR();
                    const QSize widgetSize = QSize(300, 300) * screenRatio;
                    const QSize pixmapSize = widgetSize * screenRatioPR;
                    QPixmap addressPixmap(pixmapSize);
                    addressPixmap.fill(Qt::white);
                    QPainter addressPainter(&addressPixmap);
                    svgRenderer.render(&addressPainter, QRectF(QPointF(0, 0), pixmapSize));
                    addressPainter.end();
#if QT_VERSION >= 0x050600
                    addressPixmap.setDevicePixelRatio(screenRatioPR);
#endif
                    QLabel *addressLabel = new QLabel(addressDialog);
                    addressLabel->setFixedSize(widgetSize);
                    addressLabel->setPixmap(addressPixmap);
                    addressLayout->addWidget(addressLabel);
                    addressDialog->setFixedSize(addressDialog->sizeHint());
                    QObject::connect(addressDialog, &QDialog::finished, addressDialog, &QDialog::deleteLater);
                    addressDialog->open();
                });
                addressLayout->addWidget(viewAddressButton);
                QPushButton *copyAddressButton = new QPushButton(tr("Copy"), this);
                QObject::connect(copyAddressButton, &QPushButton::pressed, this, [=](){
                    QApplication::clipboard()->setText(address);
                });
                addressLayout->addWidget(copyAddressButton);
                donateLayout->addLayout(addressLayout);
            }
        }
        donateLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
        QPushButton *closeButton = new QPushButton(donateDialog);
        closeButton->setText(tr("&Close"));
        if (QIcon::hasThemeIcon("dialog-close")) {
            closeButton->setIcon(QIcon::fromTheme("dialog-close"));
        }
        else if (QIcon::hasThemeIcon("gtk-close")) {
            closeButton->setIcon(QIcon::fromTheme("gtk-close"));
        }
        closeButton->setDefault(true);
        buttonLayout->addWidget(closeButton);
        donateLayout->addLayout(buttonLayout);
        QObject::connect(closeButton, &QPushButton::clicked, donateDialog, &QDialog::accept);
        QObject::connect(donateDialog, &QDialog::finished, donateDialog, &QDialog::deleteLater);
        QTimer::singleShot(0, closeButton, [=](){
            closeButton->setFocus();
        });
        donateDialog->open();
    });
#endif
#endif

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
#ifndef Q_QS_ANDROID
    resize(625 * screenRatio, 500 * screenRatio);
#endif
    ui->vlUserInterface->setSpacing(6 * screenRatio);
    ui->vlUserInterface->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
}

void UserInterface::setupDirEnv(bool showFolderDialog)
{
    // settings init
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);

    bool folderExists;
    GTAV_Folder = AppEnv::getGameFolder(&folderExists);
    if (folderExists) {
        QDir::setCurrent(GTAV_Folder);
    }
    else if (showFolderDialog) {
        GTAV_Folder = QFileDialog::getExistingDirectory(this, tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
        if (QFileInfo(GTAV_Folder).exists()) {
            folderExists = true;
            QDir::setCurrent(GTAV_Folder);
            AppEnv::setGameFolder(GTAV_Folder);

            // First time folder selection save
            settings.beginGroup("dir");
            if (settings.value("dir", "").toString().isEmpty())
            {
                settings.setValue("dir", GTAV_Folder);
            }
            settings.endGroup();
        }
    }

    // profiles init
    settings.beginGroup("Profile");
    QString defaultProfile = settings.value("Default", "").toString();

    contentMode = settings.value("ContentMode", 0).toInt();
    if (contentMode == 1) {
        contentMode = 21;
    }
    else if (contentMode != 10 && contentMode != 11 && contentMode != 20 && contentMode != 21) {
        contentMode = 20;
    }

    if (folderExists) {
        QDir GTAV_ProfilesDir;
        GTAV_ProfilesFolder = GTAV_Folder % "/Profiles";
        GTAV_ProfilesDir.setPath(GTAV_ProfilesFolder);

        GTAV_Profiles = GTAV_ProfilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
        setupProfileUi();

        if (GTAV_Profiles.length() == 1) {
            openProfile(GTAV_Profiles.at(0));
        }
        else if(GTAV_Profiles.contains(defaultProfile)) {
            openProfile(defaultProfile);
        }
    }
    else {
        GTAV_Profiles = QStringList();
        setupProfileUi();
    }
    settings.endGroup();
}

void UserInterface::setupProfileUi()
{
    qreal screenRatio = AppEnv::screenRatio();
    if (GTAV_Profiles.isEmpty()) {
        QPushButton *changeDirBtn = new QPushButton(tr("Select &GTA V Folder..."), ui->swSelection);
        changeDirBtn->setObjectName("cmdChangeDir");
        changeDirBtn->setMinimumSize(0, 40 * screenRatio);
        changeDirBtn->setAutoDefault(true);
        ui->vlButtons->addWidget(changeDirBtn);
        profileBtns += changeDirBtn;

        QObject::connect(changeDirBtn, SIGNAL(clicked(bool)), this, SLOT(changeFolder_clicked()));
    }
    else for (const QString &GTAV_Profile : GTAV_Profiles) {
        QPushButton *profileBtn = new QPushButton(GTAV_Profile, ui->swSelection);
        profileBtn->setObjectName(GTAV_Profile);
        profileBtn->setMinimumSize(0, 40 * screenRatio);
        profileBtn->setAutoDefault(true);
        ui->vlButtons->addWidget(profileBtn);
        profileBtns += profileBtn;

        QObject::connect(profileBtn, SIGNAL(clicked(bool)), this, SLOT(profileButton_clicked()));
    }
    profileBtns.at(0)->setFocus();
}

void UserInterface::changeFolder_clicked()
{
    on_actionSelect_GTA_Folder_triggered();
}

void UserInterface::on_cmdReload_clicked()
{
    for (QPushButton *profileBtn : profileBtns) {
        ui->vlButtons->removeWidget(profileBtn);
        delete profileBtn;
    }
    profileBtns.clear();
    setupDirEnv();
}

void UserInterface::profileButton_clicked()
{
    QPushButton *profileBtn = (QPushButton*)sender();
    openProfile(profileBtn->objectName());
}

void UserInterface::openProfile(const QString &profileName_)
{
    profileOpen = true;
    profileName = profileName_;
    profileUI = new ProfileInterface(profileDB, crewDB, threadDB);
    ui->swProfile->addWidget(profileUI);
    ui->swProfile->setCurrentWidget(profileUI);
    profileUI->setProfileFolder(GTAV_ProfilesFolder % QDir::separator() % profileName, profileName);
    profileUI->settingsApplied(contentMode, false);
    profileUI->setupProfileInterface();
    QObject::connect(profileUI, SIGNAL(profileClosed()), this, SLOT(closeProfile()));
    QObject::connect(profileUI, SIGNAL(profileLoaded()), this, SLOT(profileLoaded()));
    setWindowTitle(defaultWindowTitle.arg(profileName));
}

void UserInterface::closeProfile()
{
    if (profileOpen) {
        closeProfile_p();
    }
    setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
}

void UserInterface::closeProfile_p()
{
    profileOpen = false;
    profileName.clear();
    profileName.squeeze();
    ui->menuProfile->setEnabled(false);
    ui->actionSelect_profile->setEnabled(false);
    ui->swProfile->removeWidget(profileUI);
    profileUI->disconnect();
    delete profileUI;
}

void UserInterface::closeEvent(QCloseEvent *ev)
{
    Q_UNUSED(ev)
#ifdef GTA5SYNC_MOTD
    threadMessage->terminateThread();
#else
    threadDB->terminateThread();
#endif
}

UserInterface::~UserInterface()
{
    if (profileOpen)
        closeProfile_p();
    for (QPushButton *profileBtn : profileBtns) {
        delete profileBtn;
    }
    profileBtns.clear();
    delete ui;
}

void UserInterface::on_actionExit_triggered()
{
    close();
}

void UserInterface::on_actionSelect_profile_triggered()
{
    closeProfile();
    openSelectProfile();
}

void UserInterface::openSelectProfile()
{
    // not needed right now
}

void UserInterface::on_actionAbout_gta5sync_triggered()
{
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->setWindowIcon(windowIcon());
    aboutDialog->setModal(true);
#ifdef Q_OS_ANDROID
    // Android ...
    aboutDialog->showMaximized();
#else
    aboutDialog->show();
#endif
    aboutDialog->exec();
    delete aboutDialog;
}

void UserInterface::profileLoaded()
{
    ui->menuProfile->setEnabled(true);
    ui->actionSelect_profile->setEnabled(true);
}

void UserInterface::on_actionSelect_all_triggered()
{
    if (profileOpen)
        profileUI->selectAllWidgets();
}

void UserInterface::on_actionDeselect_all_triggered()
{
    if (profileOpen)
        profileUI->deselectAllWidgets();
}

void UserInterface::on_actionExport_selected_triggered()
{
    if (profileOpen)
        profileUI->exportSelected();
}

void UserInterface::on_actionDelete_selected_triggered()
{
    if (profileOpen)
        profileUI->deleteSelected();
}

void UserInterface::on_actionOptions_triggered()
{
    OptionsDialog *optionsDialog = new OptionsDialog(profileDB, this);
    optionsDialog->setWindowIcon(windowIcon());
    optionsDialog->commitProfiles(GTAV_Profiles);
    QObject::connect(optionsDialog, SIGNAL(settingsApplied(int, bool)), this, SLOT(settingsApplied(int, bool)));

    optionsDialog->setModal(true);
#ifdef Q_OS_ANDROID
    // Android ...
    optionsDialog->showMaximized();
#else
    optionsDialog->show();
#endif
    optionsDialog->exec();

    delete optionsDialog;
}

void UserInterface::on_action_Import_triggered()
{
    if (profileOpen)
        profileUI->importFiles();
}

void UserInterface::on_actionOpen_File_triggered()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");

fileDialogPreOpen:
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
#if QT_VERSION >= 0x050900
    fileDialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#else
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
#endif
    fileDialog.setWindowTitle(tr("Open File..."));

    QStringList filters;
    filters << ProfileInterface::tr("All profile files (*.g5e SGTA* PGTA*)");
    filters << ProfileInterface::tr("GTA V Export (*.g5e)");
    filters << ProfileInterface::tr("Savegames files (SGTA*)");
    filters << ProfileInterface::tr("Snapmatic pictures (PGTA*)");
    filters << ProfileInterface::tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value("OpenDialogDirectory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value("OpenDialogGeometry","").toByteArray());

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            if (!openFile(selectedFile, true)) goto fileDialogPreOpen;
        }
    }

    settings.setValue("OpenDialogGeometry", fileDialog.saveGeometry());
    settings.setValue("OpenDialogDirectory", fileDialog.directory().absolutePath());
    settings.endGroup();
}

bool UserInterface::openFile(QString selectedFile, bool warn)
{
    QString selectedFileName = QFileInfo(selectedFile).fileName();
    if (QFile::exists(selectedFile)) {
        if (selectedFileName.left(4) == "PGTA" || selectedFileName.right(4) == ".g5e") {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            if (picture->readingPicture()) {
                openSnapmaticFile(picture);
                delete picture;
                return true;
            }
            else {
                if (warn)
                    QMessageBox::warning(this, tr("Open File"), ProfileInterface::tr("Failed to read Snapmatic picture"));
                delete picture;
                return false;
            }
        }
        else if (selectedFileName.left(4) == "SGTA") {
            SavegameData *savegame = new SavegameData(selectedFile);
            if (savegame->readingSavegame()) {
                openSavegameFile(savegame);
                delete savegame;
                return true;
            }
            else {
                if (warn)
                    QMessageBox::warning(this, tr("Open File"), ProfileInterface::tr("Failed to read Savegame file"));
                delete savegame;
                return false;
            }
        }
        else {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            SavegameData *savegame = new SavegameData(selectedFile);
            if (picture->readingPicture()) {
                delete savegame;
                openSnapmaticFile(picture);
                delete picture;
                return true;
            }
            else if (savegame->readingSavegame()) {
                delete picture;
                openSavegameFile(savegame);
                delete savegame;
                return true;
            }
            else {
                delete savegame;
                delete picture;
                if (warn)
                    QMessageBox::warning(this, tr("Open File"), tr("Can't open %1 because of not valid file format").arg("\""+selectedFileName+"\""));
                return false;
            }
        }
    }
    if (warn)
        QMessageBox::warning(this, tr("Open File"), ProfileInterface::tr("No valid file is selected"));
    return false;
}

void UserInterface::openSnapmaticFile(SnapmaticPicture *picture)
{
    PictureDialog picDialog(profileDB, crewDB, this);
    picDialog.setSnapmaticPicture(picture, true);
    picDialog.setModal(true);

    int crewID = picture->getSnapmaticProperties().crewID;
    if (crewID != 0)
        crewDB->addCrew(crewID);

    QObject::connect(threadDB, SIGNAL(crewNameUpdated()), &picDialog, SLOT(crewNameUpdated()));
    QObject::connect(threadDB, SIGNAL(playerNameUpdated()), &picDialog, SLOT(playerNameUpdated()));

#ifdef Q_OS_ANDROID
    // Android optimization should be put here
    picDialog.showMaximized();
#else
    picDialog.show();
    picDialog.setMinimumSize(picDialog.size());
    picDialog.setMaximumSize(picDialog.size());
#endif

    picDialog.exec();
}

void UserInterface::openSavegameFile(SavegameData *savegame)
{
    SavegameDialog sgdDialog(this);
    sgdDialog.setSavegameData(savegame, savegame->getSavegameFileName(), true);
    sgdDialog.setModal(true);
#ifdef Q_OS_ANDROID
    // Android optimization should be put here
    sgdDialog.showMaximized();
#else
    sgdDialog.show();
#endif
    sgdDialog.exec();
}

void UserInterface::settingsApplied(int _contentMode, bool languageChanged)
{
    if (languageChanged) {
        retranslateUi();
    }
    contentMode = _contentMode;
    if (profileOpen) {
        profileUI->settingsApplied(contentMode, languageChanged);
    }
}

#ifdef GTA5SYNC_MOTD
void UserInterface::messagesArrived(const QJsonObject &object)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Messages");
    QJsonObject::const_iterator it = object.constBegin();
    QJsonObject::const_iterator end = object.constEnd();
    QStringList messages;
    while (it != end) {
        const QString key = it.key();
        const QJsonValue value = it.value();
        bool uintOk;
        uint messageId = key.toUInt(&uintOk);
        if (uintOk && value.isString()) {
            const QString valueStr = value.toString();
            settings.setValue(QString::number(messageId), valueStr);
            messages << valueStr;
        }
        it++;
    }
    settings.endGroup();
    if (!messages.isEmpty())
        showMessages(messages);
}

void UserInterface::showMessages(const QStringList messages)
{
    QDialog *messageDialog = new QDialog(this);
    messageDialog->setWindowTitle(tr("%1 - Messages").arg(GTA5SYNC_APPSTR));
#if QT_VERSION >= 0x050900
    messageDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#else
    messageDialog->setWindowFlags(messageDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
#endif
    QVBoxLayout *messageLayout = new QVBoxLayout;
    messageDialog->setLayout(messageLayout);
    QStackedWidget *stackWidget = new QStackedWidget(messageDialog);
    for (const QString message : messages) {
        QLabel *messageLabel = new QLabel(messageDialog);
        messageLabel->setText(message);
        messageLabel->setWordWrap(true);
        stackWidget->addWidget(messageLabel);
    }
    messageLayout->addWidget(stackWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *backButton = new QPushButton(messageDialog);
    QPushButton *nextButton = new QPushButton(messageDialog);
    if (QIcon::hasThemeIcon("go-previous") && QIcon::hasThemeIcon("go-next") && QIcon::hasThemeIcon("list-add")) {
        backButton->setIcon(QIcon::fromTheme("go-previous"));
        nextButton->setIcon(QIcon::fromTheme("go-next"));
    }
    else {
        backButton->setIcon(QIcon(AppEnv::getImagesFolder() % "/back.svgz"));
        nextButton->setIcon(QIcon(AppEnv::getImagesFolder() % "/next.svgz"));
    }
    backButton->setEnabled(false);
    if (stackWidget->count() <= 1) {
        nextButton->setEnabled(false);
    }
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    QPushButton *closeButton = new QPushButton(messageDialog);
    closeButton->setText(tr("&Close"));
    if (QIcon::hasThemeIcon("dialog-close")) {
        closeButton->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close")) {
        closeButton->setIcon(QIcon::fromTheme("gtk-close"));
    }
    buttonLayout->addWidget(closeButton);
    messageLayout->addLayout(buttonLayout);
    QObject::connect(backButton, &QPushButton::clicked, [stackWidget,backButton,nextButton,closeButton]() {
        int index = stackWidget->currentIndex();
        if (index > 0) {
            index--;
            stackWidget->setCurrentIndex(index);
            nextButton->setEnabled(true);
            if (index > 0) {
                backButton->setEnabled(true);
            }
            else {
                backButton->setEnabled(false);
                closeButton->setFocus();
            }
        }
    });
    QObject::connect(nextButton, &QPushButton::clicked, [stackWidget,backButton,nextButton,closeButton]() {
        int index = stackWidget->currentIndex();
        if (index < stackWidget->count()-1) {
            index++;
            stackWidget->setCurrentIndex(index);
            backButton->setEnabled(true);
            if (index < stackWidget->count()-1) {
                nextButton->setEnabled(true);
            }
            else {
                nextButton->setEnabled(false);
                closeButton->setFocus();
            }
        }
    });
    QObject::connect(closeButton, &QPushButton::clicked, messageDialog, &QDialog::accept);
    QObject::connect(messageDialog, &QDialog::finished, messageDialog, &QDialog::deleteLater);
    QTimer::singleShot(0, closeButton, [=](){
        closeButton->setFocus();
    });
    messageDialog->show();
}

void UserInterface::updateCacheId(uint cacheId)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Messages");
    settings.setValue("CacheId", cacheId);
    settings.endGroup();
}
#endif

void UserInterface::on_actionSelect_GTA_Folder_triggered()
{
    QString GTAV_Folder_Temp = QFileDialog::getExistingDirectory(this, tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
    if (QFileInfo(GTAV_Folder_Temp).exists()) {
        if (profileOpen) {
            closeProfile_p();
        }
        GTAV_Folder = GTAV_Folder_Temp;
        QDir::setCurrent(GTAV_Folder);
        AppEnv::setGameFolder(GTAV_Folder);
        on_cmdReload_clicked();
    }
}

void UserInterface::on_action_Enable_In_game_triggered()
{
    if (profileOpen)
        profileUI->enableSelected();
}

void UserInterface::on_action_Disable_In_game_triggered()
{
    if (profileOpen)
        profileUI->disableSelected();
}

void UserInterface::retranslateUi()
{
    ui->retranslateUi(this);
#ifdef GTA5SYNC_DONATE
#ifdef GTA5SYNC_DONATE_ADDRESSES
    donateAction->setText(tr("&Donate"));
#endif
#endif
    ui->actionAbout_gta5sync->setText(tr("&About %1").arg(GTA5SYNC_APPSTR));
    QString appVersion = GTA5SYNC_APPVER;
#ifndef GTA5SYNC_BUILDTYPE_REL
#ifdef GTA5SYNC_COMMIT
    if (!appVersion.contains("-"))
        appVersion = appVersion % "-" % GTA5SYNC_COMMIT;
#endif
#endif
    ui->labVersion->setText(QString("%1 %2").arg(GTA5SYNC_APPSTR, appVersion));
    if (profileOpen) {
        setWindowTitle(defaultWindowTitle.arg(profileName));
    }
    else {
        setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
    }
}

void UserInterface::on_actionQualify_as_Avatar_triggered()
{
    profileUI->massTool(MassTool::Qualify);
}

void UserInterface::on_actionChange_Players_triggered()
{
    profileUI->massTool(MassTool::Players);
}

void UserInterface::on_actionSet_Crew_triggered()
{
    profileUI->massTool(MassTool::Crew);
}

void UserInterface::on_actionSet_Title_triggered()
{
    profileUI->massTool(MassTool::Title);
}
