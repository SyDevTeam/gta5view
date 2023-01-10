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

#ifndef IMAGECROPPER_H
#define IMAGECROPPER_H

#include "imagecropper_p.h"
#include "imagecropper_e.h"

#include <QWidget>

class ImageCropper : public QWidget
{
    Q_OBJECT

public:
    ImageCropper(QWidget *parent = 0);
    ~ImageCropper();

public slots:
	// Установить изображение для обрезки
	void setImage(const QPixmap& _image);
	// Установить цвет фона виджета обрезки
	void setBackgroundColor(const QColor& _backgroundColor);
	// Установить цвет рамки области обрезки
	void setCroppingRectBorderColor(const QColor& _borderColor);
	// Установить пропорции области выделения
	void setProportion(const QSizeF& _proportion);
	// Использовать фиксированные пропорции области виделения
	void setProportionFixed(const bool _isFixed);

public:
	// Обрезать изображение
	const QPixmap cropImage();

protected:
	virtual void paintEvent(QPaintEvent* _event);
	virtual void mousePressEvent(QMouseEvent* _event);
	virtual void mouseMoveEvent(QMouseEvent* _event);
	virtual void mouseReleaseEvent(QMouseEvent* _event);

private:
	// Определение местоположения курсора над виджетом
	CursorPosition cursorPosition(const QRectF& _cropRect, const QPointF& _mousePosition);
	// Обновить иконку курсора соответствующую местоположению мыши
	void updateCursorIcon(const QPointF& _mousePosition);

	// Получить размер виджета после его изменения мышью
	// --------
    // Контракты:
    // 1. Метод должен вызываться, только при зажатой кнопке мыши
    //    (т.е. при перемещении или изменении размера виджета)
	// --------
	// В случае неудачи возвращает null-прямоугольник
	const QRectF calculateGeometry(
			const QRectF& _sourceGeometry,
			const CursorPosition _cursorPosition,
			const QPointF& _mouseDelta
			);
    // Получить размер виджета после его изменения мышью
    // Метод изменяет виджет не сохраняя начальных пропорций сторон
    // ------
    // Контракты:
    // 1. Метод должен вызываться, только при зажатой кнопке мыши
    //    (т.е. при перемещении или изменении размера виджета)
	const QRectF calculateGeometryWithCustomProportions(
			const QRectF& _sourceGeometry,
			const CursorPosition _cursorPosition,
			const QPointF& _mouseDelta
			);
    // Получить размер виджета после его изменения мышью
    // Метод изменяет виджет сохраняя начальные пропорции сторон
    // ------
    // Контракты:
    // 1. Метод должен вызываться, только при зажатой кнопке мыши
    //    (т.е. при перемещении или изменении размера виджета)
	const QRectF calculateGeometryWithFixedProportions(const QRectF &_sourceGeometry,
												 const CursorPosition _cursorPosition,
												 const QPointF &_mouseDelta,
												 const QSizeF &_deltas
                                                 );

private:
	// Private data implementation
	ImageCropperPrivate* pimpl;
};

#endif // IMAGECROPPER_H
