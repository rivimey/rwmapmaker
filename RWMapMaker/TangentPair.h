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

#ifndef TANGENTPAIR_H
#define TANGENTPAIR_H

class TangentPair
{
protected:
    double t1;
	double t2;
	bool valid;

public:
    TangentPair()
    {
        valid = false;
    }

    TangentPair(double Tx, double Ty)
    {
        valid = true;
        t1 = Tx;
        t2 = Ty;
    }

    bool isValid()
    {
        return valid;
    }

    double Tx()
    {
        return t1; 
    }
    void setTx(double value)
    {
        t1 = value;
    }
    double Ty()
    {
        return t2;
    }
    void setTy(double value)
    {
        t2 = value; 
    }

	double arctan2()
	{
		return atan2(t1, t2);
	}
};
#endif
