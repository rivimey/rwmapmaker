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

#ifndef GRADGGR_H
#define GRADGGR_H

#include "Settings.h"
#include "GColour.h"
#include <QtCore/QString>
#include <QtCore/QList>


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

struct GGRSegment
{
	double l;  // pos of left end
	double m;  // pos of marker
	double r;  // pos of right end

	double rl; // Red component, Left
	double gl; // Green
	double bl; // Blue
	double al; // Alpha (ignored)

	double rr; // Red component, Right
	double gr; // Green
	double br; // Blue
	double ar; // Alpha (ignored)

	int fn;    // Function 1..4
	int space; // color space. 0 == RGB, 1 == HSV/L, 2 == HSV/L
};

typedef QList<GGRSegment> GGRSegmentList;

class GimpGradient
{
    /*!
        Private Class to hold the values read from each line of the GIMP gradient definition.
        Note: there are two values ignored (appearing after each colour spec, for Alpha?),
        and can be 2 more values appended in some variations of the file.
	 */
private:
    QString name;
    GGRSegmentList *segs;
    int nsegs;
	bool isvalid;

    /*!
		Read and interpret a Gimp .ggr gradient file from the open stream.
		Do nothing if this is NULL; call .read() to read the file.
	 */
public:
	GimpGradient() : segs(0), nsegs(0), isvalid(false) {};

    /*!
        Read and interpret a Gimp .ggr gradient file.
        Exceptions are caught and ignored.
        \a name Path name of the GGR file to read
	 */
	GimpGradient(QTextStream &fs);

    /*!
        Read and interpret a Gimp .ggr gradient file.
        Exceptions are caught and ignored.
        \a name Path name of the GGR file to read
	 */
    GimpGradient(const QString &name);

    /*!
		Return the internal name of the file
	 */
	QString internalName() const { return name; }

    /*!
		Return the number of segments defined
	 */
	int nsegments() const { return nsegs; }
	GGRSegmentList &segments() { return *segs; }

	~GimpGradient()
	{
		delete segs;
	}

	bool isValid() const { return isvalid; }

    /*!
		Read and interpret a Gimp .ggr gradient file.
	 */
    void read(const QString &filename);
    void read(QTextStream &f);
	void write(const QString &filename);
	void write(QTextStream &f);

    GColourBase *color(double x);
};

#endif
