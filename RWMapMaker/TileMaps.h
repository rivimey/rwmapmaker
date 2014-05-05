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

#ifndef TILEMAPS_H
#define TILEMAPS_H

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QPoint>
#include <QtGui/QImage>
#include <QtGui/QPixmap>

class TerrainTile
{
    QPoint pos;
    QPixmap *pix;
    QImage *img;

public:
	TerrainTile();

	~TerrainTile();

	TerrainTile& operator=( TerrainTile& rhs );

    TerrainTile(QPoint p, QPixmap *i);

    TerrainTile(QPoint p, QImage *i);
    
	QString toString() const;

	QPoint getPosition();
    
	QPixmap *getImage();
	bool convertImage();
};


//
// Container for info relating to the tile extents.
// The image contained here is a reference, and should
// not be copied deeply.
class TileExtentMap
{
    int numtiles;
    QRectF region;
    QImage *img;

public:
	TileExtentMap();

	~TileExtentMap();

	TileExtentMap& operator=(const TileExtentMap& rhs );

    TileExtentMap(int num, QRectF r, QImage *i);
    
	QString toString() const;

	QRectF getRegion();
    
	QImage* getImage();
    
	int getCount();
};

typedef QList<TerrainTile*> TerrainTileList;
void deleteTerrainTileList(TerrainTileList *tiles);

#endif
