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

#include "UiModWidget.h"
#include <QStyleOption>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QDebug>
#include <QUrl>

UiModWidget::UiModWidget(QWidget *parent) : QWidget(parent)
{
    filesMode = false;
}

UiModWidget::~UiModWidget()
{
}

void UiModWidget::setFilesMode(bool filesModeEnabled)
{
    filesMode = filesModeEnabled;
}

void UiModWidget::dragEnterEvent(QDragEnterEvent *dragEnterEvent)
{
    if (filesMode && dragEnterEvent->mimeData()->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = dragEnterEvent->mimeData()->urls();

        foreach(const QUrl &currentUrl, urlList)
        {
            if (currentUrl.isLocalFile())
            {
                pathList.append(currentUrl.toLocalFile());
            }
        }

        if (!pathList.isEmpty())
        {
            dragEnterEvent->acceptProposedAction();
        }
    }
}

void UiModWidget::dropEvent(QDropEvent *dropEvent)
{
    dropEvent->acceptProposedAction();
    emit dropped(dropEvent->mimeData());
}

void UiModWidget::paintEvent(QPaintEvent *paintEvent)
{
    Q_UNUSED(paintEvent)
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
