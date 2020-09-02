#!/bin/bash

# Install nsis
apt-get update -qq && \
apt-get install -qq nsis && \

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
mkdir -p build && \
mkdir -p assets && \

# Starting build
cd build && \
qmake ${QMAKE_FLAGS_QT5} ${QMAKE_BUILD_TYPE} "DEFINES+=GTA5SYNC_BUILDCODE=\\\\\\\"${PACKAGE_CODE}\\\\\\\"" "DEFINES+=GTA5SYNC_APPVER=\\\\\\\"${APPLICATION_VERSION}\\\\\\\"" "DEFINES+=GTA5SYNC_COMMIT=\\\\\\\"${APPLICATION_COMMIT}\\\\\\\"" DEFINES+=GTA5SYNC_TELEMETRY "DEFINES+=GTA5SYNC_TELEMETRY_WEBURL=\\\\\\\"https://dev.syping.de/gta5view-userstats/\\\\\\\"" DEFINES+=GTA5SYNC_QCONF DEFINES+=GTA5SYNC_INLANG='\\\"RUNDIR:SEPARATOR:lang\\\"' DEFINES+=GTA5SYNC_LANG='\\\"RUNDIR:SEPARATOR:lang\\\"' DEFINES+=GTA5SYNC_PLUG='\\\"RUNDIR:SEPARATOR:plugins\\\"' ../gta5view.pro && \
make -j 4 && \
cd ${PROJECT_DIR}/assets && \
makensis -NOCD ${PROJECT_DIR}/.ci/gta5view.nsi && \
mv -f gta5view_setup.exe gta5view-${EXECUTABLE_VERSION}_setup.exe
