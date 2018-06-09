#!/bin/bash

apt-get update -qq && \
apt-get install -qq lua nsis && \

export PACKAGE_CODE=GitLab && \

if [ `git name-rev --tags --name-only $(git rev-parse HEAD)` == "undefined" ]; then export APPLICATION_VERSION=`lua -e 'for line in io.lines("config.h") do local m = string.match(line, "#define GTA5SYNC_APPVER \"(.+)\"$"); if m then print(m); os.exit(0) end end'`; else export APPLICATION_VERSION=`git name-rev --tags --name-only $(git rev-parse HEAD)`; fi
export PACKAGE_VERSION=$(grep -oE '^[^\-]*' <<< $APPLICATION_VERSION) && \
export PACKAGE_BUILD=$(grep -oP '\-\K.+' <<< $APPLICATION_VERSION) && \
export EXECUTABLE_VERSION=${PACKAGE_VERSION}${PACKAGE_BUILD}${EXECUTABLE_TAG} && \
if [[ ! ${PACKAGE_BUILD} ]]; then export PACKAGE_BUILD=1; fi

export PROJECT_DIR=$(pwd) && \

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
mkdir -p build && \
mkdir -p assets && \

# Starting build
cd build && \
qmake ${QMAKE_FLAGS} DEFINES+=GTA5SYNC_BUILDTYPE_DEV "DEFINES+=GTA5SYNC_BUILDCODE=\\\\\\\"${PACKAGE_CODE}\\\\\\\"" "DEFINES+=GTA5SYNC_APPVER=\\\\\\\"${APPLICATION_VERSION}\\\\\\\"" DEFINES+=GTA5SYNC_QCONF DEFINES+=GTA5SYNC_INLANG='\\\"RUNDIR:SEPARATOR:lang\\\"' DEFINES+=GTA5SYNC_LANG='\\\"RUNDIR:SEPARATOR:lang\\\"' DEFINES+=GTA5SYNC_PLUG='\\\"RUNDIR:SEPARATOR:plugins\\\"' ../gta5view.pro && \
make -j 4 && \
cd ${PROJECT_DIR}/assets && \
makensis -NOCD ${PROJECT_DIR}/.travis/gta5view.nsi && \
mv -f gta5view_setup.exe gta5view-${EXECUTABLE_VERSION}_setup.exe
