#ifndef QXTGRADIENTSLIDER_H
#define QXTGRADIENTSLIDER_H

#include <QtGui/QPen>
#include <QtWidgets/QPushButton>
#include <QtCore/QRect>
#include <QtCore/QPoint>

#include "qxtgradientbar.h"

class QxtGradientSlider : public QxtGradientBar
{
	Q_OBJECT;

	QPen endMarkPen;
	QPen midMarkPen;
	QList<QPushButton*> buttons;

public:
	QxtGradientSlider(QWidget *parent = NULL);
	~QxtGradientSlider();

	virtual void setGradient(QxtGradient *g);

private:
	void paintEndmark(QPainter &painter, QRect &overall, int pos);
	void paintMidmark(QPainter &painter, QRect &overall, int pos);

	void updateButtons();


	virtual void paintEvent(QPaintEvent* event);

    //virtual void mousePressEvent(QMouseEvent *event);
    //virtual void mouseReleaseEvent(QMouseEvent *event);
    //virtual void mouseDoubleClickEvent(QMouseEvent *event);
    //virtual void mouseMoveEvent(QMouseEvent *event);

    //virtual void keyPressEvent(QKeyEvent *event);
    //virtual void keyReleaseEvent(QKeyEvent *event);
    //virtual void focusInEvent(QFocusEvent *event);
    //virtual void focusOutEvent(QFocusEvent *event);
    //virtual void enterEvent(QEvent *event);
    //virtual void leaveEvent(QEvent *event);
};

#endif // QXTGRADIENTSLIDER_H
