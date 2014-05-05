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

#include "ColourMapper.h"
#include "GradGGR.h"
#include <assert.h>

ColourMapper::ColourMapper(GimpGradient *ggr)
	: SIZE(1800)
{
    if (ggr != NULL)
	{
		colourmap = new QColor[SIZE];
		for (double v = 0; v < SIZE; v++)
		{
			GColourBase *gcb = ggr->color(v / (double)SIZE);
			assert(gcb->ColourModel() == GColourBase::RGB);
			GColourRGB *rgb = (GColourRGB*)(gcb);

			QColor &colour = QColor(Math::roundint(rgb->R() * 255),
									Math::roundint(rgb->G() * 255),
									Math::roundint(rgb->B() * 255));
			colourmap[(int)v] = colour;

			delete rgb;
		}
	    minH = -25;
		maxH = 1000;
	    setFactor();
	}
}

ColourMapper::~ColourMapper()
{
	delete [] colourmap;
}

/*!
    Get the lowest height (metres)
 */
double ColourMapper::minHeight()
{
    return minH;
}

/*!
    Get the highest height (metres)
 */
double ColourMapper::maxHeight()
{
    return maxH;
}

/*!
    The lowest height (metres) the gradient will be mapped to
 */
void ColourMapper::setMinHeight(double h)
{
    minH = h;
    setFactor();
}


/*!
    The highest height (metres) the gradient will be mapped to
 */
void ColourMapper::setMaxHeight(double h)
{
    maxH = h;
    setFactor();
}

void ColourMapper::setFactor()
{
    // SIZE-1 is to allow for rounding errors.
    factor = (SIZE - 1) / (maxH - minH);
}

/*!
    Return the colour that a terrain height of 'v' should be mapped to.
 */
QColor & ColourMapper::mapToColour(double v)
{
    if (v >= minH && v < maxH)
    {
        double w = (v - minH) * factor;
        return colourmap[(int)w];
    }
    else if (v < minH)
        return colourmap[0];
    else //if (v >= maxH)
        return colourmap[SIZE - 1];
}

