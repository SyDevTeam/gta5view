## gta5view
Grand Theft Auto V Savegame and Snapmatic viewer/editor

- View Snapmatics with the ability to disable them in-game
- Edit Snapmatic pictures and properties in multiple ways
- Import/Export Snapmatics, Savegames and pictures
- Let choose between multiple Social Club accounts as GTA V profiles IDs

#### Screenshots
![Snapmatic Picture Viewer](res/src/picture.png)  
![User Interface](res/src/mainui.png)  
![Snapmatic Properties](res/src/prop.png)

#### Build gta5view for Windows

    # Note: Install Docker Community Edition and Git before continuing
    git clone https://gitlab.com/Syping/gta5view && cd gta5view
    docker pull sypingauto/gta5view-build:1.8-static
    docker run --rm -v ${PWD}:/gta5view -it sypingauto/gta5view-build:1.8-static
    cd /gta5view && mkdir -p build && cd build
    qmake-static ../gta5view.pro
    make depend
    make -j $(nproc --all)

#### Build gta5view for Debian/Ubuntu

    sudo apt-get install git gcc g++ libqt5svg5-dev qtbase5-dev qttranslations5-l10n qt5-qmake make
    git clone https://gitlab.com/Syping/gta5view && cd gta5view
    mkdir -p build && cd build
    ../configure --prefix=/opt/gta5view
    make depend
    make -j $(nproc --all)
    sudo make install

#### Build gta5view for Fedora

    sudo dnf install git gcc gcc-c++ qt5-qtbase-devel qt5-qtsvg-devel qt5-qttranslations make
    git clone https://gitlab.com/Syping/gta5view && cd gta5view
    mkdir -p build && cd build
    ../configure --prefix=/opt/gta5view
    make depend
    make -j $(nproc --all)
    sudo make install

#### Build gta5view for Windows (Beginner)

Download the [Qt Framework](https://www.qt.io/) and install the MinGW version.  
Download the Source Code over the Repository or with your Git client.  
Open the gta5view.pro Project file with Qt Creator and build it over Qt Creator.

#### Download Binary Releases

Go to [gta5view release](https://github.com/SyDevTeam/gta5view/releases) and download the .exe file for Windows, .deb file for Debian/Ubuntu and .dmg file for OS X.
