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

#ifndef PAIR_H
#define PAIR_H

class Pair
{
protected:
    double a;
	double b;
	bool valid;

public:
    Pair()
    {
        valid = false;
    }

    Pair(double A, double B)
    {
        valid = true;
        a = A;
        b = B;
    }

    bool isValid()
    {
        return valid;
    }

    double A()
    {
        return a; 
    }
    void setA(double value)
    {
        a = value;
    }
    double B()
    {
        return b;
    }
    void setB(double value)
    {
        b = value; 
    }
};
#endif
