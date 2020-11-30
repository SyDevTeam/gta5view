/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2020 Syping
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

#include "PictureDialog.h"
#include "PictureWidget.h"
#include "UiModLabel.h"
#include "AppEnv.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPixmap>
#include <QEvent>

PictureWidget::PictureWidget(QWidget *parent) : QDialog(parent)
{
    installEventFilter(this);

    widgetLayout = new QHBoxLayout(this);
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);

    pictureLabel = new UiModLabel(this);
    pictureLabel->setObjectName("pictureLabel");
    pictureLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pictureLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    pictureLabel->setAlignment(Qt::AlignCenter);
    widgetLayout->addWidget(pictureLabel);

    QObject::connect(pictureLabel, SIGNAL(mouseDoubleClicked(Qt::MouseButton)), this, SLOT(pictureDoubleClicked(Qt::MouseButton)));
    QObject::connect(pictureLabel, SIGNAL(customContextMenuRequested(QPoint)), parent, SLOT(exportCustomContextMenuRequested(QPoint)));

    setLayout(widgetLayout);
}

PictureWidget::~PictureWidget()
{
    widgetLayout->removeWidget(pictureLabel);
    delete pictureLabel;
    delete widgetLayout;
}

bool PictureWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == this) {
        if (ev->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent*)ev;
            switch (keyEvent->key()) {
            case Qt::Key_Left:
                emit previousPictureRequested();
                break;
            case Qt::Key_Right:
                emit nextPictureRequested();
                break;
            }
        }
    }
    return false;
}

void PictureWidget::pictureDoubleClicked(Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        close();
    }
}

void PictureWidget::setImage(QImage image_, QRect rec)
{
    const qreal screenRatioPR = AppEnv::screenRatioPR();
    image = image_;
    QPixmap pixmap = QPixmap::fromImage(image.scaled(rec.width() * screenRatioPR, rec.height() * screenRatioPR, Qt::KeepAspectRatio, Qt::SmoothTransformation));
#if QT_VERSION >= 0x050600
    pixmap.setDevicePixelRatio(AppEnv::screenRatioPR());
#endif
    pictureLabel->setPixmap(pixmap);
}

void PictureWidget::setImage(QImage image_)
{
    const qreal screenRatioPR = AppEnv::screenRatioPR();
    image = image_;
    QPixmap pixmap = QPixmap::fromImage(image.scaled(geometry().width() * screenRatioPR, geometry().height() * screenRatioPR, Qt::KeepAspectRatio, Qt::SmoothTransformation));
#if QT_VERSION >= 0x050600
    pixmap.setDevicePixelRatio(screenRatioPR);
#endif
    pictureLabel->setPixmap(pixmap);
}
