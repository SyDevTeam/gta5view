/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping
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
#include <QDesktopWidget>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPixmap>
#include <QEvent>
#include <QDebug>

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
    QObject::connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(updateWindowSize(int)));

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
    if (obj == this)
    {
        if (ev->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = (QKeyEvent*)ev;
            switch (keyEvent->key()){
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
    if (button == Qt::LeftButton)
    {
        close();
    }
}

void PictureWidget::setImage(QImage image_, QRect rec)
{
    image = image_;
    pictureLabel->setPixmap(QPixmap::fromImage(image.scaled(rec.width(), rec.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void PictureWidget::setImage(QImage image_)
{
    image = image_;
    pictureLabel->setPixmap(QPixmap::fromImage(image.scaled(geometry().width(), geometry().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void PictureWidget::updateWindowSize(int screenID)
{
    if (screenID == QApplication::desktop()->screenNumber(this))
    {
        QRect desktopRect = QApplication::desktop()->screenGeometry(this);
        this->move(desktopRect.x(), desktopRect.y());
        this->resize(desktopRect.width(), desktopRect.height());
        this->showFullScreen();
        pictureLabel->setPixmap(QPixmap::fromImage(image.scaled(desktopRect.width(), desktopRect.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}
