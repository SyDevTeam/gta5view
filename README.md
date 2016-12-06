## gta5view
Grand Theft Auto V Savegame and Snapmatic viewer

- Viewing Snapmatics and giving the ability to disable them in-game
- Import/Export Snapmatics and Savegames
- Choosing between multiple Social Club accounts as GTA V profiles IDs

#### Screenshots
<img src="https://i.imgur.com/Mi3n6IL.png"/>
<img src="https://i.imgur.com/Sg75ksS.png"/>

#### Build gta5view Debian/Ubuntu

	apt-get install git gcc g++ qtbase5-dev qt5-qmake make checkinstall
	git clone https://github.com/SyDevTeam/gta5view
	mkdir build && cd build
	qmake -qt=5 ../gta5view.pro # or just qmake ../gta5view.pro
	make
	INSTALL_ROOT=/usr checkinstall

#### Build gta5view Windows

Downloading <a href="https://www.qt.io/">Qt Framework</a> and install it.<br>
Take the Online Installer and choose the MinGW version or install Microsoft Visual Studio 2013/2015 Community<br>
Downloading Source Code over GitHub or with your Git client.<br>
Open the gta5view.pro file with Qt Creator and build it over Qt Creator.<br>
