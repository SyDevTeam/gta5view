#!/bin/bash

rm -rf tmext/TelemetryClassAuthenticator.cpp && \
openssl aes-256-cbc -K $encrypted_db000a5d87d6_key -iv $encrypted_db000a5d87d6_iv -in .travis/TelemetryClassAuthenticator.cpp.enc -out tmext/TelemetryClassAuthenticator.cpp -d && \
openssl aes-256-cbc -K $encrypted_d57e7d2f8877_key -iv $encrypted_d57e7d2f8877_iv -in .travis/dropbox_uploader.enc -out ~/.dropbox_uploader -d
