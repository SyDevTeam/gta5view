#!/bin/bash

# Install packages
.ci/debian_install.sh && \

# Build gta5view
.ci/debian_build.sh && \
cd ${PROJECT_DIR}
