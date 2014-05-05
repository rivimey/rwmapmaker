/*
 * RWMapMaker is © IvimeyCom 2011-2014.
 * 
 * This file is part of RWMapMaker.
 * 
 * RWMapMaker is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * RWMapMaker is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with RWMapMaker.  If not, see http://www.gnu.org/licenses/gpl.html.
 */

#include <math.h>
#include <QtCore/QString>
#include <QtGui/QColor>

namespace Math {
	inline int roundint(double val)
	{
		return (int)ceil(val-0.5);
	}

	inline double min(double x, double y)
	{
		return (x < y) ? x : y;
	}

	inline double max(double x, double y)
	{
		return (x > y) ? x : y;
	}
}


class GColourBase
{
public:
    enum GColourModel
    {
        Unset,
        RGB,
        HSV,
        HSL,
        XYZ,
        CIELab
    };

protected:
	GColourModel _model;

	GColourBase(void) {}

	GColourBase(GColourModel m) : _model(m) {}

public:
	inline GColourModel ColourModel() { return _model; }

	inline QString toString() { return QString("GColourBase"); }

    virtual bool isValid() { return false; }

protected:
    // "Useful bits of code"

    /// <summary>
    /// Limit the value between 0 .. max
    /// </summary>
    static double CheckRange(double value, double max = 1.0);

    static void RGBtoHSV(double r, double g, double b, double &hue, double &saturation, double &value);
    static void HSVtoRGB(double hue, double saturation, double value, double &red, double &green, double &blue);
};

class GColourRGB;

class GColourHSV : public GColourBase
{
private:
	double _a, _h, _s, _v;

public:
	GColourHSV();

    GColourHSV(double h, double s, double v, double a = 1);

    /// <summary>
    /// 
    /// </summary>
    GColourHSV(GColourRGB &value);

    // two sets of names... luxury or silliness?
    double A() { return _a; }     void setA(double value) { _a = CheckRange(value); }
    double H() { return _h; }     void setH(double value) { _h = CheckRange(value, 359.99999); }
    double S() { return _s; }     void setS(double value) { _s = CheckRange(value); }
    double V() { return _v; }     void setV(double value) { _v = CheckRange(value); }
    double Hue() { return _h; }   void setHue(double value) { _h = CheckRange(value, 359.99999); }
    double Sat() { return _s; }   void setSat(double value) { _s = CheckRange(value); }
    double Value() { return _v; } void setValue(double value) { _v = CheckRange(value); }
	double Alpha() { return _a; } void setAlpha(double value) { _a = CheckRange(value); }
 
    static GColourHSV FromRGB(double r, double g, double b, double a = 1);

    QColor ToQColor();

    QString toString();

    bool isValid();
};



class GColourRGB : public GColourBase
{
private:
	double _a, _r, _g, _b;

public:
	GColourRGB();

    GColourRGB(double r, double g, double b, double a = 1);

    GColourRGB(GColourHSV &value);

    double A() { return _a; }     void setA(double value) { _a = CheckRange(value); }
    double R() { return _r; }     void setR(double value) { _r = CheckRange(value); }
    double G() { return _g; }     void setG(double value) { _g = CheckRange(value); }
    double B() { return _b; }     void setB(double value) { _b = CheckRange(value); }
    double Alpha() { return _a; } void setAlpha(double value) { _a = CheckRange(value); }
    double Red() { return _r; }   void setRed(double value) { _r = CheckRange(value); }
    double Green() { return _g; } void setGreen(double value) { _g = CheckRange(value); }
    double Blue() { return _b; }  void setBlue(double value) { _b = CheckRange(value); }

    /// <summary>
    /// 
    /// </summary>
    static GColourRGB FromHSV(double h, double s, double v, double a = 1);

    QColor ToQColor();

    QString toString();

    bool isValid();
};
