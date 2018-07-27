#!/bin/bash

if [ $(git name-rev --tags --name-only $(git rev-parse HEAD)) == "undefined" ]; then
   export APPLICATION_VERSION=$(lua -e 'for line in io.lines("config.h") do local m = string.match(line, "#define GTA5SYNC_APPVER \"(.+)\"$"); if m then print(m); os.exit(0) end end')
else
   export APPLICATION_VERSION=$(git name-rev --tags --name-only $(git rev-parse HEAD))
fi
export PACKAGE_VERSION=$(grep -oE '^[^\-]*' <<< $APPLICATION_VERSION)
export PACKAGE_BUILD=$(grep -oP '\-\K.+' <<< $APPLICATION_VERSION)
export EXECUTABLE_VERSION=${PACKAGE_VERSION}${PACKAGE_BUILD}${EXECUTABLE_TAG}

if [ "${PACKAGE_BUILD}" == "" ]; then
   export PACKAGE_BUILD=1;
fi

if [ "${BUILD_TYPE}" == "ALPHA" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_ALPHA"
elif [ "${BUILD_TYPE}" == "Alpha" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_ALPHA"
elif [ "${BUILD_TYPE}" == "BETA" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_BETA"
elif [ "${BUILD_TYPE}" == "Beta" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_BETA"
elif [ "${BUILD_TYPE}" == "DEV" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_DEV"
elif [ "${BUILD_TYPE}" == "Development" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_DEV"
elif [ "${BUILD_TYPE}" == "DAILY" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_DAILY"
elif [ "${BUILD_TYPE}" == "Daily" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_DAILY"
elif [ "${BUILD_TYPE}" == "RC" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_RC"
elif [ "${BUILD_TYPE}" == "Release Candidate" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_RC"
elif [ "${BUILD_TYPE}" == "REL" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_REL"
elif [ "${BUILD_TYPE}" == "Release" ]; then
   export QMAKE_BUILD_TYPE="DEFINES+=GTA5SYNC_BUILDTYPE_REL"
fi

export PROJECT_DIR=$(pwd)

.ci/${BUILD_SCRIPT}
