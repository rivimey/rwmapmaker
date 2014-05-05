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

#ifndef RWPOSITION_H
#define RWPOSITION_H

#include <QtCore/QString>

#include "Settings.h"

class RWCoordinate
{
    long route;
    double tile;
	bool valid;

public:
	RWCoordinate() : route(0), tile(0), valid(false) { }

    RWCoordinate(QString &r, QString &t)
    {
		route = r.toDouble();
		tile = t.toDouble();
		valid = route < 2000 && tile <= 1024;
		if (!valid)
			valid = false;
    }

    RWCoordinate(double p)
    {
		setLoc(p);
	}

    RWCoordinate(long r, double t): route(r), tile(t)
	{
		valid = route < 2000 && tile <= 1024;
		if (!valid)
			valid = false;
	}

	bool isValid() const
	{
		return valid;
	}

	/// <summary>
	/// Return a QString represention
	/// </summary>
	QString toString() const
	{
	    return QString("%1+%2").arg(route)
			                   .arg(tile);
	}
    
	long getRoute() const
    {
		return route;
	}
	void setRoute(long value)
    {
		route = value; 
    }

    double getTile() const
    {
        return tile;
	}
    void setTile(double value)
    {
		tile = value;
		valid = true;
    }

    double getLoc() const
    {
        return route * TILE_SIZE_METRES + tile;
    }

	void setLoc(double p)
	{
		route = (int)(p / TILE_SIZE_METRES);
		tile = p - (route * TILE_SIZE_METRES);
		if (tile < 0)
		{
			route --;
			tile = TILE_SIZE_METRES + tile;
		}
		valid = true;
	}
};

class RWPosition
{
    RWCoordinate Xp;
    RWCoordinate Zp;
	bool valid;

public:
	RWPosition() { valid = false; }

    RWPosition(RWCoordinate x, RWCoordinate z)
    {
        Xp = x;
        Zp = z;
		valid = true;
    }

	bool isValid() const
	{
		return valid && Xp.isValid() && Zp.isValid();
	}
    
	/// <summary>
	/// Return a QString represention
	/// </summary>
	QString toString() const
	{
	    return QString("X %1, Z %2").arg(Xp.toString())
			                        .arg(Zp.toString());
	}
    
	RWCoordinate getX() const
    {
        return Xp;
	}
	void setX(RWCoordinate value)
    {
		Xp = value; 
    }
    
	RWCoordinate getZ() const
    {
        return Zp;
	}
	void setZ(RWCoordinate value)
    {
		Zp = value; 
    }

	QPoint getRoute() const
	{
		return QPoint(Xp.getRoute(), Zp.getRoute());
	}

	QPointF getTile() const
	{
		return QPointF(Xp.getTile(), Zp.getTile());
	}
};

#endif
