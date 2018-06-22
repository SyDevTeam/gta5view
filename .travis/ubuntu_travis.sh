#!/bin/bash

# Install packages
sudo .ci/debian_install.sh && \

# Build gta5view
sudo .ci/debian_build.sh && \
cd ${PROJECT_DIR}
