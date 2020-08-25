/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2017-2020 Syping
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

#include "MapLocationDialog.h"
#include "ui_MapLocationDialog.h"
#include "IconLoader.h"
#include "AppEnv.h"
#include <QPainter>
#include <QDebug>
#include <QStyle>

MapLocationDialog::MapLocationDialog(double x, double y, QWidget *parent) :
    QDialog(parent), xpos_old(x), ypos_old(y),
    ui(new Ui::MapLocationDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
    ui->cmdDone->setVisible(false);
    ui->cmdApply->setVisible(false);
    ui->cmdRevert->setVisible(false);
    ui->cmdDone->setCursor(Qt::ArrowCursor);
    ui->cmdClose->setCursor(Qt::ArrowCursor);

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    int widgetMargin = qRound(3 * screenRatio);
    ui->hlMapDialog->setContentsMargins(widgetMargin, widgetMargin, widgetMargin, widgetMargin);
    ui->vlMapDialog->setSpacing(widgetMargin);
    setMinimumSize(500 * screenRatio, 600 * screenRatio);
    setMaximumSize(500 * screenRatio, 600 * screenRatio);
    setFixedSize(500 * screenRatio, 600 * screenRatio);
    setMouseTracking(true);

    changeMode = false;
    propUpdate = false;
    drawPointOnMap(xpos_old, ypos_old);
}

MapLocationDialog::~MapLocationDialog()
{
    delete ui;
}

void MapLocationDialog::drawPointOnMap(double xpos_d, double ypos_d)
{
    ui->labPos->setText(tr("X: %1\nY: %2", "X and Y position").arg(QString::number(xpos_d), QString::number(ypos_d)));
    xpos_new = xpos_d;
    ypos_new = ypos_d;
    repaint();
}

void MapLocationDialog::on_cmdChange_clicked()
{
    qreal screenRatio = AppEnv::screenRatio();
    int pointMakerSize = 8 * screenRatio;
    QPixmap pointMakerPixmap = IconLoader::loadingPointmakerIcon().pixmap(QSize(pointMakerSize, pointMakerSize));
    QCursor pointMakerCursor(pointMakerPixmap);
    ui->cmdDone->setVisible(true);
    ui->cmdApply->setVisible(false);
    ui->cmdChange->setVisible(false);
    ui->cmdRevert->setVisible(false);

    setCursor(pointMakerCursor);
    changeMode = true;
}

void MapLocationDialog::on_cmdDone_clicked()
{
    ui->cmdDone->setVisible(false);
    ui->cmdChange->setVisible(true);
    if (xpos_new != xpos_old || ypos_new != ypos_old)
    {
        ui->cmdApply->setVisible(true);
        ui->cmdRevert->setVisible(true);
    }

    setCursor(Qt::ArrowCursor);
    changeMode = false;
}

void MapLocationDialog::updatePosFromEvent(int x, int y)
{
    QSize mapPixelSize = size();
    int xpos_ad = x;
    int ypos_ad = mapPixelSize.height() - y;
    double xrat = 10000 / (double)mapPixelSize.width();
    double yrat = 12000 / (double)mapPixelSize.height();
    double xpos_rv = xrat * xpos_ad;
    double ypos_rv = yrat * ypos_ad;
    double xpos_fp = xpos_rv - 4000;
    double ypos_fp = ypos_rv - 4000;
    drawPointOnMap(xpos_fp, ypos_fp);
}

void MapLocationDialog::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    qreal screenRatio = AppEnv::screenRatio();
    qreal screenRatioPR = AppEnv::screenRatioPR();

    // Paint Map
    QSize mapPixelSize = QSize(width() * screenRatioPR, height() * screenRatioPR);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/img/mappreview.jpg").scaled(mapPixelSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    // Paint Marker
    int pointMarkerSize = 8 * screenRatio;
    int pointMarkerHalfSize = pointMarkerSize / 2;
    long xpos_ms = qRound(xpos_new);
    long ypos_ms = qRound(ypos_new);
    double xpos_ma = xpos_ms + 4000;
    double ypos_ma = ypos_ms + 4000;
    double xrat = (double)width() / 10000;
    double yrat = (double)height() / 12000;
    long xpos_mp = qRound(xpos_ma * xrat);
    long ypos_mp = qRound(ypos_ma * yrat);
    long xpos_pr;
    long ypos_pr;
    if (screenRatioPR != 1) {
#ifdef GTA5SYNC_WIN
        xpos_pr = xpos_mp - pointMarkerHalfSize;
        ypos_pr = ypos_mp + pointMarkerHalfSize;
#else
        xpos_pr = xpos_mp - pointMarkerHalfSize + screenRatioPR;
        ypos_pr = ypos_mp + pointMarkerHalfSize - screenRatioPR;
#endif
    }
    else {
        xpos_pr = xpos_mp - pointMarkerHalfSize;
        ypos_pr = ypos_mp + pointMarkerHalfSize;
    }
    QPixmap mapMarkerPixmap = IconLoader::loadingPointmakerIcon().pixmap(QSize(pointMarkerSize, pointMarkerSize));
    painter.drawPixmap(xpos_pr, height() - ypos_pr, pointMarkerSize, pointMarkerSize, mapMarkerPixmap);

    QDialog::paintEvent(ev);
}

void MapLocationDialog::mouseMoveEvent(QMouseEvent *ev)
{
    if (!changeMode) { ev->ignore(); }
    else if (ev->buttons() & Qt::LeftButton)
    {
        updatePosFromEvent(ev->x(), ev->y());
        ev->accept();
    }
    else
    {
        ev->ignore();
    }
}

void MapLocationDialog::mouseReleaseEvent(QMouseEvent *ev)
{
    if (!changeMode) { ev->ignore(); }
    else if (ev->button() == Qt::LeftButton)
    {
        updatePosFromEvent(ev->x(), ev->y());
        ev->accept();
    }
    else
    {
        ev->ignore();
    }
}

void MapLocationDialog::on_cmdApply_clicked()
{
    propUpdate = true;
    xpos_old = xpos_new;
    ypos_old = ypos_new;
    ui->cmdApply->setVisible(false);
    ui->cmdRevert->setVisible(false);
}

void MapLocationDialog::on_cmdRevert_clicked()
{
    drawPointOnMap(xpos_old, ypos_old);
    ui->cmdApply->setVisible(false);
    ui->cmdRevert->setVisible(false);
}

bool MapLocationDialog::propUpdated()
{
    return propUpdate;
}

double MapLocationDialog::getXpos()
{
    return xpos_old;
}

double MapLocationDialog::getYpos()
{
    return ypos_old;
}

void MapLocationDialog::on_cmdClose_clicked()
{
    close();
}
