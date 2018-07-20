/*****************************************************************************
* ImageCropper Qt Widget for cropping images
* Copyright (C) 2013 Dimka Novikov, to@dimkanovikov.pro
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 3 of the License, or any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef IMAGECROPPER_P_H
#define IMAGECROPPER_P_H

#include "imagecropper_e.h"

#include <QtCore/QRect>
#include <QtGui/QPixmap>
#include <QtGui/QColor>

namespace {
	const QRect INIT_CROPPING_RECT = QRect();
	const QSizeF INIT_PROPORTION = QSizeF(1.0, 1.0);
}

class ImageCropperPrivate {
public:
	ImageCropperPrivate() :
		imageForCropping(QPixmap()),
		croppingRect(INIT_CROPPING_RECT),
		lastStaticCroppingRect(QRect()),
		cursorPosition(CursorPositionUndefined),
		isMousePressed(false),
		isProportionFixed(false),
		startMousePos(QPoint()),
		proportion(INIT_PROPORTION),
        deltas(INIT_PROPORTION),
		backgroundColor(Qt::black),
		croppingRectBorderColor(Qt::white)
	{}

public:
	// Изображение для обрезки
	QPixmap imageForCropping;
	// Область обрезки
	QRectF croppingRect;
	// Последняя фиксированная область обрезки
	QRectF lastStaticCroppingRect;
	// Позиция курсора относительно области обрезки
	CursorPosition cursorPosition;
	// Зажата ли левая кнопка мыши
	bool isMousePressed;
	// Фиксировать пропорции области обрезки
	bool isProportionFixed;
	// Начальная позиция курсора при изменении размера области обрезки
	QPointF startMousePos;
    // Пропорции
	QSizeF proportion;
    // Приращения
    // width  - приращение по x
    // height - приращение по y
    QSizeF deltas;
	// Цвет заливки фона под изображением
	QColor backgroundColor;
	// Цвет рамки области обрезки
	QColor croppingRectBorderColor;
};

#endif // IMAGECROPPER_P_H
