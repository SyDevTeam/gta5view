## gta5view
Grand Theft Auto V Savegame and Snapmatic viewer/editor

- View Snapmatics with the ability to disable them in-game
- Edit Snapmatic pictures and properties in multiple ways
- Import/Export Snapmatics, Savegames and pictures
- Let choose between multiple Social Club accounts as GTA V profiles IDs

#### Screenshots
<img src="https://i.imgur.com/cOcojyq.png"/>
<img src="https://i.imgur.com/LGmxdgU.png"/>
<img src="https://i.imgur.com/j1Lodiu.png"/>

#### Build gta5view Debian/Ubuntu

	sudo apt-get install git gcc g++ qtbase5-dev qttranslations5-l10n qt5-qmake make checkinstall
	git clone https://github.com/SyDevTeam/gta5view
	mkdir gta5view.build && cd gta5view.build
	qmake -qt=5 GTA5SYNC_PREFIX=/usr ../gta5view/gta5view.pro # or just qmake GTA5SYNC_PREFIX=/usr ../gta5view/gta5view.pro
	make
	sudo checkinstall --pkgname=gta5view --pkggroup=utility --requires=libqt5core5a,libqt5gui5,libqt5network5,libqt5widgets5,qttranslations5-l10n

#### Build gta5view Windows (Beginner)

Download <a href="https://www.qt.io/">Qt Framework</a> and install it.<br>
Take the Online Installer and choose the MinGW version or install Microsoft Visual Studio 2013/2015 Community<br>
Download the Source Code over GitHub or with your Git client.<br>
Open the gta5view.pro Project file with Qt Creator and build it over Qt Creator.<br>

#### Download Binary Releases

Go to <a href="https://github.com/SyDevTeam/gta5view/releases">gta5view release</a> and download the .exe file for Windows, .deb file for Debian/Ubuntu and .dmg file for OS X
