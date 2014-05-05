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

#include "stdafx.h"

#include "GColour.h"


double GColourBase::CheckRange(double value, double max)
{
    if (value < 0.0)
        value = 0.0;
    else if (value > max)
        value = max;
    return value;
}

void GColourBase::RGBtoHSV(double r, double g, double b, double &hue, double &saturation, double &value)
{
    double min, max, delta;

    min = Math::min(r, Math::min(g, b));
    max = Math::max(r, Math::max(g, b));
    value = max;				    // v
    delta = max - min;

    if (max > 0.00001)            // was (max != 0)
        saturation = delta / max;   // s
    else
    {
        // r = g = b = 0		    // s = 0, v is undefined
        saturation = 0;
        hue = -1;
        return;
    }

    // float comparison should be ok because max was copied from one of r,g,b

    if (r == max)
        hue = (g - b) / delta;		// between yellow & magenta
    else if (g == max)
        hue = 2 + (b - r) / delta;	// between cyan & yellow
    else
        hue = 4 + (r - g) / delta;	// between magenta & cyan

    hue *= 60;				            // degrees
    if (hue < 0)
        hue += 360;
}

void GColourBase::HSVtoRGB(double hue, double saturation, double value, double &red, double &green, double &blue)
{
    int hi = Math::roundint(floor(hue / 60)) % 6;
    double f = hue / 60 - floor(hue / 60);

    int v = Math::roundint(value);
    int p = Math::roundint(value * (1 - saturation));
    int q = Math::roundint(value * (1 - f * saturation));
    int t = Math::roundint(value * (1 - (1 - f) * saturation));

    if (hi == 0)
    {
        red = v; green = t; blue = p;
    }
    else if (hi == 1)
    {
        red = q; green = v; blue = p;
    }
    else if (hi == 2)
    {
        red = p; green = v; blue = t;
    }
    else if (hi == 3)
    {
        red = p; green = q; blue = v;
    }
    else if (hi == 4)
    {
        red = t; green = p; blue = v;
    }
    else
    {
        red = v; green = p; blue = q;
    }
}


GColourHSV::GColourHSV() 
{
	_model = HSV;
    _a = -1;  // not valid, see below.
}

GColourHSV::GColourHSV(double h, double s, double v, double a)
{
	_model = HSV;
    _a = CheckRange(a);
    _h = CheckRange(h, 359.99999);
    _s = CheckRange(s);
    _v = CheckRange(v);
}

/// <summary>
/// 
/// </summary>
GColourHSV::GColourHSV(GColourRGB &value)
{
    RGBtoHSV(value.R(), value.G(), value.B(), _h, _s, _v);
    _a = value.A();
}

GColourHSV GColourHSV::FromRGB(double r, double g, double b, double a)
{
    double h, s, v;
    r = CheckRange(r);
    g = CheckRange(g);
    b = CheckRange(b);
    a = CheckRange(a);
    RGBtoHSV(r, g, b, h, s, v);
    return GColourHSV(h,s,v,a);
}

QColor GColourHSV::ToQColor()
{
    // can't do better than go via RGB as QColor can't be init from non-RGB
    GColourRGB rgb = (GColourRGB)*this;
    return rgb.ToQColor();
}

QString GColourHSV::toString()
{
    return QString("H:%1,S:%2,V:%3,A:%4").arg(_h).arg(_s).arg(_v).arg(_a);
}

bool GColourHSV::isValid()
{
    return (ColourModel() == HSV &&
            (_a >= 0 && _a <= 1.0) && (_h >= 0 && _h < 360.0) &&
            (_s >= 0 && _s <= 1.0) && (_v >= 0 && _v <= 1.0));
}




GColourRGB::GColourRGB() : GColourBase(RGB)
{
    _a = -1;  // not valid, see below.
}

GColourRGB::GColourRGB(double r, double g, double b, double a) : GColourBase(RGB)
{
    _a = CheckRange(a);
    _r = CheckRange(r);
    _g = CheckRange(g);
    _b = CheckRange(b);
}

GColourRGB::GColourRGB(GColourHSV &value) : GColourBase(RGB)
{
    GColourBase::HSVtoRGB(value.H(), value.S(), value.V(), _r, _g, _b);
    _a = value.A();
}

/// <summary>
/// 
/// </summary>
GColourRGB GColourRGB::FromHSV(double h, double s, double v, double a)
{
    double r, g, b;
    h = CheckRange(h);
    s = CheckRange(s);
    v = CheckRange(v);
    a = CheckRange(a);
    GColourBase::HSVtoRGB(h, s, v, r, g, b);
    return GColourRGB(r, g, b, a);
}

QColor GColourRGB::ToQColor()
{
    return QColor(Math::roundint(_r * 255), Math::roundint(_g * 255), Math::roundint(_b * 255), Math::roundint(_a * 255));
}

QString GColourRGB::toString()
{
    return QString("R:%1,G:%2,B:%3,A:%4").arg(_r).arg(_g).arg(_b).arg(_a);
}

bool GColourRGB::isValid()
{
    return (ColourModel() == RGB &&
            (_a >= 0 && _a <= 1.0) && (_r >= 0 && _r <= 1.0) &&
            (_g >= 0 && _g <= 1.0) && (_b >= 0 && _b <= 1.0));
}

