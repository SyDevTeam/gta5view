#!/bin/bash

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
mkdir -p build && \
mkdir -p assets && \
chmod -x res/gta5sync_*.qm res/gta5view.desktop res/gta5view.png && \
cd build && \
mkdir -p qt4 && \
cd qt4 && \
echo "Grand Theft Auto V Snapmatic and Savegame viewer/manager" > ./description-pak && \
cd .. && \
mkdir -p qt5 && \
cd qt5 && \
echo "Grand Theft Auto V Snapmatic and Savegame viewer/manager" > ./description-pak && \
cd .. && \

# Prepare checkinstall step
mkdir -p /usr/share/gta5view && \

# Starting build
cd qt5 && \
qmake -qt=5 -spec linux-clang GTA5SYNC_PREFIX=/usr QMAKE_CXXFLAGS+=-std=gnu++11 ${QMAKE_FLAGS_QT5} DEFINES+=GTA5SYNC_BUILDTYPE_DEV "DEFINES+=GTA5SYNC_BUILDCODE=\\\\\\\"${PACKAGE_CODE}\\\\\\\"" "DEFINES+=GTA5SYNC_APPVER=\\\\\\\"${APPLICATION_VERSION}\\\\\\\"" DEFINES+=GTA5SYNC_QCONF DEFINES+=GTA5SYNC_TELEMETRY "DEFINES+=GTA5SYNC_TELEMETRY_WEBURL=\\\\\\\"https://dev.syping.de/gta5view-userstats/\\\\\\\"" ../../gta5view.pro && \
make -j 4 && \
checkinstall -D --default --nodoc --install=no --pkgname=gta5view-qt5 --pkgversion=${PACKAGE_VERSION} --pkgrelease=${PACKAGE_BUILD} --pkggroup=utility --maintainer="Syping \<dpkg@syping.de\>" --requires=libqt5core5a,libqt5gui5,libqt5network5,libqt5widgets5,qttranslations5-l10n --conflicts=gta5view,gta5view-qt4 --replaces=gta5view,gta5view-qt4 --pakdir=${PROJECT_DIR}/assets && \
cd .. && \
cd qt4 && \
qmake -qt=4 GTA5SYNC_PREFIX=/usr QMAKE_CXXFLAGS+=-std=gnu++11 ${QMAKE_FLAGS_QT4} DEFINES+=GTA5SYNC_BUILDTYPE_DEV "DEFINES+=GTA5SYNC_BUILDCODE=\\\\\\\"${PACKAGE_CODE}\\\\\\\"" "DEFINES+=GTA5SYNC_APPVER=\\\\\\\"${APPLICATION_VERSION}\\\\\\\"" DEFINES+=GTA5SYNC_QCONF ../../gta5view.pro && \
make -j 4 && \
checkinstall -D --default --nodoc --install=no --pkgname=gta5view-qt4 --pkgversion=${PACKAGE_VERSION} --pkgrelease=${PACKAGE_BUILD} --pkggroup=utility --maintainer="Syping \<dpkg@syping.de\>" --requires=libqtcore4,libqtgui4,libqt4-network,qtcore4-l10n --conflicts=gta5view,gta5view-qt5 --replaces=gta5view,gta5view-qt5 --pakdir=${PROJECT_DIR}/assets
