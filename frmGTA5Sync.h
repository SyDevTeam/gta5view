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

#ifndef FRMGTA5SYNC_H
#define FRMGTA5SYNC_H

#include <QMainWindow>

namespace Ui {
class frmGTA5Sync;
}

class frmGTA5Sync : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmGTA5Sync(QWidget *parent = 0);
    ~frmGTA5Sync();

private:
    QString GTAV_Folder;
    QString GTAV_ProfilesFolder;
    Ui::frmGTA5Sync *ui;
};

#endif // FRMGTA5SYNC_H
