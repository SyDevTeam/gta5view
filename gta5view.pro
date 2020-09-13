#/*****************************************************************************
#* gta5view Grand Theft Auto V Profile Viewer
#* Copyright (C) 2015-2020 Syping
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

QT += core gui network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): greaterThan(QT_MINOR_VERSION, 1): win32: QT += winextras

DEPLOYMENT.display_name = gta5view
TARGET = gta5view
TEMPLATE = app

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
    JsonEditorDialog.cpp \
    MapLocationDialog.cpp \
    MessageThread.cpp \
    OptionsDialog.cpp \
    PictureDialog.cpp \
    PictureExport.cpp \
    PictureWidget.cpp \
    PlayerListDialog.cpp \
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
    TelemetryClass.cpp \
    TranslationClass.cpp \
    UserInterface.cpp \
    anpro/imagecropper.cpp \
    pcg/pcg_basic.c \
    tmext/TelemetryClassAuthenticator.cpp \
    uimod/JSHighlighter.cpp \
    uimod/UiModLabel.cpp \
    uimod/UiModWidget.cpp

HEADERS += \
    AboutDialog.h \
    AppEnv.h \
    CrewDatabase.h \
    DatabaseThread.h \
    ExportDialog.h \
    ExportThread.h \
    GlobalString.h \
    IconLoader.h \
    ImportDialog.h \
    JsonEditorDialog.h \
    MapLocationDialog.h \
    MessageThread.h \
    OptionsDialog.h \
    PictureDialog.h \
    PictureExport.h \
    PictureWidget.h \
    PlayerListDialog.h \
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
    TelemetryClass.h \
    TranslationClass.h \
    UserInterface.h \
    anpro/imagecropper.h \
    anpro/imagecropper_e.h \
    anpro/imagecropper_p.h \
    pcg/pcg_basic.h \
    tmext/TelemetryClassAuthenticator.h \
    uimod/JSHighlighter.h \
    uimod/UiModLabel.h \
    uimod/UiModWidget.h

FORMS += \
    AboutDialog.ui \
    ExportDialog.ui \
    ImportDialog.ui \
    JsonEditorDialog.ui \
    MapLocationDialog.ui \
    OptionsDialog.ui \
    PictureDialog.ui \
    PlayerListDialog.ui \
    ProfileInterface.ui \
    SavegameDialog.ui \
    SavegameWidget.ui \
    SnapmaticEditor.ui \
    SnapmaticWidget.ui \
    UserInterface.ui

TRANSLATIONS += \
    res/gta5sync.ts \
    res/gta5sync_de.ts \
    res/gta5sync_en_US.ts \
    res/gta5sync_fr.ts \
    res/gta5sync_ko.ts \
    res/gta5sync_ru.ts \
    res/gta5sync_uk.ts \
    res/gta5sync_zh_TW.ts

RESOURCES += \
    res/app.qrc \
    res/tr_g5p.qrc

DISTFILES += \
    res/gta5view-16.png \
    res/gta5view-24.png \
    res/gta5view-32.png \
    res/gta5view-40.png \
    res/gta5view-48.png \
    res/gta5view-64.png \
    res/gta5view-96.png \
    res/gta5view-128.png \
    res/gta5view-256.png \
    res/gta5view-512.png \
    res/app.rc \
    res/de.syping.gta5view.desktop \
    res/de.syping.gta5view.png \
    res/gta5sync_de.ts \
    res/gta5sync_en_US.ts \
    res/gta5sync_fr.ts \
    res/gta5sync_ko.ts \
    res/gta5sync_ru.ts \
    res/gta5sync_uk.ts \
    res/gta5sync_zh_TW.ts \
    res/gta5view.exe.manifest \
    res/gta5view.png \
    lang/README.txt

INCLUDEPATH += ./anpro ./pcg ./tmext ./uimod

# GTA5SYNC/GTA5VIEW ONLY

DEFINES += GTA5SYNC_QMAKE # We using qmake do we?
DEFINES += GTA5SYNC_PROJECT # Enable exclusive gta5sync/gta5view functions
DEFINES += GTA5SYNC_NOASSIST # Not assisting at proper usage of SnapmaticPicture class

# WINDOWS ONLY

win32: DEFINES += GTA5SYNC_WIN
win32: RC_FILE += res/app.rc
win32: LIBS += -luser32
win32: CONFIG -= embed_manifest_exe
contains(DEFINES, GTA5SYNC_APV): greaterThan(QT_MAJOR_VERSION, 4): greaterThan(QT_MINOR_VERSION, 1): win32: LIBS += -ldwmapi
contains(DEFINES, GTA5SYNC_TELEMETRY): win32: LIBS += -ld3d9 # Required for getting information about GPU

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
isEqual(QT_MAJOR_VERSION, 4): GTA5SYNC_RCC = $$[QT_INSTALL_BINS]/rcc

# QT5 ONLY STUFF

isEqual(QT_MAJOR_VERSION, 5): RESOURCES += res/tr_qt5.qrc
isEqual(QT_MAJOR_VERSION, 5): GTA5SYNC_RCC = $$[QT_HOST_BINS]/rcc

# RESOURCE COMPILATION

system($$GTA5SYNC_RCC -threshold 0 -compress 9 $$PWD/res/global.qrc -o $$OUT_PWD/qrc_global.cpp) {
    SOURCES += $$OUT_PWD/qrc_global.cpp
} else {
    message("Failed to generate qrc_global.cpp")
}

# PROJECT INSTALLATION

isEmpty(GTA5SYNC_PREFIX): GTA5SYNC_PREFIX = /usr/local

appfiles.path = $$GTA5SYNC_PREFIX/share/applications
appfiles.files = $$PWD/res/de.syping.gta5view.desktop
pixmaps.path = $$GTA5SYNC_PREFIX/share/pixmaps
pixmaps.files = $$PWD/res/de.syping.gta5view.png
target.path = $$GTA5SYNC_PREFIX/bin
INSTALLS += target pixmaps appfiles

# QCONF BASED BUILD STUFF

contains(DEFINES, GTA5SYNC_QCONF) {
    isEqual(QT_MAJOR_VERSION, 4): RESOURCES -= res/tr_qt4.qrc
    isEqual(QT_MAJOR_VERSION, 5): RESOURCES -= res/tr_qt5.qrc
    !contains(DEFINES, GTA5SYNC_QCONF_IN) {
        RESOURCES -= res/tr_g5p.qrc
        langfiles.path = $$GTA5SYNC_PREFIX/share/gta5view/translations
        langfiles.files = $$PWD/res/gta5sync_en_US.qm $$PWD/res/gta5sync_de.qm $$PWD/res/gta5sync_fr.qm $$PWD/res/gta5sync_ko.qm $$PWD/res/gta5sync_ru.qm $$PWD/res/gta5sync_uk.qm $$PWD/res/gta5sync_zh_TW.qm $$PWD/res/qtbase_en_GB.qm
        INSTALLS += langfiles
    }
}

# TELEMETRY BASED STUFF

!contains(DEFINES, GTA5SYNC_TELEMETRY) {
    SOURCES -= TelemetryClass.cpp \
        tmext/TelemetryClassAuthenticator.cpp
    HEADERS -= TelemetryClass.h \
        tmext/TelemetryClassAuthenticator.h
}

!contains(DEFINES, GTA5SYNC_MOTD) {
    SOURCES -= MessageThread.cpp
    HEADERS -= MessageThread.h
} else {
    lessThan(QT_MAJOR_VERSION, 5) {
        SOURCES -= MessageThread.cpp
        HEADERS -= MessageThread.h
        DEFINES -= GTA5SYNC_MOTD
        message("Messages require Qt5 or newer!")
    }
}

# CMAKE BASED STUFF

unix: greaterThan(QT_MAJOR_VERSION, 4) {
    message("Building non-Windows gta5view with QMake is deprecated, please use CMake instead!")
}
