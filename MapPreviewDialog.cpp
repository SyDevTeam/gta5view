/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2017 Syping
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

#include "MapPreviewDialog.h"
#include "ui_MapPreviewDialog.h"
#include "IconLoader.h"
#include "AppEnv.h"
#include <QPainter>
#include <QDebug>

MapPreviewDialog::MapPreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapPreviewDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    setMinimumSize(500 * screenRatio, 600 * screenRatio);
    setMaximumSize(500 * screenRatio, 600 * screenRatio);
    setFixedSize(500 * screenRatio, 600 * screenRatio);
}

MapPreviewDialog::~MapPreviewDialog()
{
    delete ui;
}

void MapPreviewDialog::drawPointOnMap(double xpos_d, double ypos_d)
{
    qreal screenRatio = AppEnv::screenRatio();
    int pointMakerSize = 8 * screenRatio;
    QPixmap pointMakerPixmap = IconLoader::loadingPointmakerIcon().pixmap(QSize(pointMakerSize, pointMakerSize));
    QSize mapPixelSize = size();

    int pointMakerHalfSize = pointMakerSize / 2;
    long xpos_ms = qRound(xpos_d);
    long ypos_ms = qRound(ypos_d);
    double xpos_ma = xpos_ms + 4000;
    double ypos_ma = ypos_ms + 4000;
    double xrat = (double)mapPixelSize.width() / 10000;
    double yrat = (double)mapPixelSize.height() / 12000;
    long xpos_mp =  qRound(xpos_ma * xrat);
    long ypos_mp =  qRound(ypos_ma * yrat);
    long xpos_pr = xpos_mp - pointMakerHalfSize;
    long ypos_pr = ypos_mp + pointMakerHalfSize;

    QPixmap mapPixmap(mapPixelSize);
    QPainter mapPainter(&mapPixmap);
    mapPainter.drawPixmap(0, 0, mapPixelSize.width(), mapPixelSize.height(), QPixmap(":/img/mappreview.jpg").scaled(mapPixelSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    mapPainter.drawPixmap(xpos_pr, mapPixelSize.height() - ypos_pr, pointMakerSize, pointMakerSize, pointMakerPixmap);
    mapPainter.end();

    ui->labPicture->setPixmap(mapPixmap);
}
