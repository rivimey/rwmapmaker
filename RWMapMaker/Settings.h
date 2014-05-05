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

#ifndef SETTINGS_H
#define SETTINGS_H

class LoadingDialog;
class GimpGradient;

#define PROGRAM_NAME	"TSMapMaker"
#define PROGRAM_VERSION "1.1 Beta2"
#define PROGRAM_NAME_VER  PROGRAM_NAME " " PROGRAM_VERSION

#define PROGRAM_ID		(4)
#define PROGRAM_VER		(9)

#define FEATURE_SAVE_EXCEL		(0x001)
#define FEATURE_SAVE_IMAGES		(0x002)
#define FEATURE_NONSTD_PATH		(0x004)
#define FEATURE_BROWSEALL		(0x008)
//#define FEATURE_		(0x010)

// size of a tile side in metres
#define TILE_SIZE_METRES 1024
#define TILE_SIZE_TERRAIN 128

// size of bitmap (both x and y) used to record route tiles => +/- 512 tiles from route origin.
#define EXTENT_BITMAP_SIZE 1024

// The amount by which zoomIn(), zoomOut() change the scale. = 1/2
#define SCALE_FACTOR		(0.5)

// The amount by which Wheel events change the scale. == 7/8
#define WHEEL_SCALE_FACTOR	(0.875)
#define INITIAL_SCALE 0.07

// the palette index, and colour, used to represent Tile and No-Tile in the bitmaps.
#define INDEX_TILE 1
#define INDEX_NOTILE 0

// the size of the scale overlay
#define SCALE_WIDTH	400
#define SCALE_HEIGHT 52
#define SCALE_OPACITY 50
#define SCALE_KM_BREAK 1020		/* point at which scale goes to KM */

// the number of curve segments in a net curve per metre. range 4 to 0.25
#define CURVE_FACTOR 3.0

enum TileType
{
    TILETYPE_TERRAIN,
    TILETYPE_TRACK,
    TILETYPE_ROAD,
    TILETYPE_SCENERY
};

#endif
