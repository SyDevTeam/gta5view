#!/bin/bash

DOCKER_IMAGE=i386/debian:jessie
PROJECT_DIR_DOCKER=/gta5view/

cd $PROJECT_DIR && \
docker pull $DOCKER_IMAGE && \
docker run --rm \
	-v "${PROJECT_DIR}:${PROJECT_DIR_DOCKER}" \
	$DOCKER_IMAGE \
	/bin/bash -c "export PROJECT_DIR=${PROJECT_DIR_DOCKER} && export QT_SELECT=${QT_SELECT} && export APPLICATION_VERSION=${APPLICATION_VERSION} && export APT_INSTALL=${APT_INSTALL} && export QMAKE_FLAGS_QT4=${QMAKE_FLAGS_QT4} && export QMAKE_FLAGS_QT5=${QMAKE_FLAGS_QT5} && export PACKAGE_VERSION=${PACKAGE_VERSION} && export PACKAGE_BUILD=${PACKAGE_BUILD} && export EXECUTABLE_VERSION=${EXECUTABLE_VERSION} && export EXECUTABLE_ARCH=${EXECUTABLE_ARCH} && cd ${PROJECT_DIR_DOCKER} && .travis/debian_install.sh && .travis/debian_build.sh"
