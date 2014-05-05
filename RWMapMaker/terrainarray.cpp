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

#include <stdafx.h>
#include <assert.h>

#include "qxtlogger.h"
#include "terrainarray.h"

void deleteTerrainArrayList(TerrainArrayList *tiles)
{
	if (tiles == NULL)
		return;

    // for reasons unknown, the angle specified seems to be normal to the direction of the track. pi/2 is 90 degrees.
	//assert(_CrtIsValidHeapPointer(tiles));

	for (TerrainArrayList::const_iterator it = tiles->begin(); it != tiles->end(); ++it)
    {
		TerrainArray *tile = *it;
		//assert(_CrtIsValidHeapPointer(tile));
		delete tile;
	}
	tiles->clear();
}

TerrainArray::~TerrainArray()
{
	if (isValid)
		delete mem;
}

TerrainArray::TerrainArray(QPoint p, int x, int y)
	: pos(p), width(x), height(y)
{
	if (x > 0 && y > 0)
		mem = new HVal[x*y];
	else
		mem = NULL;
	isValid = (mem != NULL);
}

HVal &TerrainArray::at(int x, int y)
{
	HVal *a;
#ifndef NDEBUG
	if (check(x,y))
		a = mem + addr(x, y);
	else
	{
		a = mem;
		qxtLog->fatal() << "TerrainArray::at check failed: " << x << " x " << y;
	}
#else
	a = mem + addr(x, y);
#endif
	return *a;
}

