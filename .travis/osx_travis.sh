#!/bin/bash

# Install packages
.travis/osx_install.sh && \

# Build gta5view
.travis/osx_build.sh && \
cd ${PROJECT_DIR}
