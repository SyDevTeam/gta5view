#!/bin/bash

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
mkdir -p build && \
mkdir -p assets && \

# Starting build
cd build && \
mingw64-qt-cmake \
   ${CMAKE_BUILD_TYPE} \
   "-DGTA5VIEW_BUILDCODE=${PACKAGE_CODE}" \
   "-DGTA5VIEW_APPVER=${APPLICATION_VERSION}" \
   "-DGTA5VIEW_COMMIT=${APPLICATION_COMMIT}" \
   "-DWITH_TELEMETRY=ON" \
   "-DTELEMETRY_WEBURL=https://dev.syping.de/gta5view-userstats/" \
   "-DQCONF_BUILD=ON" \
   "-DGTA5VIEW_INLANG=RUNDIR:SEPARATOR:lang" \
   "-DGTA5VIEW_LANG=RUNDIR:SEPARATOR:lang" \
   "-DGTA5VIEW_PLUG=RUNDIR:SEPARATOR:plugins" \
   .. && \
make -j 4 && \
cd ${PROJECT_DIR}/assets && \
makensis -NOCD ${PROJECT_DIR}/.ci/gta5view.nsi && \
mv -f gta5view_setup.exe gta5view-${EXECUTABLE_VERSION}_setup.exe
