#!/bin/bash

# Install lua
if [ "${TRAVIS_OS_NAME}" == "osx" ]; then
	brew install lua
else
	apt-get update -qq && \
	apt-get install -qq lua5.2
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

# Init Application Commit Hash
export APPLICATION_COMMIT=$(git rev-parse --short HEAD)

# Start CI script
.ci/ci.sh
