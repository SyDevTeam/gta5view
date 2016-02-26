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

#include "frmGTA5Sync.h"
#include "ui_frmGTA5Sync.h"

frmGTA5Sync::frmGTA5Sync(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmGTA5Sync)
{
    ui->setupUi(this);
}

frmGTA5Sync::~frmGTA5Sync()
{
    delete ui;
}
