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

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QLayout>
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolTip>
#include <math.h>

#include "qtcolorpicker_p.h"
#include "qtcolorpicker.h"

/*! \class QtColorPicker

    \brief The QtColorPicker class provides a widget for selecting
    colors from a popup color grid.

    Users can invoke the color picker by clicking on it, or by
    navigating to it and pressing Space. They can use the mouse or
    arrow keys to navigate between colors on the grid, and select a
    color by clicking or by pressing Enter or Space. The
    colorChanged() signal is emitted whenever the color picker's color
    changes.

    The widget also supports negative selection: Users can click and
    hold the mouse button on the QtColorPicker widget, then move the
    mouse over the color grid and release the mouse button over the
    color they wish to select.

    The color grid shows a customized selection of colors. An optional
    ellipsis "..." button (signifying "more") can be added at the
    bottom of the grid; if the user presses this, a QColorDialog pops
    up and lets them choose any color they like. This button is made
    available by using setColorDialogEnabled().

    When a color is selected, the QtColorPicker widget shows the color
    and its name. If the name cannot be determined, the translatable
    name "Custom" is used.

    The QtColorPicker object is optionally initialized with the number
    of columns in the color grid. Colors are then added left to right,
    top to bottom using insertColor(). If the number of columns is not
    set, QtColorPicker calculates the number of columns and rows that
    will make the grid as square as possible.

    \code
    DrawWidget::DrawWidget(QWidget *parent, const char *name)
    {
        QtColorPicker *picker = new QtColorPicker(this);
        picker->insertColor(red, "Red"));
        picker->insertColor(QColor("green"), "Green"));
        picker->insertColor(QColor(0, 0, 255), "Blue"));
        picker->insertColor(white);

        connect(colors, SIGNAL(colorChanged(const QColor &)), SLOT(setCurrentColor(const QColor &)));
    }
    \endcode

    An alternative to adding colors manually is to initialize the grid
    with QColorDialog's standard colors using setStandardColors().

    QtColorPicker also provides a the static function getColor(),
    which pops up the grid of standard colors at any given point.

    \img colorpicker1.png
    \img colorpicker2.png

    \sa QColorDialog
*/

/*! \fn QtColorPicker::colorChanged(const QColor &color)

    This signal is emitted when the QtColorPicker's color is changed.
    \a color is the new color.

    To obtain the color's name, use text().
*/

/*!
    Constructs a QtColorPicker widget. The popup will display a grid
    with \a cols columns, or if \a cols is -1, the number of columns
    will be calculated automatically.

    If \a enableColorDialog is true, the popup will also have a "More"
    button (signified by an ellipsis "...") that presents a
    QColorDialog when clicked.

    After constructing a QtColorPicker, call insertColor() to add
    individual colors to the popup grid, or call setStandardColors()
    to add all the standard colors in one go.

    The \a parent argument is passed to QFrame's constructor.

    \sa QFrame
*/
QtColorPicker::QtColorPicker(QWidget *parent,
			     int cols, bool enableColorDialog)
    : QPushButton(parent), popup(0), withColorDialog(enableColorDialog)
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setAutoDefault(false);
    setAutoFillBackground(true);
    setCheckable(true);

    // Set text
    setText(tr("Black"));
    firstInserted = false;

    // Create and set icon
    col = Qt::black;
    dirty = true;

    // Create color grid popup and connect to it.
    popup = new ColorPickerPopup(cols, withColorDialog, this);
    connect(popup, SIGNAL(selected(const QColor &)),
	    SLOT(setCurrentColor(const QColor &)));
    connect(popup, SIGNAL(hid()), SLOT(popupClosed()));

    // Connect this push button's pressed() signal.
    connect(this, SIGNAL(toggled(bool)), SLOT(buttonPressed(bool)));
}

/*!
    Destructs the QtColorPicker.
*/
QtColorPicker::~QtColorPicker()
{
}

void QtColorPicker::setPopupShape(QtColorPopupShape s)
{
	shape = s;
	if (popup)
	{
		popup->setPopupShape(s);
	}
}

void QtColorPicker::setPopupRowsOrCols(int rc)
{
	rowsOrCols = rc;
	if (popup)
		popup->setPopupRowsOrCols(rc);
}


/*! \internal

    Pops up the color grid, and makes sure the status of
    QtColorPicker's button is right.
*/
void QtColorPicker::buttonPressed(bool toggled)
{
    if (!toggled)
        return;

    const QRect desktop = QApplication::desktop()->geometry();
    // Make sure the popup is inside the desktop.
    QPoint pos = mapToGlobal(rect().bottomLeft());
    if (pos.x() < desktop.left())
       pos.setX(desktop.left());
    if (pos.y() < desktop.top())
       pos.setY(desktop.top());

    if ((pos.x() + popup->sizeHint().width()) > desktop.width())
       pos.setX(desktop.width() - popup->sizeHint().width());
    if ((pos.y() + popup->sizeHint().height()) > desktop.bottom())
       pos.setY(desktop.bottom() - popup->sizeHint().height());
    popup->move(pos);

    if (ColorPickerItem *item = popup->find(col))
        item->setSelected(true);

    // Remove focus from this widget, preventing the focus rect
    // from showing when the popup is shown. Order an update to
    // make sure the focus rect is cleared.
    clearFocus();
    update();

    // Allow keyboard navigation as soon as the popup shows.
    popup->setFocus();

    // Execute the popup. The popup will enter the event loop.
    popup->show();
}

/*!
    \internal
*/
void QtColorPicker::paintEvent(QPaintEvent *e)
{
    if (dirty) {
        int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
        QPixmap pix(iconSize, iconSize);
        pix.fill(palette().button().color());

        QPainter p(&pix);

        int w = pix.width();			// width of cell in pixels
        int h = pix.height();			// height of cell in pixels
        p.setPen(QPen(Qt::gray));
        p.setBrush(col);
        p.drawRect(2, 2, w - 5, h - 5);
        setIcon(QIcon(pix));

        dirty = false;
    }
    QPushButton::paintEvent(e);
}

/*! \internal

    Makes sure the button isn't pressed when the popup hides.
*/
void QtColorPicker::popupClosed()
{
    setChecked(false);
    setFocus();
}

/*!
    Returns the currently selected color.

    \sa text()
*/
QColor QtColorPicker::currentColor() const
{
    return col;
}

/*!
    Returns the color at position \a index.
*/
QColor QtColorPicker::color(int index) const
{
    return popup->color(index);
}

/*!
    Adds the 17 predefined colors from the Qt namespace.

    (The names given to the colors, "Black", "White", "Red", etc., are
    all translatable.)

    \sa insertColor()
*/
void QtColorPicker::setStandardColors()
{
    insertColor(Qt::black, tr("Black"));
    insertColor(Qt::white, tr("White"));
    insertColor(Qt::red, tr("Red"));
    insertColor(Qt::darkRed, tr("Dark red"));
    insertColor(Qt::green, tr("Green"));
    insertColor(Qt::darkGreen, tr("Dark green"));
    insertColor(Qt::blue, tr("Blue"));
    insertColor(Qt::darkBlue, tr("Dark blue"));
    insertColor(Qt::cyan, tr("Cyan"));
    insertColor(Qt::darkCyan, tr("Dark cyan"));
    insertColor(Qt::magenta, tr("Magenta"));
    insertColor(Qt::darkMagenta, tr("Dark magenta"));
    insertColor(Qt::yellow, tr("Yellow"));
    insertColor(Qt::darkYellow, tr("Dark yellow"));
    insertColor(Qt::gray, tr("Gray"));
    insertColor(Qt::darkGray, tr("Dark gray"));
    insertColor(Qt::lightGray, tr("Light gray"));
}


/*!
    Makes \a color current. If \a color is not already in the color grid, it
    is inserted with the text "Custom".

    This function emits the colorChanged() signal if the new color is
    valid, and different from the old one.
*/
void QtColorPicker::setCurrentColor(const QColor &color)
{
    if (col == color || !color.isValid())
		return;

    ColorPickerItem *item = popup->find(color);
    if (!item) {
		insertColor(color, tr("Custom"));
		item = popup->find(color);
    }

    col = color;
    setText(item->text());

    dirty = true;

    popup->hide();
    repaint();

    item->setSelected(true);
    emit colorChanged(color);
}

/*!
    Adds the color \a color with the name \a text to the color grid,
    at position \a index. If index is -1, the color is assigned
    automatically assigned a position, starting from left to right,
    top to bottom.
*/
void QtColorPicker::insertColor(const QColor &color, const QString &text, int index)
{
    popup->insertColor(color, text, index);
    if (!firstInserted) {
		col = color;
		setText(text);
		firstInserted = true;
	}
}

/*! \property QtColorPicker::colorDialog
    \brief Whether the ellipsis "..." (more) button is available.

    If this property is set to TRUE, the color grid popup will include
    a "More" button (signified by an ellipsis, "...") which pops up a
    QColorDialog when clicked. The user will then be able to select
    any custom color they like.
*/
void QtColorPicker::setColorDialogEnabled(bool enabled)
{
    withColorDialog = enabled;
}
bool QtColorPicker::colorDialogEnabled() const
{
    return withColorDialog;
}

/*!
    Pops up a color grid with Qt default colors at \a point, using
    global coordinates. If \a allowCustomColors is true, there will
    also be a button on the popup that invokes QColorDialog.

    For example:

    \code
        void Drawer::mouseReleaseEvent(QMouseEvent *e)
        {
	    if (e->button() & RightButton) {
                QColor color = QtColorPicker::getColor(mapToGlobal(e->pos()));
            }
        }
    \endcode
*/
QColor QtColorPicker::getColor(const QPoint &point, bool allowCustomColors)
{
    ColorPickerPopup popup(-1, allowCustomColors);

    popup.insertColor(Qt::black, tr("Black"), 0);
    popup.insertColor(Qt::white, tr("White"), 1);
    popup.insertColor(Qt::red, tr("Red"), 2);
    popup.insertColor(Qt::darkRed, tr("Dark red"), 3);
    popup.insertColor(Qt::green, tr("Green"), 4);
    popup.insertColor(Qt::darkGreen, tr("Dark green"), 5);
    popup.insertColor(Qt::blue, tr("Blue"), 6);
    popup.insertColor(Qt::darkBlue, tr("Dark blue"), 7);
    popup.insertColor(Qt::cyan, tr("Cyan"), 8);
    popup.insertColor(Qt::darkCyan, tr("Dark cyan"), 9);
    popup.insertColor(Qt::magenta, tr("Magenta"), 10);
    popup.insertColor(Qt::darkMagenta, tr("Dark magenta"), 11);
    popup.insertColor(Qt::yellow, tr("Yellow"), 12);
    popup.insertColor(Qt::darkYellow, tr("Dark yellow"), 13);
    popup.insertColor(Qt::gray, tr("Gray"), 14);
    popup.insertColor(Qt::darkGray, tr("Dark gray"), 15);
    popup.insertColor(Qt::lightGray, tr("Light gray"), 16);

    popup.move(point);
    popup.exec();
    return popup.lastSelected();
}

