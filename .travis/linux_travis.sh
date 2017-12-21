#!/bin/bash

# Install packages
sudo apt-get update -qq && \
sudo apt-get install -qq checkinstall dpkg-dev g++ gcc qtbase5-dev qt5-qmake qttranslations5-l10n libqt4-dev && \

.travis/linux_build.sh && \
cd $PROJECT_DIR
