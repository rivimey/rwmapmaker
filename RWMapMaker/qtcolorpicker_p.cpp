/****************************************************************************
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of a Qt Solutions component.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
**
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
**
****************************************************************************/
#include "stdafx.h"

#include <QtWidgets/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QLayout>
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolTip>

#include <math.h>

#include "qtcolorpicker_p.h"

/*! \internal

    Constructs the popup widget.
*/
ColorPickerPopup::ColorPickerPopup(int width, bool withColorDialog,
				       QWidget *parent)
    : QFrame(parent, Qt::Popup)
{
    setFrameStyle(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    cols = width;

    if (withColorDialog) {
		moreButton = new ColorPickerButton(this);
		moreButton->setFixedWidth(24);
		moreButton->setFixedHeight(21);
		moreButton->setFrameRect(QRect(2, 2, 20, 17));
		connect(moreButton, SIGNAL(clicked()), SLOT(getColorFromDialog()));
	} else {
		moreButton = 0;
    }

    eventLoop = 0;
    grid = 0;
    regenerateGrid();
}


/*! \internal

    Destructs the popup widget.
*/
ColorPickerPopup::~ColorPickerPopup()
{
    if (eventLoop)
        eventLoop->exit();
}

/*! \internal

    If there is an item whole color is equal to \a col, returns a
    pointer to this item; otherwise returns 0.
*/
ColorPickerItem *ColorPickerPopup::find(const QColor &col) const
{
    for (int i = 0; i < items.size(); ++i) {
		if (items.at(i) && items.at(i)->color() == col)
		    return items.at(i);
    }

    return 0;
}

/*! \internal

    Adds \a item to the grid. The items are added from top-left to
    bottom-right.
*/
void ColorPickerPopup::insertColor(const QColor &col, const QString &text, int index)
{
    // Don't add colors that we have already.
    ColorPickerItem *existingItem = find(col);
    ColorPickerItem *lastSelectedItem = find(lastSelected());

    if (existingItem) {
        if (lastSelectedItem && existingItem != lastSelectedItem)
            lastSelectedItem->setSelected(false);
        existingItem->setFocus();
        existingItem->setSelected(true);
        return;
    }

    ColorPickerItem *item = new ColorPickerItem(col, text, this);

    if (lastSelectedItem) {
        lastSelectedItem->setSelected(false);
    }
    else {
        item->setSelected(true);
        lastSel = col;
    }
    item->setFocus();

    connect(item, SIGNAL(selected()), SLOT(updateSelected()));

    if (index == -1)
	index = items.count();

    items.insert((unsigned int)index, item);
    regenerateGrid();

    update();
}

/*! \internal

*/
QColor ColorPickerPopup::color(int index) const
{
    if (index < 0 || index > (int) items.count() - 1)
        return QColor();

    ColorPickerPopup *that = (ColorPickerPopup *)this;
    return that->items.at(index)->color();
}

/*! \internal

*/
void ColorPickerPopup::exec()
{
    show();

    QEventLoop e;
    eventLoop = &e;
    (void) e.exec();
    eventLoop = 0;
}

/*! \internal

*/
void ColorPickerPopup::updateSelected()
{
    QLayoutItem *layoutItem;
    int i = 0;
    while ((layoutItem = grid->itemAt(i)) != 0) {
	QWidget *w = layoutItem->widget();
	if (w && w->inherits("ColorPickerItem")) {
	    ColorPickerItem *litem = reinterpret_cast<ColorPickerItem *>(layoutItem->widget());
	    if (litem != sender())
		litem->setSelected(false);
	}
	++i;
    }

    if (sender() && sender()->inherits("ColorPickerItem")) {
	ColorPickerItem *item = (ColorPickerItem *)sender();
	lastSel = item->color();
	emit selected(item->color());
    }

    hide();
}

/*! \internal

*/
void ColorPickerPopup::mouseReleaseEvent(QMouseEvent *e)
{
    if (!rect().contains(e->pos()))
	hide();
}

/*! \internal

    Controls keyboard navigation and selection on the color grid.
*/
void ColorPickerPopup::keyPressEvent(QKeyEvent *e)
{
    int curRow = 0;
    int curCol = 0;

    bool foundFocus = false;
    for (int j = 0; !foundFocus && j < grid->rowCount(); ++j) {
	for (int i = 0; !foundFocus && i < grid->columnCount(); ++i) {
	    if (widgetAt[j][i] && widgetAt[j][i]->hasFocus()) {
		curRow = j;
		curCol = i;
		foundFocus = true;
		break;
	    }
	}
    }

    switch (e->key()) {
	case Qt::Key_Left:
	    if (curCol > 0) --curCol;
	    else if (curRow > 0) { --curRow; curCol = grid->columnCount() - 1; }
	    break;
	case Qt::Key_Right:
	    if (curCol < grid->columnCount() - 1 && widgetAt[curRow][curCol + 1]) ++curCol;
	    else if (curRow < grid->rowCount() - 1) { ++curRow; curCol = 0; }
	    break;
	case Qt::Key_Up:
	    if (curRow > 0) --curRow;
	    else curCol = 0;
	    break;
	case Qt::Key_Down:
	    if (curRow < grid->rowCount() - 1) {
		QWidget *w = widgetAt[curRow + 1][curCol];
		if (w) {
		    ++curRow;
		} else for (int i = 1; i < grid->columnCount(); ++i) {
		    if (!widgetAt[curRow + 1][i]) {
			curCol = i - 1;
			++curRow;
			break;
		    }
		}
	    }
	    break;
	case Qt::Key_Space:
	case Qt::Key_Return:
	case Qt::Key_Enter: {
	    QWidget *w = widgetAt[curRow][curCol];
	    if (w && w->inherits("ColorPickerItem")) {
		ColorPickerItem *wi = reinterpret_cast<ColorPickerItem *>(w);
		wi->setSelected(true);

		QLayoutItem *layoutItem;
                int i = 0;
		while ((layoutItem = grid->itemAt(i)) != 0) {
		    QWidget *w = layoutItem->widget();
		    if (w && w->inherits("ColorPickerItem")) {
			ColorPickerItem *litem
			    = reinterpret_cast<ColorPickerItem *>(layoutItem->widget());
			if (litem != wi)
			    litem->setSelected(false);
		    }
		    ++i;
		}

		lastSel = wi->color();
		emit selected(wi->color());
		hide();
	    } else if (w && w->inherits("QPushButton")) {
		ColorPickerItem *wi = reinterpret_cast<ColorPickerItem *>(w);
		wi->setSelected(true);

		QLayoutItem *layoutItem;
                int i = 0;
		while ((layoutItem = grid->itemAt(i)) != 0) {
		    QWidget *w = layoutItem->widget();
		    if (w && w->inherits("ColorPickerItem")) {
			ColorPickerItem *litem
			    = reinterpret_cast<ColorPickerItem *>(layoutItem->widget());
			if (litem != wi)
			    litem->setSelected(false);
		    }
		    ++i;
		}

		lastSel = wi->color();
		emit selected(wi->color());
		hide();
	    }
	}
	break;
        case Qt::Key_Escape:
            hide();
        break;
	default:
	    e->ignore();
	    break;
    }

    widgetAt[curRow][curCol]->setFocus();
}

/*! \internal

*/
void ColorPickerPopup::hideEvent(QHideEvent *e)
{
    if (eventLoop) {
	eventLoop->exit();
    }

    setFocus();

    emit hid();
    QFrame::hideEvent(e);
}

/*! \internal

*/
QColor ColorPickerPopup::lastSelected() const
{
    return lastSel;
}

/*! \internal

    Sets focus on the popup to enable keyboard navigation. Draws
    focusRect and selection rect.
*/
void ColorPickerPopup::showEvent(QShowEvent *)
{
    bool foundSelected = false;
    for (int i = 0; i < grid->columnCount(); ++i) {
	for (int j = 0; j < grid->rowCount(); ++j) {
	    QWidget *w = widgetAt[j][i];
	    if (w && w->inherits("ColorPickerItem")) {
		if (((ColorPickerItem *)w)->isSelected()) {
		    w->setFocus();
		    foundSelected = true;
		    break;
		}
	    }
	}
    }

    if (!foundSelected) {
	if (items.count() == 0)
	    setFocus();
	else
	    widgetAt[0][0]->setFocus();
    }
}

/*!

*/
void ColorPickerPopup::regenerateGrid()
{
	int fixed;

    widgetAt.clear();

	switch(shape)
	{
	case Shape_Auto:
		fixed = cols;
		if (fixed == -1)
			fixed = (int) ceil(sqrt((float) items.count()));
		break;

	case Shape_FixedRows:
	case Shape_FixedCols:
		fixed = rowsOrCols;
		break;
	}

    if (grid)
		delete grid;
    grid = new QGridLayout(this);
    grid->setMargin(1);
    grid->setSpacing(0);

    int ccol = 0, crow = 0;
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i)) {
            widgetAt[crow][ccol] = items.at(i);
            grid->addWidget(items.at(i), crow, ccol);

			switch(shape)
			{
			case Shape_FixedRows:
				crow++;
				if (crow == fixed) {
					++ccol;
					crow = 0;
				}
				break;
			case Shape_Auto:
			case Shape_FixedCols:
				ccol++;
				if (ccol == fixed) {
					++crow;
					ccol = 0;
				}
				break;
			}
        }
    }

    if (moreButton) {
		grid->addWidget(moreButton, crow, ccol);
		widgetAt[crow][ccol] = moreButton;
    }
    updateGeometry();
}

/*! \internal

    Copies the color dialog's currently selected item and emits
    itemSelected().
*/
void ColorPickerPopup::getColorFromDialog()
{
    bool ok;
    QRgb rgb = QColorDialog::getRgba(lastSel.rgba(), &ok, parentWidget());
    if (!ok)
	return;

    QColor col = QColor::fromRgba(rgb);
    insertColor(col, tr("Custom"), -1);
    lastSel = col;
    emit selected(col);
}

/*!
    Constructs a ColorPickerItem whose color is set to \a color, and
    whose name is set to \a text.
*/
ColorPickerItem::ColorPickerItem(const QColor &color, const QString &text,
				     QWidget *parent)
    : QFrame(parent), c(color), t(text), sel(false)
{
    setToolTip(t);
    setFixedWidth(24);
    setFixedHeight(21);
}

/*!
    Destructs a ColorPickerItem.
 */
ColorPickerItem::~ColorPickerItem()
{
}

/*!
    Returns the item's color.

    \sa text()
*/
QColor ColorPickerItem::color() const
{
    return c;
}

/*!
    Returns the item's text.

    \sa color()
*/
QString ColorPickerItem::text() const
{
    return t;
}

/*!

*/
bool ColorPickerItem::isSelected() const
{
    return sel;
}

/*!

*/
void ColorPickerItem::setSelected(bool selected)
{
    sel = selected;
    update();
}

/*!
    Sets the item's color to \a color, and its name to \a text.
*/
void ColorPickerItem::setColor(const QColor &color, const QString &text)
{
    c = color;
    t = text;
    setToolTip(t);
    update();
}

/*!

*/
void ColorPickerItem::mouseMoveEvent(QMouseEvent *)
{
    setFocus();
    update();
}

/*!

*/
void ColorPickerItem::mouseReleaseEvent(QMouseEvent *)
{
    sel = true;
    emit selected();
}

/*!

*/
void ColorPickerItem::mousePressEvent(QMouseEvent *)
{
    setFocus();
    update();
}

/*!

*/
void ColorPickerItem::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int w = width();			// width of cell in pixels
    int h = height();			// height of cell in pixels

    p.setPen( QPen( Qt::gray, 0, Qt::SolidLine ) );

    if (sel)
	p.drawRect(1, 1, w - 3, h - 3);

    p.setPen( QPen( Qt::black, 0, Qt::SolidLine ) );
    p.drawRect(3, 3, w - 7, h - 7);
    p.fillRect(QRect(4, 4, w - 8, h - 8), QBrush(c));

    if (hasFocus())
	p.drawRect(0, 0, w - 1, h - 1);
}

/*!

*/
ColorPickerButton::ColorPickerButton(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(StyledPanel);
}

/*!

*/
void ColorPickerButton::mousePressEvent(QMouseEvent *)
{
    setFrameShadow(Sunken);
    update();
}

/*!

*/
void ColorPickerButton::mouseMoveEvent(QMouseEvent *)
{
    setFocus();
    update();
}

/*!

*/
void ColorPickerButton::mouseReleaseEvent(QMouseEvent *)
{
    setFrameShadow(Raised);
    repaint();
    emit clicked();
}

/*!

*/
void ColorPickerButton::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up
	|| e->key() == Qt::Key_Down
	|| e->key() == Qt::Key_Left
	|| e->key() == Qt::Key_Right) {
	qApp->sendEvent(parent(), e);
    } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
	setFrameShadow(Sunken);
	update();
    } else {
	QFrame::keyPressEvent(e);
    }
}

/*!

*/
void ColorPickerButton::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up
	|| e->key() == Qt::Key_Down
	|| e->key() == Qt::Key_Left
	|| e->key() == Qt::Key_Right) {
	qApp->sendEvent(parent(), e);
    } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
	setFrameShadow(Raised);
	repaint();
	emit clicked();
    } else {
	QFrame::keyReleaseEvent(e);
    }

}

/*!

*/
void ColorPickerButton::focusInEvent(QFocusEvent *e)
{
    setFrameShadow(Raised);
    update();
    QFrame::focusOutEvent(e);
}

/*!

*/
void ColorPickerButton::focusOutEvent(QFocusEvent *e)
{
    setFrameShadow(Raised);
    update();
    QFrame::focusOutEvent(e);
}

/*!

*/
void ColorPickerButton::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter p(this);
    p.fillRect(contentsRect(), palette().button());

    QRect r = rect();

    int offset = frameShadow() == Sunken ? 1 : 0;

    QPen pen(palette().buttonText(), 1);
    p.setPen(pen);

    p.drawRect(r.center().x() + offset - 4, r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset    , r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset + 4, r.center().y() + offset, 1, 1);
    if (hasFocus()) {
	p.setPen( QPen( Qt::black, 0, Qt::SolidLine ) );
	p.drawRect(0, 0, width() - 1, height() - 1);
    }

    p.end();

}


