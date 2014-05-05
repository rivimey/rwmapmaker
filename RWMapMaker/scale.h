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

#ifndef QSCALE_H
#define QSCALE_H

class QScale
{
protected:
    double sx;
	double sy;
	bool valid;

public:
    QScale()
    {
        valid = false;
    }

    QScale(double s)
    {
        valid = true;
        sx = s;
        sy = s;
    }

    QScale(double x, double y)
    {
        valid = true;
        sx = x;
        sy = y;
    }

    bool isValid()
    {
        return valid;
    }

    double SX()
    {
        return sx; 
    }
    void setSX(double value)
    {
        sx = value;
    }
    double SY()
    {
        return sy;
    }
    void setSY(double value)
    {
        sy = value; 
    }
};

#endif
