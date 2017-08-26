## gta5view
Grand Theft Auto V Savegame and Snapmatic viewer

- Viewing Snapmatics and giving the ability to disable them in-game
- Import/Export Snapmatics and Savegames
- Choosing between multiple Social Club accounts as GTA V profiles IDs

#### Screenshots
<img src="https://i.imgur.com/ncMtWjR.png"/>
<img src="https://i.imgur.com/4THtwb1.png"/>
<img src="https://i.imgur.com/STkRl6j.png"/>

#### Build gta5view Debian/Ubuntu

	apt-get install git gcc g++ qtbase5-dev qt5-qmake make checkinstall
	git clone https://github.com/SyDevTeam/gta5view
	mkdir build && cd build
	qmake -qt=5 ../gta5view.pro # or just qmake ../gta5view.pro
	make
	INSTALL_ROOT=/usr checkinstall --pkgname=gta5view --pkggroup=utility --requires=libqt5core5a,libqt5gui5,libqt5network5,libqt5widgets5

#### Build gta5view Windows

Downloading <a href="https://www.qt.io/">Qt Framework</a> and install it.<br>
Take the Online Installer and choose the MinGW version or install Microsoft Visual Studio 2013/2015 Community<br>
Downloading Source Code over GitHub or with your Git client.<br>
Open the gta5view.pro file with Qt Creator and build it over Qt Creator.<br>

#### Download Binary Releases

Go to <a href="https://github.com/SyDevTeam/gta5view/releases">gta5view release</a> and download the .exe file for Windows, .deb file for Debian/Ubuntu and .dmg file for OS X
