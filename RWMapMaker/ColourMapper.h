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

#ifndef COLOURMAPPER_H
#define COLOURMAPPER_H

#include <QtGui/QColor>
class GimpGradient;

class ColourMapper
{
    // SIZE is mostly determined by the number of actual colours in the gradient...
    const int SIZE;
    QColor *colourmap;
    double minH;
    double maxH;
    double factor;

public:
	ColourMapper(GimpGradient *ggr);
	~ColourMapper();

    /// <summary>
    /// Get the lowest height (metres)
    /// </summary>
    double minHeight();

    /// <summary>
    /// Get the highest height (metres)
    /// </summary>
    double maxHeight();

    /// <summary>
    /// The lowest height (metres) the gradient will be mapped to
    /// </summary>
    void setMinHeight(double h);

    /// <summary>
    /// The highest height (metres) the gradient will be mapped to
    /// </summary>
    void setMaxHeight(double h);

    void setFactor();

    /// <summary>
    /// Return the colour that a terrain height of 'v' should be mapped to.
    /// </summary>
    QColor & mapToColour(double v);
};

#endif
