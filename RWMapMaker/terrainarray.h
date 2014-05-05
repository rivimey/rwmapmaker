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

#ifndef TERRAINARRAY_H
#define TERRAINARRAY_H

#include "Settings.h"

typedef short HVal;

#define HVAL_BITS	3
#define HVAL_SCALE	(1<<HVAL_BITS)
#define HVAL_ROUND	(0.0625)
#define HVAL_MAXVAL (4095)
#define HVAL_MINVAL (-4095)

inline HVal hvalFromInt(int x)
{
	return (HVal)(x << HVAL_BITS);
}

inline int intFromHval(HVal x)
{
	return (int)(x >> HVAL_BITS);
}

inline HVal hvalFromDouble(double x)
{
	// saturate to limits
	if (x > HVAL_MAXVAL)
		x = HVAL_MAXVAL;
	else if (x < HVAL_MINVAL)
		x = HVAL_MINVAL;

	// round nearest and scale.
	HVal v = (HVal)floor((x + HVAL_ROUND) * HVAL_SCALE);
	return v;
}

inline double doubleFromHval(HVal x)
{
	return (double)(x / HVAL_SCALE);
}

inline HVal hvalFromFloat(float x)
{
	// saturate to limits
	if (x > HVAL_MAXVAL)
		x = HVAL_MAXVAL;
	else if (x < HVAL_MINVAL)
		x = HVAL_MINVAL;

	// round nearest and scale.
	short v = (HVal)floor((x + HVAL_ROUND) * HVAL_SCALE);
	return v;
}

inline float floatFromHval(HVal x)
{
	float v = (float)(x / HVAL_SCALE);
	return v;
}


class TerrainArray
{
private:
    QPoint pos;
	bool isValid;
	HVal *mem;
	int width;
	int height;

private:
	// array address calculation... keep in one place.
	inline int addr(int x, int y) const
	{
		return (x * width + y);
	}

public:
	/*!
		Create an empty/null tile height array. 
	*/
	TerrainArray(): isValid(false), mem(NULL) { };

	/*!
		Create a tile height array for the tile at address \a pos
		and that is \a sx, \a sy points in size.
	*/
	TerrainArray(QPoint pos, int sx, int sy);

	/*!
		Delete the tile array resources.
	*/
	~TerrainArray();

	/*!
		Return a reference to the height of terrain at location \a x, \a y
		Bounds are checked in Debug builds by calling check().
	*/
	HVal &at(int x, int y);

	/*!
		Return a reference to the height of terrain at location \a p
		[calls HVal &at(int x, int y) ]. Bounds are checked in Debug
		builds by calling check().
	*/
	HVal &at(const QPoint &p) { return at(p.x(), p.y()); }

	/*!
		Fetch the height of terrain at location \a x, \a y (no bounds checking).
	*/
	inline HVal at(int x, int y) const
			{ HVal *a = mem + addr(x, y); return *a; }

	/*!
		Fetch the height of terrain at location \a p (no bounds checking).
	*/
	inline HVal at(const QPoint &p) const { return at(p.x(), p.y()); }

	/*!
		Check that coordinate \a x , \a y are valid for this tile-height array.
	*/
	inline bool check(int x, int y)
			{ return (isValid) &&  (x >= 0 && x < width) && (y >= 0 && y < height); }

	/*!
		Check that the tile is validly constructed.
	*/
	inline bool tileIsValid(int x, int y)
			{ return isValid && width > 0 && height > 0; }

	/*!
		Set the tile grid address as an XY coordinate based at the map orgin.
		[Within a tile the bottom-left point is the orgin]
	*/
	void setPosition(QPoint p) { pos = p; }

	/*!
		Get the tile grid address as an XY coordinate based at the map orgin.
		[Within a tile the bottom-left point is the orgin]
	*/
	QPoint getPosition() const { return pos; }
	
	/*!
	   Convert location within tile expressed in metres from origin to a tile
	   coordinate value.
	*/
	static QPoint fromMetres(QPointF p) 
	{
		return QPoint(p.x() * TILE_SIZE_TERRAIN / TILE_SIZE_METRES,
		              p.y() * TILE_SIZE_TERRAIN / TILE_SIZE_METRES );
	}

	/*!
	   Convert location within tile expressed as a tile coordinate value
	   to metres from the tile origin
	*/
	static QPointF toMetres(QPoint p) 
	{
		return QPointF(p.x() * TILE_SIZE_METRES / TILE_SIZE_TERRAIN ,
		               p.y() * TILE_SIZE_METRES / TILE_SIZE_TERRAIN );
	}
};


typedef QHash<QPoint, TerrainArray*> TerrainArrayList;
void deleteTerrainArrayList(TerrainArrayList *tiles);

#endif
