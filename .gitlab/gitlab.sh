#!/bin/bash

# Install curl, git, lua, nsis and openssl
apt-get update -qq && \
apt-get install -qq curl git lua5.2 nsis openssl

# Decrypt Telemetry Authenticator
rm -rf tmext/TelemetryClassAuthenticator.cpp && \
openssl aes-256-cbc -k $tca_pass -in .gitlab/TelemetryClassAuthenticator.cpp.enc -out tmext/TelemetryClassAuthenticator.cpp -d

# Check if build is not tagged
if [ "${CI_COMMIT_TAG}" == "" ]; then
	export EXECUTABLE_TAG=-$(git rev-parse --short HEAD)
else
	export EXECUTABLE_TAG=
fi

# Check if package code is not set
if [ "${PACKAGE_CODE}" == "" ]; then
	export PACKAGE_CODE=GitLab
fi

# Init Application Commit Hash
export APPLICATION_COMMIT=$(git rev-parse --short HEAD)

# Start CI script and copying assets into base directory
.ci/ci.sh && \
cp -Rf assets/* ./
