/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/

#include "SnapmaticPicture.h"
#include "PictureDialog.h"
#include <QApplication>
#include <QStringList>
#include <QString>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList applicationArgs = a.arguments();
    QString selectedAction;
    QString arg1;

    foreach(QString currentArg, applicationArgs)
    {
        QString reworkedArg;
        if (currentArg.left(9) == "-showpic=" && selectedAction == "")
        {
            reworkedArg = currentArg.remove(0,9);
            arg1 = reworkedArg;
            selectedAction = "showpic";
        }
    }

    if (selectedAction == "showpic")
    {
        SnapmaticPicture picture;
        qDebug() << picture.readingPictureFromFile(arg1);
        qDebug() << picture.getLastStep();
        PictureDialog picDialog;
        picDialog.setWindowTitle(picture.getPictureStr());
        picDialog.setSnapmaticPicture(picture.getPixmap());
        picDialog.show();

        return a.exec();

        qDebug() << "showpic runned";
    }

    return a.exec();
}
