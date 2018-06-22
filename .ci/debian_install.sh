#!/bin/bash

# Install packages
apt-get update -qq && \
apt-get install -qq ${APT_INSTALL} checkinstall dpkg-dev fakeroot g++ gcc qtbase5-dev qt5-qmake qttranslations5-l10n libqt4-dev
