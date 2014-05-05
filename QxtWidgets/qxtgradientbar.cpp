#include "qxtgradient.h"
#include "qxtgradientbar.h"
#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtCore/QRect>
#include <QtCore/QPoint>
#include <assert.h>

QxtGradientBar::QxtGradientBar(QxtGradient *grad, QWidget *parent)
	: QFrame(parent), theGradient(grad), orient(Qt::Horizontal)
{
}

QxtGradientBar::QxtGradientBar(QWidget *parent)
	: QFrame(parent), theGradient(NULL), orient(Qt::Horizontal)
{
}

QxtGradientBar::~QxtGradientBar()
{
	theGradient = NULL;
}

/*
	Connect the required signals between the gradient object and this widget.
*/
void QxtGradientBar::setSigs()
{
	if (theGradient != NULL)
		connect(theGradient, SIGNAL(gradientChanged()), this, SLOT(gradientUpdate()));
}

/*
	Disconnect the required signals between the gradient object and this widget.
*/
void QxtGradientBar::unsetSigs()
{
	if (theGradient != NULL)
		disconnect(theGradient, SIGNAL(gradientChanged()), this, SLOT(gradientUpdate()));
}

/*
	Public function to cause an update of the widget.
*/
void QxtGradientBar::gradientUpdate()
{
	update();
}

/*
	Return the orientation of the widget's gradient.
*/
Qt::Orientation QxtGradientBar::orientation() const
{
	return orient;
}

/*
	Set the orientation of the widget. Horizontal means
	lines of the same colour are vertical and the "lower"
	position is at the small-x end. Vertical means lines
	of the same colour are horizontal and the "lower"
	position is at the small-y end.
*/
void QxtGradientBar::setOrientation(Qt::Orientation o)
{
	if (orient == o)
	{
		orient = o;
		update();
	}
}

/*
	Return a pointer to the current QxtGradient object, which is
	the container of the gradient information.
*/
QxtGradient *QxtGradientBar::gradient() const
{
	return theGradient;
}

/*
	Set the gradient object, which maps a value from 0 to 1 to a 
	specific colour. When the gradient object actually changes
	emit the gradientChanged() signal.
*/
void QxtGradientBar::setGradient(QxtGradient *g)
{
	if (g != theGradient)
	{
		unsetSigs();
		theGradient = g;
		emit gradientChanged();
		setSigs();
		
		update();
	}
}

/*
	Given a value within a gradient in the range 0 to 1, return the 
	coordinate in the widget.
*/
double QxtGradientBar::valueToPoint(double value)
{
	double p;
	assert(value >= 0 && value <= 1);
	QRect overall = contentsRect();

	if (orientation() == Qt::Horizontal)
		p = overall.left() + value * overall.width();
	else
		p = overall.top() + value * overall.height();
	return p;
}

/*
	Given a point within the widget's width orheight, return the gradient value
	for that point in the range 0 to 1.
*/
double QxtGradientBar::pointToValue(double point)
{
	double v;
	QRect overall = contentsRect();

	if (orientation() == Qt::Horizontal)
		v = (point - overall.left()) / overall.width();
	else
		v = (point - overall.top()) / overall.height();

	assert(v >= 0 && v <= 1);
	return v;
}

/*
	Paint the gradient as colour bars using the complete area of the widget,
	and the associated gradient object to determine colours. If there is
	no gradient object then fill with mid-grey.
*/
void QxtGradientBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
	//QRect overall(QPoint(0,0), this->size());
	QRect overall = contentsRect();

	if (theGradient == NULL)
	{
		painter.fillRect(overall, Qt::gray);
	}
	else
	{
		if (orientation() == Qt::Horizontal)
		{
			for (int i = overall.left(); i < overall.right(); i++)
			{
				double x = (i - overall.left()) / (double)overall.width();
				painter.setPen(QPen(theGradient->color(x),1));
				painter.drawLine(i, overall.top(), i, overall.bottom());
			}
		}
		else
		{
			for (int i = overall.top(); i < overall.bottom(); i++)
			{
				double x = (i - overall.top()) / (double)overall.height();
				painter.setPen(QPen(theGradient->color(x),1));
				painter.drawLine(overall.left(), i, overall.right(), i);
			}
		}
	}
}
