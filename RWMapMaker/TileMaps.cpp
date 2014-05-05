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
#include <QtCore/QRectF>
#include <QtCore/QPoint>
#include <QtGui/QImage>
#include <QtGui/QPixmap>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>

#include "TileMaps.h"

void deleteTerrainTileList(TerrainTileList *tiles)
{
	if (tiles == NULL)
		return;

	for (TerrainTileList::const_iterator it = tiles->begin(); it != tiles->end(); ++it)
    {
		TerrainTile *tile = *it;
		delete tile;
	}
	tiles->clear();
}

TerrainTile::TerrainTile() : pos(), img(NULL)
{ }

TerrainTile::~TerrainTile()
{
    delete pix;
    pix = NULL;
}

TerrainTile& TerrainTile::operator=( TerrainTile& rhs )
{
    delete pix;				// ensure any current img is not leaked
	pix = rhs.pix;
	pos = rhs.pos;
	return *this;
}

TerrainTile::TerrainTile(QPoint p, QPixmap *i) : pos(p), pix(i), img(0)
{
}

TerrainTile::TerrainTile(QPoint p, QImage *i) : pos(p), pix(0), img(i)
{
}
    
QString TerrainTile::toString() const
{
	return QString("Tile (%1, %2) pix %3 img %3")
					.arg(pos.x())
					.arg(pos.y())
			        .arg((long)(void*)pix)
			        .arg((long)(void*)img);
}

bool TerrainTile::convertImage()
{
	bool b = true;
	if (pix == 0 && img != 0)
	{
		pix = new QPixmap();
		b = pix->convertFromImage(*img);
		if (!b)
		{
			//qxtLog->error() << "TerrainTile::convertImage: Failed to convert tile image to pixmap.";
			delete pix;
			pix = NULL;
		}
		else
		{
			delete img;
			img = NULL;
		}
	}
	return b;
}

QPoint TerrainTile::getPosition()
{
    return pos;
}
    
QPixmap *TerrainTile::getImage()
{
    return pix;
}



//
// Container for info relating to the tile extents.
// The image contained here is a reference, and should
// not be copied deeply.
TileExtentMap::TileExtentMap() : numtiles(0), img( NULL)
{ }

TileExtentMap::~TileExtentMap()
{
	//if (img != NULL)
	//	OutputDebugStringA(QString("Warning: deleting image at 0x%1 in TileExtentMap\n").arg((ulong)img, 0, 16).toLocal8Bit().constData());
    delete img;
	img = NULL;
}

TileExtentMap& TileExtentMap::operator=(const TileExtentMap& rhs )
{
	numtiles = rhs.numtiles;
	region = rhs.region;
	if (img != NULL) 
		OutputDebugStringA(QString("Warning: overwriting image at 0x%1 in TileExtentMap\n").arg((ulong)img, 0, 16).toLocal8Bit().constData());
    delete img;				// ensure any current img is not leaked
	img = rhs.img;
	//assert(_CrtIsValidHeapPointer(img));
	return *this;
}

TileExtentMap::TileExtentMap(int num, QRectF r, QImage *i) : numtiles(num), region(r), img(i)
{
	//assert(_CrtIsValidHeapPointer(img));
	//if (img != NULL) 
	//	OutputDebugStringA(QString("Info: Image at %1 in New TileExtentMap\n").arg((ulong)img, 0, 16).toLocal8Bit().constData());
}
    
QString TileExtentMap::toString() const
{
	return QString("Tile Extent (#%1, @%2/%3, %4x%5) %6")
					.arg(numtiles)
					.arg(region.x())
					.arg(region.y())
					.arg(region.width())
					.arg(region.height())
			        .arg((long)(void*)img);
}

QRectF TileExtentMap::getRegion()
{
    return region;
}
    
QImage* TileExtentMap::getImage()
{
    return img;
}
    
int TileExtentMap::getCount()
{
    return numtiles;
}
