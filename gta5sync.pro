#/*****************************************************************************
#* gta5sync GRAND THEFT AUTO V SYNC
#* Copyright (C) 2015-2016 Syping Gaming Team
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
isEqual(QT_MAJOR_VERSION, 5): DEFINES += QT5_MODE

TARGET = gta5sync
TEMPLATE = app


SOURCES += main.cpp \
    SnapmaticPicture.cpp \
    PictureDialog.cpp \
    ProfileDatabase.cpp \
    DatabaseThread.cpp \
    CrewDatabase.cpp \
    SavegameData.cpp \
    SavegameDialog.cpp \
    SyncFramework.cpp \
    UserInterface.cpp \
    ProfileInterface.cpp

HEADERS  += \
    SnapmaticPicture.h \
    PictureDialog.h \
    ProfileDatabase.h \
    DatabaseThread.h \
    CrewDatabase.h \
    SavegameData.h \
    SavegameDialog.h \
    SyncFramework.h \
    UserInterface.h \
    ProfileInterface.h

FORMS    += \
    PictureDialog.ui \
    SavegameDialog.ui \
    UserInterface.ui \
    ProfileInterface.ui

TRANSLATIONS += \
    gta5sync_de.ts

RESOURCES += \
    app.qrc

OTHER_FILES += \
    app.rc

win32: RC_FILE += app.rc

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
