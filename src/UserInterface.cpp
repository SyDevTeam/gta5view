/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2023 Syping
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
#include <QToolButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QLineEdit>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMap>

#ifdef GTA5SYNC_DONATE
#ifdef GTA5SYNC_DONATE_ADDRESSES
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
#ifdef Q_OS_MAC
    const char* macOS_aboutString = "About %1";
    const char* macOS_preferencesString = "Preferences...";
    ui->actionAbout_gta5sync->setText(QApplication::translate("MAC_APPLICATION_MENU", macOS_aboutString).arg(GTA5SYNC_APPSTR));
    ui->actionOptions->setText(QApplication::translate("MAC_APPLICATION_MENU", macOS_preferencesString));
#else
    ui->actionAbout_gta5sync->setText(tr("&About %1").arg(GTA5SYNC_APPSTR));
#endif
    ui->cmdClose->setToolTip(ui->cmdClose->toolTip().arg(GTA5SYNC_APPSTR));
    defaultWindowTitle = tr("%2 - %1").arg("%1", GTA5SYNC_APPSTR);

    setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
    QString appVersion = QApplication::applicationVersion();
#ifdef GTA5SYNC_COMMIT
    const char* literalBuildType = GTA5SYNC_BUILDTYPE;
    if ((strcmp(literalBuildType, REL_BUILDTYPE) != 0) && !appVersion.contains("-"))
        appVersion = appVersion % "-" % GTA5SYNC_COMMIT;
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
        ui->actionSelect_Game_Folder->setIcon(QIcon::fromTheme("document-open-folder"));
    }
    else if (QIcon::hasThemeIcon("gtk-directory")) {
        ui->actionSelect_Game_Folder->setIcon(QIcon::fromTheme("gtk-directory"));
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
#ifndef Q_OS_MACOS // Setting icon for exit/quit lead to a crash in macOS
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
        donateDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        QVBoxLayout *donateLayout = new QVBoxLayout;
        donateDialog->setLayout(donateLayout);
        QLabel *methodsLabel = new QLabel(QString("<b>%1</b>").arg(tr("Donation methods").toHtmlEscaped()), donateDialog);
        methodsLabel->setWordWrap(true);
        donateLayout->addWidget(methodsLabel);
        QHBoxLayout *currencyLayout = new QHBoxLayout;
        donateLayout->addLayout(currencyLayout);
        const QStringList addressList = QString::fromUtf8(GTA5SYNC_DONATE_ADDRESSES).split(',');
        for (const QString &address : addressList) {
            const QStringList addressList = address.split(':');
            if (addressList.length() == 2) {
                const QString currency = addressList.at(0);
                const QString address = addressList.at(1);
                QString currencyStr = currency;
                const QString strPath = QString(":/donate/%1.str").arg(currency);
                if (QFile::exists(strPath)) {
                    QFile strFile(strPath);
                    if (strFile.open(QIODevice::ReadOnly)) {
                        currencyStr = QString::fromUtf8(strFile.readAll());
                        strFile.close();
                    }
                }
                const QString iconPath = QString(":/donate/%1.svgz").arg(currency);
                QPushButton *currencyButton = new QPushButton(currencyStr, donateDialog);
                currencyButton->setToolTip(currencyStr);
                if (QFile::exists(iconPath)) {
                    currencyButton->setIconSize(QSize(32, 32));
                    currencyButton->setIcon(QIcon(iconPath));
                }
                currencyLayout->addWidget(currencyButton);
                QObject::connect(currencyButton, &QPushButton::pressed, donateDialog, [=](){
                    QDialog *addressDialog = new QDialog(donateDialog);
                    addressDialog->setWindowTitle(currencyStr);
                    addressDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
                    QVBoxLayout *addressLayout = new QVBoxLayout;
                    addressDialog->setLayout(addressLayout);
                    QLabel *addressLabel = new QLabel(address, addressDialog);
                    addressLabel->setAlignment(Qt::AlignCenter);
                    addressLabel->setTextFormat(Qt::PlainText);
                    addressLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    addressLayout->addWidget(addressLabel);
                    QHBoxLayout *qrLayout = new QHBoxLayout;
                    qrLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
                    QrCode qr = QrCode::encodeText(address.toUtf8().constData(), QrCode::Ecc::MEDIUM);
                    const std::string svgString = qr.toSvgString(0);
                    QSvgRenderer svgRenderer(QByteArray::fromRawData(svgString.c_str(), svgString.size()));
                    qreal screenRatioPR = AppEnv::screenRatioPR();
                    const QSize widgetSize = QSize(200, 200);
                    const QSize pixmapSize = widgetSize * screenRatioPR;
                    QPixmap qrPixmap(pixmapSize);
                    qrPixmap.fill(Qt::white);
                    QPainter qrPainter(&qrPixmap);
                    svgRenderer.render(&qrPainter, QRectF(QPointF(0, 0), pixmapSize));
                    qrPainter.end();
#if QT_VERSION >= 0x050600
                    qrPixmap.setDevicePixelRatio(screenRatioPR);
#endif
                    QLabel *qrLabel = new QLabel(addressDialog);
                    qrLabel->setFixedSize(widgetSize);
                    qrLabel->setPixmap(qrPixmap);
                    qrLayout->addWidget(qrLabel);
                    qrLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
                    addressLayout->addLayout(qrLayout);
                    QHBoxLayout *buttonLayout = new QHBoxLayout;
                    buttonLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
                    QPushButton *copyAddressButton = new QPushButton(tr("&Copy"), addressDialog);
                    if (QIcon::hasThemeIcon("edit-copy")) {
                        copyAddressButton->setIcon(QIcon::fromTheme("edit-copy"));
                    }
                    QObject::connect(copyAddressButton, &QPushButton::pressed, addressDialog, [=](){
                        QApplication::clipboard()->setText(address);
                    });
                    buttonLayout->addWidget(copyAddressButton);
                    QPushButton *closeButton = new QPushButton(tr("&Close"), addressDialog);
                    if (QIcon::hasThemeIcon("dialog-close")) {
                        closeButton->setIcon(QIcon::fromTheme("dialog-close"));
                    }
                    else if (QIcon::hasThemeIcon("gtk-close")) {
                        closeButton->setIcon(QIcon::fromTheme("gtk-close"));
                    }
                    closeButton->setDefault(true);
                    buttonLayout->addWidget(closeButton);
                    buttonLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
                    addressLayout->addLayout(buttonLayout);
                    QObject::connect(closeButton, &QPushButton::clicked, addressDialog, &QDialog::accept);
                    QObject::connect(addressDialog, &QDialog::finished, addressDialog, &QDialog::deleteLater);
                    QTimer::singleShot(0, addressDialog, [=](){
                        addressDialog->setFocus();
                    });
                    addressDialog->open();
                    addressDialog->setFixedSize(addressDialog->size());
                });
            }
        }
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
        QTimer::singleShot(0, donateDialog, [=](){
            donateDialog->setFocus();
        });
        donateDialog->open();
        donateDialog->setFixedSize(donateDialog->size());
    });
#endif
#endif

    // Profile UI defaults
    ui->labGTAV->setVisible(false);
    ui->labRDR2->setVisible(false);

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    resize(625 * screenRatio, 500 * screenRatio);
    ui->vlUserInterface->setSpacing(6 * screenRatio);
    ui->vlUserInterface->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
}

void UserInterface::setupDirEnv()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);

    bool folderExists_GTAV, folderExists_RDR2;
    if (GTAV_Folder.isEmpty())
        GTAV_Folder = AppEnv::getGTAVFolder(&folderExists_GTAV);
    else
        folderExists_GTAV = QDir(GTAV_Folder).exists();
    if (RDR2_Folder.isEmpty())
        RDR2_Folder = AppEnv::getRDR2Folder(&folderExists_RDR2);
    else
        folderExists_RDR2 = QDir(RDR2_Folder).exists();

    settings.beginGroup("Profile");
    const QString defaultProfile = settings.value("Default", QString()).toString();
    const QString defaultGame = settings.value("DefaultGame", QStringLiteral("GTA V")).toString();
    settings.endGroup();

    contentMode = settings.value("ContentMode", 0).toInt();
    if (contentMode == 1) {
        contentMode = 21;
    }
    else if (contentMode != 10 && contentMode != 11 && contentMode != 20 && contentMode != 21) {
        contentMode = 20;
    }

    if (folderExists_GTAV) {
        QDir GTAV_ProfilesDir;
        GTAV_ProfilesDir.setPath(GTAV_Folder % "/Profiles");
        GTAV_Profiles = GTAV_ProfilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    }
    else {
        GTAV_Profiles = QStringList();
    }

    if (folderExists_RDR2) {
        QDir RDR2_ProfilesDir;
        RDR2_ProfilesDir.setPath(RDR2_Folder % "/Profiles");
        RDR2_Profiles = RDR2_ProfilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    }
    else {
        RDR2_Profiles = QStringList();
    }

    setupProfileUi();

    if (GTAV_Profiles.length() == 1 && RDR2_Profiles.length() == 0) {
        openProfile(GTAV_Profiles.at(0), RagePhoto::PhotoFormat::GTA5);
    }
    else if (GTAV_Profiles.length() == 0 && RDR2_Profiles.length() == 1) {
        openProfile(RDR2_Profiles.at(0), RagePhoto::PhotoFormat::RDR2);
    }
    else if (defaultGame == QStringLiteral("GTA V") && GTAV_Profiles.contains(defaultProfile)) {
        openProfile(defaultProfile, RagePhoto::PhotoFormat::GTA5);
    }
    else if (defaultGame == QStringLiteral("RDR 2") && RDR2_Profiles.contains(defaultProfile)) {
        openProfile(defaultProfile, RagePhoto::PhotoFormat::RDR2);
    }
}

void UserInterface::setupProfileUi()
{
    bool profileFound = false;
    qreal screenRatio = AppEnv::screenRatio();
    if (!GTAV_Profiles.isEmpty()) {
        int row = 1;
        for (const QString &GTAV_Profile : GTAV_Profiles) {
            QPushButton *profileBtn = new QPushButton(tr("Profile: %1").arg(GTAV_Profile), ui->swSelection);
            profileBtn->setObjectName(GTAV_Profile);
            profileBtn->setMinimumSize(0, 40 * screenRatio);
            profileBtn->setAutoDefault(true);
            ui->glProfiles->addWidget(profileBtn, row++, 0);
            profileBtns += profileBtn;

            QObject::connect(profileBtn, &QPushButton::clicked, this, [=](){
                openProfile(profileBtn->objectName(), RagePhoto::PhotoFormat::GTA5);
            });
        }
        ui->labGTAV->setVisible(true);
        profileFound = true;
    }
    else {
        ui->labGTAV->setVisible(false);
    }
    if (!RDR2_Profiles.isEmpty()) {
        int row = 1;
        for (const QString &RDR2_Profile : RDR2_Profiles) {
            QPushButton *profileBtn = new QPushButton(tr("Profile: %1").arg(RDR2_Profile), ui->swSelection);
            profileBtn->setObjectName(RDR2_Profile);
            profileBtn->setMinimumSize(0, 40 * screenRatio);
            profileBtn->setAutoDefault(true);
            ui->glProfiles->addWidget(profileBtn, row++, 1);
            profileBtns += profileBtn;

            QObject::connect(profileBtn, &QPushButton::clicked, this, [=](){
                openProfile(profileBtn->objectName(), RagePhoto::PhotoFormat::RDR2);
            });
        }
        ui->labRDR2->setVisible(true);
        profileFound = true;
    }
    else {
        ui->labRDR2->setVisible(false);
    }
    if (profileFound) {
        ui->cmdSelectGameFolder->setVisible(false);
    }
    else {
        ui->cmdSelectGameFolder->setVisible(true);
        ui->cmdSelectGameFolder->setFocus();
    }
}

void UserInterface::changeFolder_clicked()
{
    on_actionSelect_Game_Folder_triggered();
}

void UserInterface::on_cmdReload_clicked()
{
    for (QPushButton *profileBtn : profileBtns) {
        ui->glProfiles->removeWidget(profileBtn);
        delete profileBtn;
    }
    profileBtns.clear();
    setupDirEnv();
}

void UserInterface::openProfile(const QString &profileName_, quint32 gameFormat)
{
    profileOpen = true;
    profileName = profileName_;
    profileUI = new ProfileInterface(profileDB, crewDB, threadDB);
    ui->swProfile->addWidget(profileUI);
    ui->swProfile->setCurrentWidget(profileUI);
    if (gameFormat == RagePhoto::PhotoFormat::GTA5)
        profileUI->setProfileFolder(GTAV_Folder % "/Profiles/" % profileName, profileName, gameFormat);
    else if (gameFormat == RagePhoto::PhotoFormat::RDR2)
        profileUI->setProfileFolder(RDR2_Folder % "/Profiles/" % profileName, profileName, gameFormat);
    profileUI->settingsApplied(contentMode, false);
    profileUI->setupProfileInterface();
    QObject::connect(profileUI, &ProfileInterface::profileClosed, this, &UserInterface::closeProfile);
    QObject::connect(profileUI, &ProfileInterface::profileLoaded, this, &UserInterface::profileLoaded);
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
    AboutDialog aboutDialog(this);
    aboutDialog.setWindowIcon(windowIcon());
    aboutDialog.setModal(true);
    aboutDialog.show();
    aboutDialog.exec();
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
    OptionsDialog optionsDialog(profileDB, this);
    optionsDialog.setWindowIcon(windowIcon());
    optionsDialog.commitProfiles(GTAV_Profiles, QStringLiteral("GTA V"));
    optionsDialog.commitProfiles(RDR2_Profiles, QStringLiteral("RDR 2"));
    QObject::connect(&optionsDialog, &OptionsDialog::settingsApplied, this, &UserInterface::settingsApplied);
    optionsDialog.setModal(true);
    optionsDialog.show();
    optionsDialog.exec();
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
    fileDialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    fileDialog.setWindowTitle(tr("Open File..."));

    QStringList filters;
    filters << ProfileInterface::tr("All profile files (%1)").arg("*.g5e SGTA5* PGTA5* SRDR3* PRDR3*");
    filters << ProfileInterface::tr("GTA V Export (%1)").arg("*.g5e");
    filters << ProfileInterface::tr("GTA V Savegames files (%1)").arg("SGTA5*");
    filters << ProfileInterface::tr("GTA V Snapmatic files (%1)").arg("PGTA5*");
    filters << ProfileInterface::tr("RDR 2 Savegames files (%1)").arg("SRDR3*");
    filters << ProfileInterface::tr("RDR 2 Photo files (%1)").arg("PRDR3*");
    filters << ProfileInterface::tr("All files (%1)").arg("**");
    fileDialog.setNameFilters(filters);
    fileDialog.setDirectory(settings.value("OpenDialogDirectory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value("OpenDialogGeometry","").toByteArray());

    if (fileDialog.exec()) {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1) {
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
        if (selectedFileName.startsWith("PGTA5") || selectedFileName.startsWith("PRDR3") || selectedFileName.endsWith(".g5e")) {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            if (picture->readingPicture()) {
                openSnapmaticFile(picture);
                delete picture;
                return true;
            }
            else {
                if (warn)
                    QMessageBox::warning(this, tr("Open File"), ProfileInterface::tr("Failed to read Photo file"));
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
                    QMessageBox::warning(this, tr("Open File"), tr("Can not open %1 because file format is not valid").arg("\""+selectedFileName+"\""));
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

    picDialog.show();
    picDialog.setMinimumSize(picDialog.size());
    picDialog.setMaximumSize(picDialog.size());

    picDialog.exec();
}

void UserInterface::openSavegameFile(SavegameData *savegame)
{
    SavegameDialog sgdDialog(this);
    sgdDialog.setSavegameData(savegame, savegame->getSavegameFileName(), true);
    sgdDialog.setModal(true);
    sgdDialog.show();
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
    messageDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
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

void UserInterface::on_actionSelect_Game_Folder_triggered()
{
    QDialog gameFolderDialog;
    gameFolderDialog.setWindowTitle(tr("Select Game Folder..."));
    gameFolderDialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    QVBoxLayout *gameFolderLayout = new QVBoxLayout(&gameFolderDialog);

    QHBoxLayout gtaFolderLayout;
    gameFolderLayout->addLayout(&gtaFolderLayout);
    QLabel *gtaLabel = new QLabel(tr("GTA V:"), &gameFolderDialog);
    gtaFolderLayout.addWidget(gtaLabel);
    QLineEdit *gtaLocation = new QLineEdit(AppEnv::getGTAVFolder(), &gameFolderDialog);
    gtaLocation->setMinimumWidth(400);
    gtaLocation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    gtaFolderLayout.addWidget(gtaLocation);
    QToolButton *gtaSelectButton = new QToolButton(&gameFolderDialog);
    gtaSelectButton->setText(QStringLiteral("..."));
    QObject::connect(gtaSelectButton, &QPushButton::clicked, &gameFolderDialog, [&,gtaLocation](){
        const QString GTAV_Folder_Temp = QFileDialog::getExistingDirectory(&gameFolderDialog, tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
        if (!GTAV_Folder_Temp.isEmpty() && QDir(GTAV_Folder_Temp).exists())
            gtaLocation->setText(GTAV_Folder_Temp);
    });
    gtaFolderLayout.addWidget(gtaSelectButton);

    QHBoxLayout rdrFolderLayout;
    gameFolderLayout->addLayout(&rdrFolderLayout);
    QLabel *rdrLabel = new QLabel(tr("RDR 2:"), &gameFolderDialog);
    rdrFolderLayout.addWidget(rdrLabel);
    QLineEdit *rdrLocation = new QLineEdit(AppEnv::getRDR2Folder(), &gameFolderDialog);
    rdrLocation->setMinimumWidth(400);
    rdrLocation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    rdrFolderLayout.addWidget(rdrLocation);
    QToolButton *rdrSelectButton = new QToolButton(&gameFolderDialog);
    rdrSelectButton->setText(QStringLiteral("..."));
    QObject::connect(rdrSelectButton, &QPushButton::clicked, &gameFolderDialog, [&,rdrLocation](){
        const QString RDR2_Folder_Temp = QFileDialog::getExistingDirectory(&gameFolderDialog, tr("Select RDR 2 Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
        if (!RDR2_Folder_Temp.isEmpty() && QDir(RDR2_Folder_Temp).exists())
            rdrLocation->setText(RDR2_Folder_Temp);
    });
    rdrFolderLayout.addWidget(rdrSelectButton);

    QSpacerItem *gameFolderSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gameFolderLayout->addSpacerItem(gameFolderSpacer);

    QHBoxLayout buttonLayout;
    gameFolderLayout->addLayout(&buttonLayout);
    QSpacerItem *buttonSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout.addSpacerItem(buttonSpacer);
    QPushButton *selectButton = new QPushButton(tr("&Select"), &gameFolderDialog);
    QObject::connect(selectButton, &QPushButton::clicked, &gameFolderDialog, &QDialog::accept);
    selectButton->setFocus();
    buttonLayout.addWidget(selectButton);
    QPushButton *closeButton = new QPushButton(tr("&Close"), &gameFolderDialog);
    QObject::connect(closeButton, &QPushButton::clicked, &gameFolderDialog, &QDialog::reject);
    buttonLayout.addWidget(closeButton);

    gameFolderDialog.setMinimumSize(gameFolderDialog.sizeHint());
    gameFolderDialog.setMaximumSize(gameFolderDialog.sizeHint());

    if (gameFolderDialog.exec() == QDialog::Accepted) {
        const QString GTAV_Folder_Temp = gtaLocation->text();
        const QString RDR2_Folder_Temp = rdrLocation->text();
        const bool folderExists_GTAV = (!GTAV_Folder_Temp.isEmpty() && QDir(GTAV_Folder_Temp).exists());
        const bool folderExists_RDR2 = (!RDR2_Folder_Temp.isEmpty() && QDir(RDR2_Folder_Temp).exists());
        if (folderExists_GTAV && folderExists_RDR2) {
            GTAV_Folder = GTAV_Folder_Temp;
            RDR2_Folder = RDR2_Folder_Temp;
            if (profileOpen)
                closeProfile_p();
            on_cmdReload_clicked();
        }
        else if (folderExists_GTAV && !folderExists_RDR2) {
            GTAV_Folder = GTAV_Folder_Temp;
            RDR2_Folder = QString();
            if (profileOpen)
                closeProfile_p();
            on_cmdReload_clicked();
        }
        else if (folderExists_RDR2 && !folderExists_GTAV) {
            GTAV_Folder = QString();
            RDR2_Folder = RDR2_Folder_Temp;
            if (profileOpen)
                closeProfile_p();
            on_cmdReload_clicked();
        }
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
#ifdef Q_OS_MAC
    const char* macOS_aboutString = "About %1";
    const char* macOS_preferencesString = "Preferences...";
    ui->actionAbout_gta5sync->setText(QApplication::translate("MAC_APPLICATION_MENU", macOS_aboutString).arg(GTA5SYNC_APPSTR));
    ui->actionOptions->setText(QApplication::translate("MAC_APPLICATION_MENU", macOS_preferencesString));
#else
    ui->actionAbout_gta5sync->setText(tr("&About %1").arg(GTA5SYNC_APPSTR));
#endif
    QString appVersion = QApplication::applicationVersion();
#ifdef GTA5SYNC_COMMIT
    const char* literalBuildType = GTA5SYNC_BUILDTYPE;
    if ((strcmp(literalBuildType, REL_BUILDTYPE) != 0) && !appVersion.contains("-"))
        appVersion = appVersion % "-" % GTA5SYNC_COMMIT;
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
