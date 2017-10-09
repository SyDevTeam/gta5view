#/*****************************************************************************
#* gta5view Grand Theft Auto V Profile Viewer
#* Copyright (C) 2015-2017 Syping
#*
#* This program is free software: you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
#* the Free Software Foundation, either version 3 of the License, or
#* (at your option) any later version.
#*
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*****************************************************************************/

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): greaterThan(QT_MINOR_VERSION, 1): win32: QT += winextras

DEFINES += GTA5SYNC_DISABLED

DEPLOYMENT.display_name = gta5view
TARGET = gta5view
TEMPLATE = app

DEFINES += GTA5SYNC_CSDF # Not assisting at proper usage of SnapmaticPicture class
HEADERS += config.h
PRECOMPILED_HEADER += config.h

SOURCES += main.cpp \
    AboutDialog.cpp \
    AppEnv.cpp \
    CrewDatabase.cpp \
    DatabaseThread.cpp \
    ExportDialog.cpp \
    ExportThread.cpp \
    GlobalString.cpp \
    IconLoader.cpp \
    ImportDialog.cpp \
    MapPreviewDialog.cpp \
    OptionsDialog.cpp \
    PictureDialog.cpp \
    PictureExport.cpp \
    PictureWidget.cpp \
    ProfileDatabase.cpp \
    ProfileInterface.cpp \
    ProfileLoader.cpp \
    ProfileWidget.cpp \
    SavegameCopy.cpp \
    SavegameData.cpp \
    SavegameDialog.cpp \
    SavegameWidget.cpp \
    SidebarGenerator.cpp \
    SnapmaticEditor.cpp \
    SnapmaticPicture.cpp \
    SnapmaticWidget.cpp \
    StandardPaths.cpp \
    StringParser.cpp \
    TranslationClass.cpp \
    UserInterface.cpp \
    uimod/UiModLabel.cpp \
    uimod/UiModWidget.cpp

HEADERS  += \
    AboutDialog.h \
    AppEnv.h \
    CrewDatabase.h \
    DatabaseThread.h \
    ExportDialog.h \
    ExportThread.h \
    GlobalString.h \
    IconLoader.h \
    ImportDialog.h \
    MapPreviewDialog.h \
    OptionsDialog.h \
    PictureDialog.h \
    PictureExport.h \
    PictureWidget.h \
    ProfileDatabase.h \
    ProfileInterface.h \
    ProfileLoader.h \
    ProfileWidget.h \
    SavegameCopy.h \
    SavegameData.h \
    SavegameDialog.h \
    SavegameWidget.h \
    SidebarGenerator.h \
    SnapmaticEditor.h \
    SnapmaticPicture.h \
    SnapmaticWidget.h \
    StandardPaths.h \
    StringParser.h \
    TranslationClass.h \
    UserInterface.h \
    uimod/UiModLabel.h \
    uimod/UiModWidget.h

FORMS    += \
    AboutDialog.ui \
    ExportDialog.ui \
    ImportDialog.ui \
    MapPreviewDialog.ui \
    OptionsDialog.ui \
    PictureDialog.ui \
    ProfileInterface.ui \
    SavegameDialog.ui \
    SavegameWidget.ui \
    SnapmaticEditor.ui \
    SnapmaticWidget.ui \
    UserInterface.ui

TRANSLATIONS += \
    res/gta5sync_en_US.ts \
    res/gta5sync_de.ts \
    res/gta5sync_fr.ts \
    res/gta5sync_ru.ts \
    lang/gta5sync_no.ts

RESOURCES += \
    res/tr_g5p.qrc \
    res/app.qrc

DISTFILES += res/app.rc \
    res/gta5view.desktop \
    res/gta5sync_de.ts \
    res/gta5sync_fr.ts \
    res/gta5sync_ru.ts \
    res/gta5view.exe.manifest \
    res/gta5view.png \
    lang/gta5sync_no.ts \
    lang/README.txt

INCLUDEPATH += ./uimod

# WINDOWS ONLY

win32: DEFINES += GTA5SYNC_WIN
win32: RC_FILE += res/app.rc
win32: LIBS += -luser32
win32: CONFIG -= embed_manifest_exe

# MAC OS X ONLY
macx: ICON = res/5sync.icns

# QT4 ONLY STUFF

isEqual(QT_MAJOR_VERSION, 4): INCLUDEPATH += ./qjson4
isEqual(QT_MAJOR_VERSION, 4): HEADERS += qjson4/QJsonArray.h \
    qjson4/QJsonDocument.h \
    qjson4/QJsonObject.h \
    qjson4/QJsonParseError.h \
    qjson4/QJsonValue.h \
    qjson4/QJsonValueRef.h \
    qjson4/QJsonParser.h \
    qjson4/QJsonRoot.h

isEqual(QT_MAJOR_VERSION, 4): SOURCES += qjson4/QJsonArray.cpp \
    qjson4/QJsonDocument.cpp \
    qjson4/QJsonObject.cpp \
    qjson4/QJsonParseError.cpp \
    qjson4/QJsonValue.cpp \
    qjson4/QJsonValueRef.cpp \
    qjson4/QJsonParser.cpp

isEqual(QT_MAJOR_VERSION, 4): RESOURCES += res/tr_qt4.qrc

# QT5 ONLY STUFF
isEqual(QT_MAJOR_VERSION, 5): RESOURCES += res/tr_qt5.qrc

# PROJECT INSTALLATION

isEmpty(GTA5SYNC_PREFIX): GTA5SYNC_PREFIX = /usr/local

appfiles.path = $$GTA5SYNC_PREFIX/share/applications
appfiles.files = $$PWD/res/gta5view.desktop
pixmaps.path = $$GTA5SYNC_PREFIX/share/pixmaps
pixmaps.files = $$PWD/res/gta5view.png
target.path = $$GTA5SYNC_PREFIX/bin
INSTALLS += target pixmaps appfiles

# QCONF BASED BUILD STUFF

contains(DEFINES, GTA5SYNC_QCONF){
    isEqual(QT_MAJOR_VERSION, 4): RESOURCES -= res/tr_qt4.qrc
    isEqual(QT_MAJOR_VERSION, 5): RESOURCES -= res/tr_qt5.qrc
    !contains(DEFINES, GTA5SYNC_QCONF_IN){
        RESOURCES -= res/tr_g5p.qrc
        langfiles.path = $$GTA5SYNC_PREFIX/share/gta5view/translations
        langfiles.files = $$PWD/res/gta5sync_en_US.qm $$PWD/res/gta5sync_de.qm $$PWD/res/gta5sync_fr.qm $$PWD/res/gta5sync_ru.qm $$PWD/res/qtbase_en_GB.qm
        INSTALLS += langfiles
    }
}
