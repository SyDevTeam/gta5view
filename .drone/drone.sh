#!/bin/bash

# Cleanup previous Drone build
if [ -d "assets" ]; then
	rm -rf assets
fi
if [ -d "build" ]; then
	rm -rf build
fi

# Decrypt Telemetry Authenticator
rm -rf tmext/TelemetryClassAuthenticator.cpp && \
openssl aes-256-cbc -k ${TCA_PASS} -in .drone/TelemetryClassAuthenticator.cpp.enc -out tmext/TelemetryClassAuthenticator.cpp -d -pbkdf2

# Check if build is not tagged
if [ "${DRONE_TAG}" == "" ]; then
	export EXECUTABLE_TAG=-$(git rev-parse --short HEAD)
else
	export EXECUTABLE_TAG=
fi

# Check if package code is not set
if [ "${PACKAGE_CODE}" == "" ]; then
	export PACKAGE_CODE=Drone
fi

# Init Application Commit Hash
export APPLICATION_COMMIT=$(git rev-parse --short HEAD)

# Start CI script and copying assets into gta5view directory
.ci/ci.sh && \
mkdir -p /srv/gta5view/${APPLICATION_COMMIT} && \
cp -Rf assets/* /srv/gta5view/${APPLICATION_COMMIT}/
