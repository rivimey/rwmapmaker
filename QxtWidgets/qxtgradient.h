#ifndef QXTGRADIENT_H
#define QXTGRADIENT_H

#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <QtCore/QList>

class QxtGradientSegment : public QObject
{
	Q_OBJECT;

public:
	enum QxtGradientFunction
	{
		GradFn_Linear,
		GradFn_Curved,
		GradFn_Sinusoidal,
		GradFn_SphericalInc,
		GradFn_SphericalDec
	};

	enum QxtGradColorspace
	{
		GradCS_RGB,
		GradCS_HSV,
		GradCS_HSL
	};

private:
	double lPos;  // pos of left end
	QColor lCol;

	double mPos;  // pos of marker

	double rPos;  // pos of upper end
	QColor rCol;

	QxtGradientFunction func;
	QxtGradColorspace cSpace;

public:
	QxtGradientSegment();
	QxtGradientSegment(double Lpos, QColor &Lcol, double markPos, double Rpos, QColor &Rcol);

	QxtGradColorspace colorSpace() const;
	QxtGradientFunction function() const;
	QColor lowerColor() const;
	QColor upperColor() const;
	double lowerPos() const;
	double upperPos() const;
	double markerPos() const;

	QColor color(double x) const;

public Q_SLOTS:
	void setColorSpace(QxtGradColorspace cs);
	void setFunction(QxtGradientFunction f);
	void setLowerPos(double p);
	void setLowerColor(QColor &c);
	void setUpperPos(double p);
	void setUpperColor(QColor &c);
	void setMarkerPos(double p);


Q_SIGNALS:
	void gradientChanged();
	void markerPosChanged(double p);
	void lowerPosChanged(double v);
	void lowerColorChanged(QColor &c);
	void upperPosChanged(double v);
	void upperColorChanged(QColor &c);
	void functionChanged(QxtGradientFunction f);

private:
	double foldRange(double x);
	bool checkRange(double x);

	friend class QxtGradient;
};

typedef QList<QxtGradientSegment*> QxtGradientSegmentList;

class QxtGradient : public QObject, public QxtGradientSegmentList
{
	Q_OBJECT;

public:
	QxtGradient();
	~QxtGradient();

	void linkPosition(int x, int y);
	void linkPositions();
	void unlinkPositions();

	QColor color(double p);

	void append(QxtGradientSegment *s);
	void clear();
};


#endif // QXTGRADIENT_H
