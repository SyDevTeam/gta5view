#!/bin/bash

# Install packages
.ci/osx_install.sh && \

# Build gta5view
.ci/osx_build.sh && \
cd ${PROJECT_DIR}
