/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "SnapmaticPicture.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "SavegameDialog.h"
#include "PictureDialog.h"
#include "CrewDatabase.h"
#include "SavegameData.h"
#include <QApplication>
#include <QStringList>
#include <QObject>
#include <QString>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("gta5sync");
    a.setApplicationVersion("1.0.0");

    QStringList applicationArgs = a.arguments();
    QString selectedAction;
    QString arg1;
    applicationArgs.removeAt(0);

    foreach(QString currentArg, applicationArgs)
    {
        QString reworkedArg;
        if (currentArg.left(9) == "-showpic=" && selectedAction == "")
        {
            reworkedArg = currentArg.remove(0,9);
            arg1 = reworkedArg;
            selectedAction = "showpic";
        }
        else if (currentArg.left(9) == "-showsgd=" && selectedAction == "")
        {
            reworkedArg = currentArg.remove(0,9);
            arg1 = reworkedArg;
            selectedAction = "showsgd";
        }
        else if (selectedAction == "")
        {
            QFile argumentFile(currentArg);
            if (argumentFile.exists())
            {
                arg1 = currentArg;
                selectedAction = "showpic";
            }
        }
    }

    if (selectedAction == "showpic")
    {
        CrewDatabase *crewDB = new CrewDatabase();
        ProfileDatabase *profileDB = new ProfileDatabase();
        DatabaseThread *threadDB = new DatabaseThread(crewDB);
        PictureDialog *picDialog = new PictureDialog(profileDB);
        SnapmaticPicture picture;

        bool readOk = picture.readingPictureFromFile(arg1);
        picDialog->setWindowFlags(picDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
        picDialog->setSnapmaticPicture(&picture, readOk);

        int crewID = picture.getCrewNumber();
        if (crewID != 0) { crewDB->addCrew(crewID); }
        if (!readOk) { return 1; }

        QObject::connect(threadDB, SIGNAL(playerNameFound(int, QString)), profileDB, SLOT(setPlayerName(int, QString)));
        QObject::connect(threadDB, SIGNAL(playerNameUpdated()), picDialog, SLOT(on_playerNameUpdated()));
        threadDB->start();

        picDialog->show();

        return a.exec();
    }
    else if (selectedAction == "showsgd")
    {
        SavegameDialog *savegameDialog = new SavegameDialog();
        SavegameData savegame;

        bool readOk = savegame.readingSavegameFromFile(arg1);
        savegameDialog->setWindowFlags(savegameDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
        savegameDialog->setSavegameData(&savegame, readOk);

        if (!readOk) { return 1; }

        savegameDialog->show();

        return a.exec();
    }

    return 0;
}
