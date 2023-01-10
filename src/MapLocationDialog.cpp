/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2017-2021 Syping
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
#include <QStringBuilder>
#include <QPainter>
#include <QStyle>

MapLocationDialog::MapLocationDialog(double x, double y, QWidget *parent) :
    QDialog(parent), xpos_old(x), ypos_old(y),
    ui(new Ui::MapLocationDialog)
{
    // Set Window Flags
#if QT_VERSION >= 0x050900
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#else
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
#endif

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

    zoomPercent = 100;
    changeMode = false;
    propUpdate = false;
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

void MapLocationDialog::setCayoPerico(bool isCayoPerico)
{
    qreal screenRatio = AppEnv::screenRatio();
    p_isCayoPerico = isCayoPerico;
    if (isCayoPerico) {
        setMinimumSize(500 * screenRatio, 500 * screenRatio);
        setMaximumSize(500 * screenRatio, 500 * screenRatio);
        ui->hlMapDialog->removeItem(ui->vlMapDialog);
        ui->hlMapDialog->insertLayout(0, ui->vlMapDialog);
        ui->hlMapDialog->removeItem(ui->vlPosLayout);
        ui->hlMapDialog->addLayout(ui->vlPosLayout);
        ui->labPos->setAlignment(Qt::AlignRight);
        mapImage = QImage(AppEnv::getImagesFolder() % "/mapcayoperico.jpg");
    }
    else {
        mapImage = QImage(AppEnv::getImagesFolder() % "/mappreview.jpg");
    }
    drawPointOnMap(xpos_old, ypos_old);
}

void MapLocationDialog::updatePosFromEvent(double x, double y)
{
    QSize mapPixelSize = size();
    double x_per = x / mapPixelSize.width(); // get X %
    double y_per = y / mapPixelSize.height(); // get Y %
    double x_pos, y_pos;
    if (p_isCayoPerico) {
        x_pos = x_per * 2340; // 2340 is 100% for X (Cayo Perico)
        y_pos = y_per * -2340; // -2340 is 100% for Y (Cayo Perico)
        x_pos = x_pos + 3560; // +3560 gets corrected for X (Cayo Perico)
        y_pos = y_pos - 3980; // -3980 gets corrected for Y (Cayo Perico)
    }
    else {
        x_pos = x_per * 10000; // 10000 is 100% for X (Los Santos)
        y_pos = y_per * -12000; // -12000 is 100% for Y (Los Santos)
        x_pos = x_pos - 4000; // -4000 gets corrected for X (Los Santos)
        y_pos = y_pos + 8000; // +8000 gets corrected for Y (Los Santos)
    }
    drawPointOnMap(x_pos, y_pos);
}

void MapLocationDialog::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Screen Ratio
    qreal screenRatio = AppEnv::screenRatio();
    qreal screenRatioPR = AppEnv::screenRatioPR();

    // Paint Map
    const double zoomLevel = static_cast<double>(zoomPercent) / 100;
    const QSize mapImageSize = mapImage.size();
    const QPointF mapImageMid(static_cast<double>(mapImageSize.width()) / 2, static_cast<double>(mapImageSize.height()) / 2);
    const QSizeF srcImageSize(static_cast<double>(mapImageSize.width()) / zoomLevel , static_cast<double>(mapImageSize.height()) / zoomLevel);
    const QPointF mapImageTopLeft(mapImageMid.x() - (srcImageSize.width() / 2), mapImageMid.y() - (srcImageSize.height() / 2));
    const QPointF mapImageBottomRight(mapImageMid.x() + (srcImageSize.width() / 2), mapImageMid.y() + (srcImageSize.height() / 2));
    painter.drawImage(QRect(QPoint(0, 0), size()), mapImage, QRectF(mapImageTopLeft, mapImageBottomRight));

    // Paint Marker
    QSize mapPixelSize = size();
    int pointMarkerSize = 8 * screenRatio;
    int pointMarkerHalfSize = pointMarkerSize / 2;
    double xpos_mp, ypos_mp;
    if (p_isCayoPerico) {
        double xpos_per = xpos_new - 3560; // correct X in reserve
        double ypos_per = ypos_new + 3980; // correct y in reserve
        xpos_per = xpos_per / 2340; // divide 100% for X
        ypos_per = ypos_per / -2340; // divide 100% for Y
        xpos_mp = xpos_per * mapPixelSize.width(); // locate window width pos
        ypos_mp = ypos_per * mapPixelSize.height(); // locate window height pos
    }
    else {
        double xpos_per = xpos_new + 4000; // correct X in reserve
        double ypos_per = ypos_new - 8000; // correct y in reserve
        xpos_per = xpos_per / 10000; // divide 100% for X
        ypos_per = ypos_per / -12000; // divide 100% for Y
        xpos_mp = xpos_per * mapPixelSize.width(); // locate window width pos
        ypos_mp = ypos_per * mapPixelSize.height(); // locate window height pos
    }
    QPointF pointMarkerPos(xpos_mp, ypos_mp);
    if (screenRatioPR != 1) {
        pointMarkerPos.setX(pointMarkerPos.x() - pointMarkerHalfSize + screenRatioPR);
        pointMarkerPos.setY(pointMarkerPos.y() - pointMarkerHalfSize + screenRatioPR);
    }
    else {
        pointMarkerPos.setX(pointMarkerPos.x() - pointMarkerHalfSize);
        pointMarkerPos.setY(pointMarkerPos.y() - pointMarkerHalfSize);
    }
    QPixmap mapMarkerPixmap = IconLoader::loadingPointmakerIcon().pixmap(QSize(pointMarkerSize, pointMarkerSize));
    painter.drawPixmap(pointMarkerPos, mapMarkerPixmap);

    QDialog::paintEvent(ev);
}

void MapLocationDialog::mouseMoveEvent(QMouseEvent *ev)
{
    if (changeMode && ev->buttons() & Qt::LeftButton) {
#if QT_VERSION >= 0x060000
        const QPointF localPos = ev->position();
#elif QT_VERSION >= 0x050000
        const QPointF localPos = ev->localPos();
#else
        const QPointF localPos = ev->posF();
#endif
#ifdef Q_OS_WIN
        qreal screenRatioPR = AppEnv::screenRatioPR();
        if (screenRatioPR != 1) {
            updatePosFromEvent(localPos.x() - screenRatioPR, localPos.y() - screenRatioPR);
        }
        else {
            updatePosFromEvent(localPos.x(), localPos.y());
        }
#else
        updatePosFromEvent(localPos.x(), localPos.y());
#endif
    }
    else if (dragStart && ev->buttons() & Qt::LeftButton) {
#if QT_VERSION >= 0x060000
        const QPointF dragNewPosition = ev->position();
#elif QT_VERSION >= 0x050000
        const QPointF dragNewPosition = ev->localPos();
#else
        const QPointF dragNewPosition = ev->posF();
#endif
        mapDiffPosition = dragNewPosition - dragPosition + mapDiffPosition;
        dragPosition = dragNewPosition;
    }
}

void MapLocationDialog::mousePressEvent(QMouseEvent *ev)
{
    if (!changeMode && ev->button() == Qt::LeftButton) {
#if QT_VERSION >= 0x060000
        dragPosition = ev->position();
#elif QT_VERSION >= 0x050000
        dragPosition = ev->localPos();
#else
        dragPosition = ev->posF();
#endif
        dragStart = true;
    }
}

void MapLocationDialog::mouseReleaseEvent(QMouseEvent *ev)
{
    if (changeMode && ev->button() == Qt::LeftButton) {
#if QT_VERSION >= 0x060000
        const QPointF localPos = ev->position();
#elif QT_VERSION >= 0x050000
        const QPointF localPos = ev->localPos();
#else
        const QPointF localPos = ev->posF();
#endif
#ifdef Q_OS_WIN
        qreal screenRatioPR = AppEnv::screenRatioPR();
        if (screenRatioPR != 1) {
            updatePosFromEvent(localPos.x() - screenRatioPR, localPos.y() - screenRatioPR);
        }
        else {
            updatePosFromEvent(localPos.x(), localPos.y());
        }
#else
        updatePosFromEvent(localPos.x(), localPos.y());
#endif
    }
    else if (dragStart && ev->button() == Qt::LeftButton) {
        dragStart = false;
    }
}

void MapLocationDialog::wheelEvent(QWheelEvent *ev)
{
#ifdef GTA5SYNC_EXPERIMENTAL
#if QT_VERSION >= 0x050000
    const QPoint numPixels = ev->pixelDelta();
    const QPoint numDegrees = ev->angleDelta();
#else
    QPoint numDegrees;
    if (ev->orientation() == Qt::Horizontal) {
        numDegrees.setX(ev->delta());
    }
    else {
        numDegrees.setY(ev->delta());
    }
#endif
#if QT_VERSION >= 0x050000
    if (!numPixels.isNull()) {
        if (numPixels.y() < 0 && zoomPercent != 100) {
            zoomPercent = zoomPercent - 10;
            repaint();
        }
        else if (numPixels.y() > 0 && zoomPercent != 400) {
            zoomPercent = zoomPercent + 10;
            repaint();
        }
        return;
    }
#endif
    if (!numDegrees.isNull()) {
        if (numDegrees.y() < 0 && zoomPercent != 100) {
            zoomPercent = zoomPercent - 10;
            repaint();
        }
        else if (numDegrees.y() > 0 && zoomPercent != 400) {
            zoomPercent = zoomPercent + 10;
            repaint();
        }
    }
#else
    Q_UNUSED(ev)
#endif
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
    if (xpos_new != xpos_old || ypos_new != ypos_old) {
        ui->cmdApply->setVisible(true);
        ui->cmdRevert->setVisible(true);
    }
    setCursor(Qt::ArrowCursor);
    changeMode = false;
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
