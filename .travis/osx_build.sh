#!/bin/bash

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
mkdir build && \
mkdir assets && \
cd build && \

/usr/local/opt/qt/bin/qmake ${QMAKE_FLAGS_QT5} DEFINES+=GTA5SYNC_BUILDTYPE_RC "DEFINES+=GTA5SYNC_APPVER=\\\\\\\"${APPLICATION_VERSION}\\\\\\\"" DEFINES+=GTA5SYNC_TELEMETRY "DEFINES+=GTA5SYNC_TELEMETRY_WEBURL=\\\\\\\"https://dev.syping.de/gta5view-userstats/\\\\\\\"" ../gta5view.pro && \
make -j 4 && \
/usr/local/opt/qt/bin/macdeployqt gta5view.app -dmg && \
cp -Rf gta5view.dmg ../assets/gta5view.dmg
