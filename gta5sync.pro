#/*****************************************************************************
#* gta5sync GRAND THEFT AUTO V SYNC
#* Copyright (C) 2015-2016 Syping Gaming Team
#*
#* Licensed under the Apache License, Version 2.0 (the "License");
#* you may not use this file except in compliance with the License.
#* You may obtain a copy of the License at
#*
#*     http://www.apache.org/licenses/LICENSE-2.0
#*
#* Unless required by applicable law or agreed to in writing, software
#* distributed under the License is distributed on an "AS IS" BASIS,
#* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#* See the License for the specific language governing permissions and
#* limitations under the License.
#*****************************************************************************/

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gta5sync
TEMPLATE = app


SOURCES += main.cpp\
    SnapmaticPicture.cpp \
    PictureDialog.cpp

HEADERS  += \
    SnapmaticPicture.h \
    PictureDialog.h

FORMS    += \
    PictureDialog.ui

RESOURCES +=

OTHER_FILES +=
