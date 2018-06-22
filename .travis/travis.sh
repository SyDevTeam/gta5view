#!/bin/bash

# Install lua
if [ "${TRAVIS_OS_NAME}" == "linux" ]; then
	sudo apt-get update -qq && \
	sudo apt-get install -qq curl lua5.2
elif [ "${TRAVIS_OS_NAME}" == "osx" ]; then
	brew install lua
fi

# Check if build is not tagged
if [ "${TRAVIS_TAG}" == "" ]; then
	export EXECUTABLE_TAG=-$(git rev-parse --short HEAD)
else
	export EXECUTABLE_TAG=
fi

# Check if package code is not set
if [ "${PACKAGE_CODE}" == "" ]; then
	export PACKAGE_CODE=GitHub
fi

.ci/ci.sh
