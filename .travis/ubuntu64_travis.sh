#!/bin/bash

# Install packages
sudo .travis/debian_install.sh && \

# Build gta5view
sudo .travis/debian_build.sh && \
cd ${PROJECT_DIR}
