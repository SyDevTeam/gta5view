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

#include "SnapmaticPicture.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "SavegameDialog.h"
#include "PictureDialog.h"
#include "UserInterface.h"
#include "CrewDatabase.h"
#include "SavegameData.h"
#include "IconLoader.h"
#include "AppEnv.h"
#include "config.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QStringList>
#include <QTranslator>
#include <QMessageBox>
#include <QFileInfo>
#include <QSysInfo>
#include <QRawFont>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFont>
#include <QFile>
#include <QDir>

#ifdef GTA5SYNC_WIN
#include "windows.h"
#include <iostream>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(GTA5SYNC_APPSTR);
    a.setApplicationVersion(GTA5SYNC_APPVER);

#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050400
    if (QSysInfo::windowsVersion() >= 0x0080)
    {
        // Get Windows Font
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
        LOGFONTW uiFont = ncm.lfMessageFont;
        QString uiFontStr(QString::fromStdWString(std::wstring(uiFont.lfFaceName)));

#ifdef GTA5SYNC_DEBUG
        QMessageBox::information(a.desktop(), QApplication::tr("Font"), QApplication::tr("Selected Font: %1").arg(uiFontStr));
#endif

        // Set Application Font
        QFont appFont(uiFontStr, 9);
        a.setFont(appFont);
    }
#endif
#endif

    QString pluginsDir = AppEnv::getPluginsFolder();
    if (QFileInfo(pluginsDir).exists())
    {
        a.addLibraryPath(pluginsDir);
    }

    // Loading translation settings
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Interface");
    QString language = settings.value("Language","System").toString();
    settings.endGroup();

    // Start external translate loading
    QString langpath = AppEnv::getLangFolder();
    bool trsf = false;
    bool svlp = false;
    QTranslator EappTranslator;
    if (language == "System" || language.trimmed() == "")
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "gta5sync_" + langList.at(0) + ".qm"))
            {
                EappTranslator.load(langpath + QDir::separator() + "/gta5sync_" + langList.at(0) + ".qm");
                QLocale::setDefault(QLocale::system());
            }
        }
    }
    else
    {
        QString languageName = language;
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "gta5sync_" + langList.at(0) + ".qm"))
            {
                if (!EappTranslator.load(langpath + QDir::separator() + "gta5sync_" + langList.at(0) + ".qm"))
                {
                    if (langList.at(0) != "en")
                    {
                        trsf = true;
                    }
                }
                else
                {
                    QLocale::setDefault(QLocale(langList.at(0)));
                    svlp = true;
                }
            }
            else
            {
                if (langList.at(0) != "en")
                {
                    trsf = true;
                }
            }
        }
    }
    if (trsf)
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "gta5sync_" + langList.at(0) + ".qm"))
            {
                EappTranslator.load(langpath + QDir::separator() + "gta5sync_" + langList.at(0) + ".qm");
                QLocale::setDefault(QLocale(langList.at(0)));
            }
        }
    }
    a.installTranslator(&EappTranslator);
#if QT_VERSION >= 0x050000
    QTranslator EqtTranslator1;
    if (language == "System" || language.trimmed() == "")
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "qtbase_" + langList.at(0) + ".qm"))
            {
                EqtTranslator1.load(langpath + QDir::separator() + "qtbase_" + langList.at(0) + ".qm");
            }
        }
    }
    else
    {
        QString languageName = language;
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "qtbase_" + langList.at(0) + ".qm"))
            {
                EqtTranslator1.load(langpath + QDir::separator() + "qtbase_" + langList.at(0) + ".qm");
            }
        }
    }
    if (trsf)
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "qtbase_" + langList.at(0) + ".qm"))
            {
                EqtTranslator1.load(langpath + QDir::separator() + "qtbase_" + langList.at(0) + ".qm");
            }
        }
    }
    a.installTranslator(&EqtTranslator1);
#else
    QTranslator EqtTranslator;
    if (language == "System" || language.trimmed() == "")
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "qt_" + langList.at(0) + ".qm"))
            {
                EqtTranslator.load(langpath + QDir::separator() + "qt_" + langList.at(0) + ".qm");
            }
        }
    }
    else
    {
        QString languageName = language;
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "qt_" + langList.at(0) + ".qm"))
            {
                EqtTranslator.load(langpath + QDir::separator() + "qt_" + langList.at(0) + ".qm");
            }
        }
    }
    if (trsf)
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(langpath + QDir::separator() + "qt_" + langList.at(0) + ".qm"))
            {
                EqtTranslator.load(langpath + QDir::separator() + "qt_" + langList.at(0) + ".qm");
            }
        }
    }
    a.installTranslator(&EqtTranslator);
#endif
    // End external translate loading
    // Start internal translate loading
    QTranslator appTranslator;
    trsf = false;
    if (language == "System" || language.trimmed() == "")
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/gta5sync_" + langList.at(0) + ".qm"))
            {
                if (!appTranslator.load(":/tr/gta5sync_" + langList.at(0) + ".qm"))
                {
                    if (langList.at(0) != "en")
                    {
                        if (svlp) { trsf = true; }
                    }
                }
                else
                {
                    QLocale::setDefault(QLocale(langList.at(0)));
                }
            }
            else
            {
                if (langList.at(0) != "en")
                {
                    if (svlp) { trsf = true; }
                }
            }
        }
    }
    else if (language == "en" || language == "English")
    {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::AnyCountry));
    }
    else
    {
        QString languageName = language;
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/gta5sync_" + langList.at(0) + ".qm"))
            {
                appTranslator.load(":/tr/gta5sync_" + langList.at(0) + ".qm");
                QLocale::setDefault(QLocale(langList.at(0)));

            }
        }
    }
    if (trsf)
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/gta5sync_" + langList.at(0) + ".qm"))
            {
                appTranslator.load(":/tr/gta5sync_" + langList.at(0) + ".qm");
                QLocale::setDefault(QLocale(langList.at(0)));
            }
        }
    }
    a.installTranslator(&appTranslator);
#if QT_VERSION >= 0x050000
    QTranslator qtTranslator1;
    if (language == "System" || language.trimmed() == "")
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/qtbase_" + langList.at(0) + ".qm"))
            {
                qtTranslator1.load(":/tr/qtbase_" + langList.at(0) + ".qm");
            }
        }
    }
    else
    {
        QString languageName = language;
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/qtbase_" + langList.at(0) + ".qm"))
            {
                qtTranslator1.load(":/tr/qtbase_" + langList.at(0) + ".qm");
            }
        }
    }
    if (trsf)
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/qtbase_" + langList.at(0) + ".qm"))
            {
                qtTranslator1.load(":/tr/qtbase_" + langList.at(0) + ".qm");
            }
        }
    }
    a.installTranslator(&qtTranslator1);
#else
    QTranslator qtTranslator1;
    if (language == "System" || language.trimmed() == "")
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/qt_" + langList.at(0) + ".qm"))
            {
                qtTranslator1.load(":/tr/qt_" + langList.at(0) + ".qm");
            }
        }
    }
    else
    {
        QString languageName = language;
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/qt_" + langList.at(0) + ".qm"))
            {
                qtTranslator1.load(":/tr/qt_" + langList.at(0) + ".qm");
            }
        }
    }
    if (trsf)
    {
        QString languageName = QLocale::system().name();
        QStringList langList = languageName.split("_");
        if (langList.length() >= 1)
        {
            if (QFile::exists(":/tr/qt_" + langList.at(0) + ".qm"))
            {
                qtTranslator1.load(":/tr/qt_" + langList.at(0) + ".qm");
            }
        }
    }
    a.installTranslator(&qtTranslator1);
#endif
    // End internal translate loading

    QStringList applicationArgs = a.arguments();
    QString selectedAction;
    QString arg1;
    applicationArgs.removeAt(0);

    foreach(QString currentArg, applicationArgs)
    {
        QString reworkedArg;
        if (currentArg.left(9) == "-showpic=" && selectedAction == "")
        {
            reworkedArg = currentArg.remove(0,9);
            arg1 = reworkedArg;
            selectedAction = "showpic";
        }
        else if (currentArg.left(9) == "-showsgd=" && selectedAction == "")
        {
            reworkedArg = currentArg.remove(0,9);
            arg1 = reworkedArg;
            selectedAction = "showsgd";
        }
        else if (selectedAction == "")
        {
            QFile argumentFile(currentArg);
            QFileInfo argumentFileInfo(argumentFile);
            if (argumentFile.exists())
            {
                QString argumentFileName = argumentFileInfo.fileName();
                QString argumentFileType = argumentFileName.left(4);
                QString argumentFileExt = argumentFileName.right(4);

                if (argumentFileType == "PGTA" || argumentFileExt == ".g5e")
                {
                    arg1 = currentArg;
                    selectedAction = "showpic";
                }
                else if (argumentFileType == "SGTA")
                {
                    arg1 = currentArg;
                    selectedAction = "showsgd";
                }
                else if (argumentFileType == "MISR")
                {
                    arg1 = currentArg;
                    selectedAction = "showsgd";
                }
            }
        }
    }

    if (selectedAction == "showpic")
    {
        CrewDatabase crewDB;
        ProfileDatabase profileDB;
        DatabaseThread threadDB(&crewDB);
        PictureDialog picDialog(true, &profileDB, &crewDB);
        SnapmaticPicture picture;

        bool readOk = picture.readingPictureFromFile(arg1);
        picDialog.setWindowFlags(picDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
        picDialog.setWindowIcon(IconLoader::loadingAppIcon());
        picDialog.setSnapmaticPicture(&picture, readOk);

        int crewID = picture.getSnapmaticProperties().crewID;
        if (crewID != 0) { crewDB.addCrew(crewID); }
        if (!readOk) { return 1; }

        QEventLoop threadLoop;
        QObject::connect(&threadDB, SIGNAL(playerNameFound(int, QString)), &profileDB, SLOT(setPlayerName(int, QString)));
        QObject::connect(&threadDB, SIGNAL(playerNameUpdated()), &picDialog, SLOT(playerNameUpdated()));
        QObject::connect(&threadDB, SIGNAL(finished()), &threadLoop, SLOT(quit()));
        QObject::connect(&picDialog, SIGNAL(endDatabaseThread()), &threadDB, SLOT(doEndThread()));
        threadDB.start();

        picDialog.show();

        threadLoop.exec();

        return 0;
    }
    else if (selectedAction == "showsgd")
    {
        SavegameDialog savegameDialog;
        SavegameData savegame;

        bool readOk = savegame.readingSavegameFromFile(arg1);
        savegameDialog.setWindowFlags(savegameDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
        savegameDialog.setWindowIcon(IconLoader::loadingAppIcon());
        savegameDialog.setSavegameData(&savegame, arg1, readOk);

        if (!readOk) { return 1; }

        savegameDialog.show();

        return a.exec();
    }

    CrewDatabase crewDB;
    ProfileDatabase profileDB;
    DatabaseThread threadDB(&crewDB);

    QEventLoop threadLoop;
    QObject::connect(&threadDB, SIGNAL(playerNameFound(int, QString)), &profileDB, SLOT(setPlayerName(int, QString)));
    QObject::connect(&threadDB, SIGNAL(finished()), &threadLoop, SLOT(quit()));
    threadDB.start();

    UserInterface uiWindow(&profileDB, &crewDB, &threadDB);
    uiWindow.setWindowIcon(IconLoader::loadingAppIcon());
    uiWindow.setupDirEnv();
    uiWindow.show();

    threadLoop.exec();

    return 0;
}

