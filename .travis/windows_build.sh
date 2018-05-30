#!/bin/bash

apt-get update -qq && \
apt-get install -qq curl && \

export GTA5VIEW_EXECUTABLE=gta5view-${EXECUTABLE_VERSION}${EXECUTABLE_ARCH}.exe && \

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
echo "gta5view executable is ${GTA5VIEW_EXECUTABLE}" && \
mkdir -p build && \
mkdir -p assets && \

# Starting build
cd build && \
qmake-static ${QMAKE_FLAGS} DEFINES+=GTA5SYNC_BUILDTYPE_DEV "DEFINES+=GTA5SYNC_BUILDCODE=\\\\\\\"${PACKAGE_CODE}\\\\\\\"" "DEFINES+=GTA5SYNC_APPVER=\\\\\\\"${APPLICATION_VERSION}\\\\\\\"" DEFINES+=GTA5SYNC_TELEMETRY "DEFINES+=GTA5SYNC_TELEMETRY_WEBURL=\\\\\\\"https://dev.syping.de/gta5view-userstats/\\\\\\\"" ../gta5view.pro && \
make -j 4 && \
cp -Rf release/*.exe ${PROJECT_DIR}/assets/${GTA5VIEW_EXECUTABLE} && \
cd ${PROJECT_DIR}/assets && \
upx --best ${GTA5VIEW_EXECUTABLE} && \

if [ "${PACKAGE_CODE}" == "Dropbox" ]; then
	${PROJECT_DIR}/.travis/dropbox_uploader.sh mkdir gta5view-builds/${PACKAGE_VERSION}
	${PROJECT_DIR}/.travis/dropbox_uploader.sh upload ${GTA5VIEW_EXECUTABLE} gta5view-builds/${PACKAGE_VERSION}/${GTA5VIEW_EXECUTABLE} && \
	rm -rf ${GTA5VIEW_EXECUTABLE}
elif [ "${PACKAGE_CODE}" == "gta5-mods" ]; then
	${PROJECT_DIR}/.travis/dropbox_uploader.sh mkdir gta5-mods/${PACKAGE_VERSION}
	${PROJECT_DIR}/.travis/dropbox_uploader.sh upload ${GTA5VIEW_EXECUTABLE} gta5-mods/${PACKAGE_VERSION}/${GTA5VIEW_EXECUTABLE} && \
	rm -rf ${GTA5VIEW_EXECUTABLE}
fi
