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

#include <QtCore/QString>
#include <math.h>
#include "Settings.h"
#include "GradGGR.h"

#define PI 3.141592653589793

// Read Gimp .ggr gradient files.
// Ned Batchelder, http://nedbatchelder.com
// This code is in the public domain.
//
// Ggr.py reads Gimp .ggr gradient files, providing a Python interface to get color
// values from them. There are only two methods on GimpGradient:
//
// read(f) reads a .ggr file from f, either an open file or a file path. The constructor
// also take an optional file to read as an argument.  color(pos) takes a position between
// 0 and 1 and returns the rgb color at that point along the gradient.
//
// >>> import ggr
// >>> g = ggr.GimpGradient('Sunrise.ggr')
// >>> g.color(.25)
// (0.95501612668614855, 0.91457275670187077, 0.74046874445166)
// >>> g.color(.5)
// (0.83733161389613664, 0.37510844064480114, 0.23112679845322415)
// >>> g.color(.75)
// (0.42744738324707615, 0.52517311887199969, 0.83543339174375486)
//
// This Python code is in the public domain.
// __version__ = '1.0.20070915'
// Converted from Python to C# by Ruth Ivimey-Cook, January 2013
// Converted from C# to C++ by Ruth Ivimey-Cook, March 2013

//import colorsys // now uses GColor Classes

GimpGradient::GimpGradient(QTextStream &fs)
	 : segs(0), nsegs(0), isvalid(false)
{
	if (fs.status() == QTextStream::Ok)
		read(fs);
}

/*!
	Read and interpret a Gimp .ggr gradient file.
	Exceptions are caught and ignored.
	\a name Path name of the GGR file to read
 */
GimpGradient::GimpGradient(const QString &name)
	 : segs(0), nsegs(0), isvalid(false)
{
	read(name);
}

/*!
	Read and interpret a Gimp .ggr gradient file.
 */
void GimpGradient::read(QTextStream &f)
{
    /// Read a .ggr file from f (either an open file or a file path).
    QString line;

	if (f.readLine().trimmed() != "GIMP Gradient")
        throw std::logic_error("Not a GIMP gradient file");
    line = f.readLine().trimmed();
	if (! line.startsWith("Name: "))
        throw std::logic_error("Not a GIMP gradient file");
    name = line.mid(6);  // 6 == length("Name: ")

    nsegs = QString(f.readLine().trimmed()).toInt();
	segs = new 	GGRSegmentList();
    for(int i = 0; i < nsegs; i++)
    {
        line = f.readLine().trimmed();
        GGRSegment seg;
        QStringList &split = line.split(QRegExp("[ \\t]"));
        //if (split.Length < 14)
        //    throw new InvalidDataException("Insufficient values in GGR file line " + i.toString());
        seg.l =  QString(split[ 0]).toDouble();
        seg.m =  QString(split[ 1]).toDouble();
        seg.r =  QString(split[ 2]).toDouble();
        seg.rl = QString(split[ 3]).toDouble();
        seg.gl = QString(split[ 4]).toDouble();
        seg.bl = QString(split[ 5]).toDouble();
        seg.al = QString(split[ 6]).toDouble();
        seg.rr = QString(split[ 7]).toDouble();
        seg.gr = QString(split[ 8]).toDouble();
        seg.br = QString(split[ 9]).toDouble();
        seg.ar = QString(split[10]).toDouble();
        seg.fn =    QString(split[12]).toInt();
        seg.space = QString(split[13]).toInt();
        segs->append(seg);
    }
	isvalid = true;
}


/*!
	Read the contents of the gradient from the named file.
 */
void GimpGradient::read(const QString &fname)
{
	delete segs;
	isvalid = false;
	nsegs = 0;

	QFile theFile(fname);
	if (theFile.open(QIODevice::ReadOnly))
	{
		QTextStream fs(&theFile);
		if (fs.status() == QTextStream::Ok)
		{
			read(fs);
		}
		theFile.close();
	}
}

/*!
	Write the contents of the gradient to the named file.
 */
void GimpGradient::write(const QString &fname)
{
	QFile theFile(fname);
	if (theFile.open(QIODevice::WriteOnly))
	{
		QTextStream fs(&theFile);
		if (fs.status() == QTextStream::Ok)
		{
			write(fs);
		}
		theFile.close();
	}
}

/*!
	Write the contents of the gradient to the stream.
 */
void GimpGradient::write(QTextStream &f)
{
    QString line;
	f << "GIMP Gradient" << endl;
	f << "Name: " << name << endl;
	f << nsegs << endl;
	f.setRealNumberNotation(QTextStream::FixedNotation);
	f.setRealNumberPrecision(6);
    for(int i = 0; i < nsegs; i++)
    {
		const GGRSegment &seg = segs->at(i);
		f << seg.l  << " " << seg.m  << " " << seg.r  << " " << 
			 seg.rl << " " << seg.gl << " " << seg.bl << " " << seg.al << " " << 
			 seg.rr << " " << seg.gr << " " << seg.br << " " << seg.ar << " " << 
			 seg.fn << " " << seg.space << endl;
	}
}

/*!
	Generate a Colour for the point on the gradient defined by \a x
	where x is between 0 and 1 inclusive.
 */
GColourBase *GimpGradient::color(double x)
{
    double f;

	if (!isvalid)
		return NULL;

    // Get the color for the point x in the range [0..1).
    //  The color is returned as an rgb triple, with all values in the range
    //  [0..1).

    // Find the segment.
    GGRSegment seg;
	bool found = false;
    for(int i = 0; i < segs->size(); i++)
    {
		const 	GGRSegment &s = segs->at(i);
        if (s.l <= x && x <= s.r)
        {
            seg = s;
			found = true;
            break;
        }
    }

    if (!found)
        // No segment applies! Return black I guess.
        return new GColourRGB(0,0,0);

    // Normalize the segment geometry.
    double mid = (seg.m - seg.l)/(seg.r - seg.l);
    double pos = (x - seg.l)/(seg.r - seg.l);
        
    // Assume linear (most common, and needed by most others).
    if (pos <= mid)
        f = pos/mid/2;
    else
        f = (pos - mid)/(1 - mid)/2 + 0.5;

    // Find the correct interpolation factor.
    switch(seg.fn)
    {
		case 0:   // Linear
			break;

        case 1:   // Curved
            f = pow(pos, log(0.5) / log(mid));
            break;

        case 2:   // Sinusoidal
            f = (sin(( - PI / 2) + PI * f) + 1)/2;
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
    if (seg.space == 0)
        return new GColourRGB(seg.rl + (seg.rr - seg.rl) * f,
                        seg.gl + (seg.gr - seg.gl) * f,
                        seg.bl + (seg.br - seg.bl) * f);

    else if (seg.space == 1 || seg.space == 2)
    {
        // we've been thinking of the numbers so far as RGB, but these ones
        // are HSV or HSL. Convert them, fixup and convert back before
        // returning.
        //    hl, sl, vl = colorsys.rgb_to_hsv(seg.rl, seg.gl, seg.bl);
        //    hr, sr, vr = colorsys.rgb_to_hsv(seg.rr, seg.gr, seg.br);
        GColourHSV hsvL(seg.rl, seg.gl, seg.bl);
        GColourHSV hsvR(seg.rr, seg.gr, seg.br);

        if (seg.space == 1 && hsvR.H() < hsvL.H())
            hsvR.setH(hsvR.H() + 1);
        else if (seg.space == 2 && hsvR.H() > hsvL.H())
            hsvR.setH(hsvR.H() - 1);

        //    return new GColourRGB colorsys.hsv_to_rgb( (hl + (hr-hl) * f) % 1.0,
        //                                        sl + (sr-sl) * f,  vl + (vr-vl) * f    );
        GColourRGB col = GColourRGB::FromHSV(fmod((hsvL.H() + (hsvR.H() - hsvL.H()) * f), 1.0),
                                    hsvL.S() + (hsvR.S() - hsvL.S()) * f,
                                    hsvL.V() + (hsvR.V() - hsvL.V()) * f);
		
		return new GColourRGB(col);
    }
    else
        return new GColourRGB();
}
