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

#include "TranslationClass.h"
#include "SnapmaticPicture.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "SavegameDialog.h"
#include "PictureDialog.h"
#include "UserInterface.h"
#include "CrewDatabase.h"
#include "SavegameData.h"
#include "UiModLabel.h"
#include "IconLoader.h"
#include "config.h"
#include <QStringBuilder>
#include <QSignalMapper>
#include <QStyleFactory>
#include <QApplication>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStringList>
#include <QTranslator>
#include <QResource>
#include <QCheckBox>
#include <QFileInfo>
#include <QSysInfo>
#include <QLayout>
#include <QObject>
#include <QString>
#include <QFont>
#include <QFile>

#if QT_VERSION < 0x060000
#include <QDesktopWidget>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include <iostream>
#endif

#ifdef GTA5SYNC_MOTD
#include "MessageThread.h"
#endif

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION >= 0x050600
#if QT_VERSION < 0x060000
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
#endif
    QApplication a(argc, argv);
    a.setApplicationName(GTA5SYNC_APPSTR);
    a.setApplicationVersion(GTA5SYNC_APPVER);
    a.setQuitOnLastWindowClosed(false);

    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Startup");

#ifdef GTA5SYNC_TELEMETRY
    // Increase Start count at every startup
    uint startCount = settings.value("StartCount", 0).toUInt();
    startCount++;
    settings.setValue("StartCount", startCount);
    settings.sync();
#endif

    bool customStyle = settings.value("CustomStyle", false).toBool();
    if (customStyle) {
        const QString appStyle = settings.value("AppStyle", "Default").toString();
        if (QStyleFactory::keys().contains(appStyle, Qt::CaseInsensitive)) {
            a.setStyle(QStyleFactory::create(appStyle));
        }
    }

#ifdef Q_OS_WIN
#if QT_VERSION >= 0x060000
    a.setFont(QApplication::font("QMenu"));
#elif QT_VERSION >= 0x050400
    if (QSysInfo::windowsVersion() >= 0x0080) {
        a.setFont(QApplication::font("QMenu"));
    }
#endif
#endif

    bool customFont = settings.value("CustomFont", false).toBool();
    if (customFont) {
        const QFont appFont = qvariant_cast<QFont>(settings.value("AppFont", a.font()));
        a.setFont(appFont);
    }

    QStringList applicationArgs = a.arguments();
    QString selectedAction;
    QString arg1;
    applicationArgs.removeAt(0);

    Translator->initUserLanguage();
    Translator->loadTranslation(&a);

#ifdef GTA5SYNC_TELEMETRY
    Telemetry->init();
    Telemetry->work();
#endif

#ifdef GTA5SYNC_TELEMETRY
    bool telemetryWindowLaunched = settings.value("PersonalUsageDataWindowLaunched", false).toBool();
    bool pushUsageData = settings.value("PushUsageData", false).toBool();
    if (!telemetryWindowLaunched && !pushUsageData) {
        QDialog telemetryDialog;
        telemetryDialog.setObjectName(QStringLiteral("TelemetryDialog"));
        telemetryDialog.setWindowTitle(QString("%1 %2").arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER));
        telemetryDialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        telemetryDialog.setWindowFlag(Qt::WindowCloseButtonHint, false);
        telemetryDialog.setWindowIcon(IconLoader::loadingAppIcon());
        QVBoxLayout *telemetryLayout = new QVBoxLayout(&telemetryDialog);
        telemetryLayout->setObjectName(QStringLiteral("TelemetryLayout"));
        telemetryDialog.setLayout(telemetryLayout);
        UiModLabel *telemetryLabel = new UiModLabel(&telemetryDialog);
        telemetryLabel->setObjectName(QStringLiteral("TelemetryLabel"));
        telemetryLabel->setText(QString("<h4>%2</h4>%1").arg(
                                    QApplication::translate("TelemetryDialog", "You want help %1 to improve in the future by including personal usage data in your submission?").arg(GTA5SYNC_APPSTR),
                                    QApplication::translate("TelemetryDialog", "%1 User Statistics").arg(GTA5SYNC_APPSTR)));
        telemetryLayout->addWidget(telemetryLabel);
        QCheckBox *telemetryCheckBox = new QCheckBox(&telemetryDialog);
        telemetryCheckBox->setObjectName(QStringLiteral("TelemetryCheckBox"));
        telemetryCheckBox->setText(QApplication::translate("TelemetryDialog", "Yes, I want include personal usage data."));
        telemetryLayout->addWidget(telemetryCheckBox);
        QHBoxLayout telemetryButtonLayout;
        telemetryButtonLayout.setObjectName(QStringLiteral("TelemetryButtonLayout"));
        telemetryLayout->addLayout(&telemetryButtonLayout);
        QSpacerItem *telemetryButtonSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
        telemetryButtonLayout.addSpacerItem(telemetryButtonSpacer);
        QPushButton *telemetryButton = new QPushButton(&telemetryDialog);
        telemetryButton->setObjectName(QStringLiteral("TelemetryButton"));
        telemetryButton->setText(QApplication::translate("TelemetryDialog", "&OK"));
        telemetryButtonLayout.addWidget(telemetryButton);
        QObject::connect(telemetryButton, &QPushButton::clicked, &telemetryDialog, &QDialog::close);
        telemetryDialog.setFixedSize(telemetryDialog.sizeHint());
        telemetryDialog.exec();
        QObject::disconnect(telemetryButton, &QPushButton::clicked, &telemetryDialog, &QDialog::close);
        if (telemetryCheckBox->isChecked()) {
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            telemetrySettings.setValue("PushUsageData", true);
            telemetrySettings.setValue("PushAppConf", true);
            telemetrySettings.endGroup();
            telemetrySettings.sync();
            Telemetry->init();
            Telemetry->work();
        }
        settings.setValue("PersonalUsageDataWindowLaunched", true);
    }
#endif
    settings.endGroup();

    for (const QString &currentArg : applicationArgs) {
        QString reworkedArg;
        if (currentArg.startsWith("-showpic=") && selectedAction == "") {
            reworkedArg = QString(currentArg).remove(0, 9);
            arg1 = reworkedArg;
            selectedAction = "showpic";
        }
        else if (currentArg.startsWith("-showsgd=") && selectedAction == "") {
            reworkedArg = QString(currentArg).remove(0, 9);
            arg1 = reworkedArg;
            selectedAction = "showsgd";
        }
        else if (selectedAction == "") {
            QFile argumentFile(currentArg);
            QFileInfo argumentFileInfo(argumentFile);
            if (argumentFile.exists()) {
                const QString argumentFileName = argumentFileInfo.fileName();
                if (argumentFileName.startsWith("PGTA5") || argumentFileName.startsWith("PRDR3") || argumentFileName.endsWith(".g5e")) {
                    arg1 = currentArg;
                    selectedAction = "showpic";
                }
                else if (argumentFileName.startsWith("SGTA5") || argumentFileName.startsWith("SRDR3")) {
                    arg1 = currentArg;
                    selectedAction = "showsgd";
                }
                else if (argumentFileName.startsWith("MISREP")) {
                    arg1 = currentArg;
                    selectedAction = "showsgd";
                }
            }
        }
    }

    if (selectedAction == "showpic") {
        CrewDatabase crewDB;
        ProfileDatabase profileDB;
        DatabaseThread threadDB(&crewDB);
        PictureDialog picDialog(true, &profileDB, &crewDB);
        SnapmaticPicture picture;

        bool readOk = picture.readingPictureFromFile(arg1);
        picDialog.setWindowIcon(IconLoader::loadingAppIcon());
        picDialog.setSnapmaticPicture(&picture, readOk);

        int crewID = picture.getSnapmaticProperties().crewID;
        if (crewID != 0)
            crewDB.addCrew(crewID);
        if (!readOk)
            return 1;

        QObject::connect(&threadDB, &DatabaseThread::crewNameFound, &crewDB, &CrewDatabase::setCrewName);
        QObject::connect(&threadDB, &DatabaseThread::crewNameUpdated, &picDialog, &PictureDialog::crewNameUpdated);
        QObject::connect(&threadDB, &DatabaseThread::playerNameFound, &profileDB, &ProfileDatabase::setPlayerName);
        QObject::connect(&threadDB, &DatabaseThread::playerNameUpdated, &picDialog, &PictureDialog::playerNameUpdated);
        QObject::connect(&threadDB, &DatabaseThread::finished, &a, &QApplication::quit);
        QObject::connect(&picDialog, &PictureDialog::endDatabaseThread, &threadDB, &DatabaseThread::terminateThread);
        threadDB.start();

        picDialog.show();

        return a.exec();
    }
    else if (selectedAction == "showsgd") {
        SavegameDialog savegameDialog;
        SavegameData savegame;

        bool readOk = savegame.readingSavegameFromFile(arg1);
        savegameDialog.setWindowIcon(IconLoader::loadingAppIcon());
        savegameDialog.setSavegameData(&savegame, arg1, readOk);

        if (!readOk)
            return 1;

        a.setQuitOnLastWindowClosed(true);
        savegameDialog.show();

        return a.exec();
    }

    CrewDatabase crewDB;
    ProfileDatabase profileDB;
    DatabaseThread threadDB(&crewDB);

    QObject::connect(&threadDB, &DatabaseThread::crewNameFound, &crewDB, &CrewDatabase::setCrewName);
    QObject::connect(&threadDB, &DatabaseThread::playerNameFound, &profileDB, &ProfileDatabase::setPlayerName);
    QObject::connect(&threadDB, &DatabaseThread::finished, &a, &QApplication::quit);
    threadDB.start();

#ifdef GTA5SYNC_MOTD
    uint cacheId;
    {
        QSettings messageSettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
        messageSettings.beginGroup("Messages");
        cacheId = messageSettings.value("CacheId", 0).toUInt();
        messageSettings.endGroup();
    }
    MessageThread threadMessage(cacheId);
    QObject::connect(&threadMessage, SIGNAL(finished()), &threadDB, SLOT(terminateThread()));
    threadMessage.start();
#endif

#ifdef GTA5SYNC_MOTD
    UserInterface uiWindow(&profileDB, &crewDB, &threadDB, &threadMessage);
    QObject::connect(&threadMessage, SIGNAL(messagesArrived(QJsonObject)), &uiWindow, SLOT(messagesArrived(QJsonObject)));
    QObject::connect(&threadMessage, SIGNAL(updateCacheId(uint)), &uiWindow, SLOT(updateCacheId(uint)));
#else
    UserInterface uiWindow(&profileDB, &crewDB, &threadDB);
#endif
    uiWindow.setWindowIcon(IconLoader::loadingAppIcon());
    uiWindow.setupDirEnv();
    uiWindow.show();

    return a.exec();
}
