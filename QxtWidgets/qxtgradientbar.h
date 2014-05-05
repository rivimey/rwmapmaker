#ifndef QXTGRADIENTBAR_H
#define QXTGRADIENTBAR_H

#include <QtCore/QObject>
#include <QtWidgets/QFrame>
#include <QtCore/QList>

#include "qxtgradient.h"


class QxtGradientBar : public QFrame
{
	Q_OBJECT;
	Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation);
	Q_PROPERTY(QxtGradient * gradient READ gradient WRITE setGradient);

	QxtGradient *theGradient;
	Qt::Orientation orient;

public:
	QxtGradientBar(QWidget *parent = NULL);
	QxtGradientBar(QxtGradient *grad, QWidget *parent = NULL);
	~QxtGradientBar();

	QxtGradient *gradient() const;
	virtual void setGradient(QxtGradient *g);

	Qt::Orientation orientation() const;
	virtual void setOrientation(Qt::Orientation o);

	double pointToValue(double point);
	double valueToPoint(double value);

protected:
	virtual void paintEvent(QPaintEvent* event);

Q_SIGNALS:
	void gradientChanged();

private:
	void gradientUpdate();
	void setSigs();
	void unsetSigs();
};

#endif // QXTGRADIENTBAR_H
