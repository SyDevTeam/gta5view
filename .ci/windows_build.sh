#!/usr/bin/env bash

# Prepare environment variable
export GTA5VIEW_EXECUTABLE=gta5view-${EXECUTABLE_VERSION}${EXECUTABLE_ARCH}.exe && \

# Creating folders
cd ${PROJECT_DIR} && \
echo "gta5view build version is ${APPLICATION_VERSION}" && \
echo "gta5view executable is ${GTA5VIEW_EXECUTABLE}" && \
mkdir -p build && \
mkdir -p assets && \

# Starting build
cd build && \
mingw64-qt-cmake \
   "${CMAKE_BUILD_TYPE}" \
   "-DGTA5VIEW_BUILDCODE=${PACKAGE_CODE}" \
   "-DGTA5VIEW_APPVER=${APPLICATION_VERSION}" \
   "-DGTA5VIEW_COMMIT=${APPLICATION_COMMIT}" \
   "-DWITH_DONATE=ON" \
   "-DWITH_TELEMETRY=ON" \
   "-DDONATE_ADDRESSES=$(cat ${PROJECT_DIR}/.ci/donate.txt)" \
   "-DTELEMETRY_WEBURL=https://dev.syping.de/gta5view-userstats/" \
   .. && \
make -j 4 && \
x86_64-w64-mingw32-strip -s gta5view.exe && \
cp -Rf *.exe ${PROJECT_DIR}/assets/${GTA5VIEW_EXECUTABLE} && \
cd ${PROJECT_DIR}/assets
