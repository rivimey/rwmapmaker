#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtGui/QPainter>

#include "qxtgradient.h"
#include "qxtgradientslider.h"

QxtGradientSlider::QxtGradientSlider(QWidget *parent)
	: QxtGradientBar(parent)
{
	endMarkPen = QPen(Qt::black);
	midMarkPen = QPen(Qt::red);
}

QxtGradientSlider::~QxtGradientSlider()
{

}


void QxtGradientSlider::paintEndmark(QPainter &painter, QRect &overall, int pos)
{
	QPoint p1, p2;
	if (orientation() == Qt::Horizontal)
	{
		p1 = QPoint(pos, overall.bottom());
		p2 = QPoint(pos, overall.top());
	}
	else
	{
		p1 = QPoint(overall.left(), pos);
		p2 = QPoint(overall.right(), pos);
	}
	painter.setPen(endMarkPen);
	painter.drawLine(p1, p2);
}

void QxtGradientSlider::paintMidmark(QPainter &painter, QRect &overall, int pos)
{
	QPoint p1, p2;
	if (orientation() == Qt::Horizontal)
	{
		p1 = QPoint(pos, overall.bottom());
		p2 = QPoint(pos, overall.top());
	}
	else
	{
		p1 = QPoint(overall.left(), pos);
		p2 = QPoint(overall.right(), pos);
	}
	painter.setPen(midMarkPen);
	painter.drawLine(p1, p2);
}

void QxtGradientSlider::setGradient(QxtGradient *g)
{
	QxtGradientBar::setGradient(g);

	updateButtons();
}

void QxtGradientSlider::updateButtons()
{
	if (gradient() == NULL)
	{
		buttons.clear();
	}
	else
	{
		for(int i = 0; i < gradient()->size(); i++)
		{
			QPushButton *b;
			const QxtGradientSegment *s = gradient()->at(i);
			b = new QPushButton(this);
			b->setUserData(0,(QObjectUserData*)s);
			b->setBackgroundRole(QPalette::Light);
			b->setFixedSize(6,8);
			b->setFlat(true);
			b->move((int)valueToPoint(s->lowerPos()), y());
			b->show();
			buttons.append(b);

			b = new QPushButton(this);
			b->setUserData(0,(QObjectUserData*)s);
			b->setBackgroundRole(QPalette::Midlight);
			b->setFixedSize(6,8);
			b->setFlat(true);
			b->move((int)valueToPoint(s->markerPos()), y());
			b->show();
			buttons.append(b);

			b = new QPushButton(this);
			b->setUserData(0,(QObjectUserData*)s);
			b->setBackgroundRole(QPalette::Light);
			b->setFixedSize(6,8);
			b->setFlat(true);
			b->move((int)valueToPoint(s->upperPos()), y());
			b->show();
			buttons.append(b);
		}
	}
}

void QxtGradientSlider::paintEvent(QPaintEvent* event)
{
	// paint the gradient itself
	QxtGradientBar::paintEvent(event);

	// add in the markers
	if (gradient() != NULL)
	{
	    QPainter painter(this);
		QRect overall(QPoint(0,0), this->size());

		for(int i = 0; i < gradient()->size(); i++)
		{
			const QxtGradientSegment *s = gradient()->at(i);
			paintEndmark(painter, overall, (int)valueToPoint(s->lowerPos()));
			paintMidmark(painter, overall, (int)valueToPoint(s->markerPos()));
			paintEndmark(painter, overall, (int)valueToPoint(s->upperPos()));
		}
	}
}
