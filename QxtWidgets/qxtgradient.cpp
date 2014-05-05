#include "qxtgradient.h"

#include <QtWidgets/QStylePainter>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

QxtGradientSegment::QxtGradientSegment() :
	lPos(0), mPos(0.5), rPos(1), func(GradFn_Linear), cSpace(GradCS_RGB)
{
}

QxtGradientSegment::QxtGradientSegment(double Lpos, QColor &Lcol, double markPos, double Rpos, QColor &Rcol) :
	lPos(Lpos), lCol(Lcol), mPos(markPos), rPos(Rpos), rCol(Rcol), func(GradFn_Linear), cSpace(GradCS_RGB)
{
}

QxtGradientSegment::QxtGradColorspace QxtGradientSegment::colorSpace() const
{
	return cSpace;
}

QxtGradientSegment::QxtGradientFunction QxtGradientSegment::function() const
{
	return func;
}

QColor QxtGradientSegment::lowerColor() const
{
	return lCol;
}

QColor QxtGradientSegment::upperColor() const
{
	return rCol;
}

double QxtGradientSegment::lowerPos() const
{
	return lPos;
}

double QxtGradientSegment::upperPos() const
{
	return rPos;
}

double QxtGradientSegment::markerPos() const
{
	return mPos;
}


void QxtGradientSegment::setColorSpace(QxtGradColorspace cs)
{
	if (cs != cSpace)
	{
		cSpace = cs;
	}
}

void QxtGradientSegment::setFunction(QxtGradientFunction f)
{
	if (func != f)
	{
		func = f;
		emit functionChanged(func);
		emit gradientChanged();
	}
}

void QxtGradientSegment::setLowerPos(double p)
{
	if (lPos != foldRange(p))
	{
		lPos = foldRange(p);
		emit lowerPosChanged(lPos);
		emit gradientChanged();
	}
}

void QxtGradientSegment::setLowerColor(QColor &c)
{
	if (lCol != c)
	{
		lCol = c;
		emit lowerColorChanged(lCol);
		emit gradientChanged();
	}
}

void QxtGradientSegment::setUpperPos(double p)
{
	if (rPos != foldRange(p))
	{
		rPos = foldRange(p);
		emit upperPosChanged(rPos);
		emit gradientChanged();
	}
}

void QxtGradientSegment::setUpperColor(QColor &c)
{
	if (rCol != c)
	{
		rCol = c;
		emit upperColorChanged(rCol);
		emit gradientChanged();
	}
}

void QxtGradientSegment::setMarkerPos(double p)
{
	if (mPos != foldRange(p))
	{
		mPos = foldRange(p);
		emit markerPosChanged(mPos);
		emit gradientChanged();
	}
}


bool QxtGradientSegment::checkRange(double x)
{
	return (x >= 0 && x <= 1.0);
}

double QxtGradientSegment::foldRange(double x)
{
	if (x >= 0 && x <= 1.0)
		return x;
	else if (x < 0)
		return 0;
	else
		return 1;
}


QColor QxtGradientSegment::color(double x) const
{
	double f;

    // Normalize the segment geometry.
    double mid = (mPos - lPos)/(rPos - lPos);
    double pos = (x - lPos)/(rPos - lPos);
        
    // Assume linear (most common, and needed by most others).
    if (pos <= mid)
        f = pos/mid/2;
    else
        f = (pos - mid)/(1 - mid)/2 + 0.5;

    // Find the correct interpolation factor.
    switch(func)
    {
		case 0:   // Linear
			break;

        case 1:   // Curved
            f = pow(pos, log(0.5) / log(mid));
            break;

        case 2:   // Sinusoidal
            f = (sin(( - M_PI / 2) + M_PI * f) + 1)/2;
            break;

        case 3:   // Spherical increasing
            f -= 1;
            f = sqrt(1 - f*f);
            break;
                
        case 4:   // Spherical decreasing
            f = 1 - sqrt(1 - f*f);
            break;

		default:
			throw std::logic_error("Unknown Gradient Function");
    }

    // Interpolate the colors
    if (cSpace == QxtGradientSegment::GradCS_RGB)
		return QColor( lCol.red() + (rCol.red() - lCol.red()) * f,
					   lCol.green() + (rCol.green() - lCol.green()) * f,
					   lCol.blue() + (rCol.blue() - lCol.blue()) * f      );

    else // (cSpace == QxtGradientSegment::GradCS_HSV || cSpace == QxtGradientSegment::GradCS_HSL)
		return QColor();
}



QxtGradient::QxtGradient()
{
	QxtGradientSegment *seg = new QxtGradientSegment(0, QColor(Qt::black), 0.5, 1.0, QColor(Qt::white));
	append(seg);
}

QxtGradient::~QxtGradient()
{
	for(int i = 0; i < size(); i++)
		delete at(i);
}

void QxtGradient::clear()
{
	for(int i = 0; i < size(); i++)
		delete at(i);
	QxtGradientSegmentList::clear();
}

void QxtGradient::append(QxtGradientSegment *seg)
{
	QxtGradientSegmentList::append(seg);
	if (size() > 1)
	{
		linkPosition(size()-2, size()-1);
	}
}

void QxtGradient::unlinkPositions()
{
	int i;
	for(i = 0; i < size(); i++)
	{
		QObject::disconnect(at(i), SIGNAL(upperPosChanged(QColor&)));
		QObject::disconnect(at(i), SIGNAL(upperColorChanged(QColor&)));
	}
}

void QxtGradient::linkPositions()
{
	unlinkPositions();

	// recreate signal links between adjacent items.
	for(int i = 1; i < size(); i++)
	{
		QObject::connect(at(i-1), SIGNAL(upperPosChanged(double)),    at(i), SLOT(setLowerPos(double)));
		QObject::connect(at(i-1), SIGNAL(upperColorChanged(QColor&)), at(i), SLOT(setLowerColor(QColor&)));
	}
}

void QxtGradient::linkPosition(int x, int y)
{
	assert(x >= 0 && x < size());
	assert(y >= 0 && y < size());
	assert(x != y);

	QObject::connect(at(x), SIGNAL(upperPosChanged(double)),    at(y), SLOT(setLowerPos(double)));
	QObject::connect(at(x), SIGNAL(upperColorChanged(QColor&)), at(y), SLOT(setLowerColor(QColor&)));
}


QColor QxtGradient::color(double x)
{
    // Get the color for the point x in the range [0..1).
    //  The color is returned as an rgb triple, with all values in the range
    //  [0..1).

    // Find the segment.
    const QxtGradientSegment *seg = 0;
	bool found = false;
    for(int i = 0; i < this->size(); i++)
    {
		const QxtGradientSegment *s = at(i);
        if (s->lPos <= x && x <= s->rPos)
        {
            seg = s;
			found = true;
            break;
        }
    }

    if (!found)
        // No segment applies! Return black
		return QColor(Qt::black);

    return seg->color(x);
}



