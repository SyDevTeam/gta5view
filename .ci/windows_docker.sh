#!/bin/bash

DOCKER_IMAGE=sypingauto/gta5view-build:1.7-static
PROJECT_DIR_DOCKER=/gta5view

cd ${PROJECT_DIR} && \
docker pull ${DOCKER_IMAGE} && \
docker run --rm \
	-v "${PROJECT_DIR}:${PROJECT_DIR_DOCKER}" \
	${DOCKER_IMAGE} \
	/bin/bash -c "export PROJECT_DIR=${PROJECT_DIR_DOCKER} && export QT_SELECT=${QT_SELECT} && export APPLICATION_VERSION=${APPLICATION_VERSION} && export APPLICATION_COMMIT=${APPLICATION_COMMIT} && export BUILD_TYPE=${BUILD_TYPE} && export QMAKE_FLAGS_QT4=${QMAKE_FLAGS_QT4} && export QMAKE_FLAGS_QT5=${QMAKE_FLAGS_QT5} && export QMAKE_BUILD_TYPE=${QMAKE_BUILD_TYPE} && export PACKAGE_VERSION=${PACKAGE_VERSION} && export PACKAGE_BUILD=${PACKAGE_BUILD} && export PACKAGE_CODE=${PACKAGE_CODE} && export EXECUTABLE_VERSION=${EXECUTABLE_VERSION} && export EXECUTABLE_ARCH=${EXECUTABLE_ARCH} && cd ${PROJECT_DIR_DOCKER} && .ci/windows_build.sh" && \

# Prepare environment variable
export GTA5VIEW_EXECUTABLE=gta5view-${EXECUTABLE_VERSION}${EXECUTABLE_ARCH}.exe && \

# Upload Assets to Dropbox
if [ "${PACKAGE_CODE}" == "gta5-mods" ]; then
	${PROJECT_DIR}/.ci/dropbox_uploader.sh mkdir gta5-mods/${PACKAGE_VERSION}
	${PROJECT_DIR}/.ci/dropbox_uploader.sh upload ${PROJECT_DIR}/assets/${GTA5VIEW_EXECUTABLE} gta5-mods/${PACKAGE_VERSION}/${GTA5VIEW_EXECUTABLE} && \
	rm -rf ${GTA5VIEW_EXECUTABLE}
fi
