#!/bin/bash

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
echo "gta5view image name is gta5view-osx_${APPLICATION_VERSION}.dmg" && \
mkdir -p build && \
mkdir -p assets && \
cd build && \

/usr/local/opt/qt/bin/qmake ${QMAKE_FLAGS_QT5} ${QMAKE_BUILD_TYPE} "DEFINES+=GTA5SYNC_BUILDCODE=\\\\\\\"${PACKAGE_CODE}\\\\\\\"" "DEFINES+=GTA5SYNC_APPVER=\\\\\\\"${APPLICATION_VERSION}\\\\\\\"" "DEFINES+=GTA5SYNC_COMMIT=\\\\\\\"${APPLICATION_COMMIT}\\\\\\\"" DEFINES+=GTA5SYNC_TELEMETRY "DEFINES+=GTA5SYNC_TELEMETRY_WEBURL=\\\\\\\"https://dev.syping.de/gta5view-userstats/\\\\\\\"" ../gta5view.pro && \
make depend && \
make -j 4 && \
/usr/local/opt/qt/bin/macdeployqt gta5view.app -dmg && \
cp -Rf gta5view.dmg ../assets/gta5view-osx_${APPLICATION_VERSION}.dmg
