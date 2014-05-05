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

#include "StdAfx.h"
#include "sceneryitem.h"
#include "qxtlogger.h"

void deleteSceneryItemList(SceneryItemList *items)
{
	if (items == NULL)
		return;

	//assert(_CrtIsValidHeapPointer(items));

	for (SceneryItemList::const_iterator it = items->begin(); it != items->end(); ++it)
    {
		SceneryItem *item = *it;
		//assert(_CrtIsValidHeapPointer(item));
		delete item;
	}
}

SceneryItem::SceneryItem()
	: detail(0), location(), blueprint(), height(0), cat(SceneryItem::Other)
{
}

SceneryItem::SceneryItem(QString bp, RWPosition coord, double h)
	: blueprint(bp), detail(0), location(coord), height(h)
{
	cat = guessCategory();
}

SceneryItem::~SceneryItem(void)
{
}

/*
	Return the within-tile X coordinate of the start of the curve
*/
double SceneryItem::getTx()
{
    return location.getX().getTile();
}

/*
	Return the X coordinate in the route of the start of the curve
*/
double SceneryItem::getRx()
{
    return location.getX().getRoute();
}
    
/*
	Return the within-tile Y(Z) coordinate of the start of the curve
*/
double SceneryItem::getTz()
{
    return location.getZ().getTile();
}
    
/*
	Return the Y(Z) coordinate in the route of the start of the curve
*/
double SceneryItem::getRz()
{
    return location.getZ().getRoute();
}
    
/*
	Return the X coordinate of the point in global terms.
*/
double SceneryItem::getX()
{
    return getTx() + TILE_SIZE_METRES * getRx();
}

/*
	Return the Y (Z) coordinate of the point in global terms.
*/
double SceneryItem::getZ()
{
    return getTz() + TILE_SIZE_METRES * getRz();
}

SceneryItem::ScenicCategory SceneryItem::guessCategory()
{
	if (blueprint.isEmpty())
		return SceneryItem::Other;

	if (blueprint.contains("signal", Qt::CaseInsensitive) ||
	    blueprint.contains("platform", Qt::CaseInsensitive) ||
	    blueprint.contains("gantries", Qt::CaseInsensitive) ||
	    blueprint.contains("speedsign", Qt::CaseInsensitive) ||
	    blueprint.contains("buffer", Qt::CaseInsensitive) ||
	    blueprint.contains("billboard", Qt::CaseInsensitive) ||
	    blueprint.contains("levelcrossings", Qt::CaseInsensitive) ||
	    blueprint.contains("station", Qt::CaseInsensitive) ||
	    blueprint.contains("clock", Qt::CaseInsensitive) ||
	    blueprint.contains("hut", Qt::CaseInsensitive))
		return SceneryItem::Lineside;

	if (blueprint.contains("factory", Qt::CaseInsensitive) ||
	    blueprint.contains("shop", Qt::CaseInsensitive) ||
	    blueprint.contains("block", Qt::CaseInsensitive))
		return SceneryItem::Factory;

	if (blueprint.contains("water", Qt::CaseInsensitive) ||
	    blueprint.contains("river", Qt::CaseInsensitive) ||
	    blueprint.contains("sea", Qt::CaseInsensitive))
		return SceneryItem::Water;

	if (blueprint.contains("clutter", Qt::CaseInsensitive) ||
	    blueprint.contains("particle", Qt::CaseInsensitive) ||
	    blueprint.contains("lights", Qt::CaseInsensitive) ||
	    blueprint.contains("trolley", Qt::CaseInsensitive))
		return SceneryItem::Clutter;

	if (blueprint.contains("building", Qt::CaseInsensitive) ||
	    blueprint.contains("structure", Qt::CaseInsensitive) ||
	    blueprint.contains("tunnel", Qt::CaseInsensitive) ||
	    blueprint.contains("bridge", Qt::CaseInsensitive) ||
	    blueprint.contains("domestic", Qt::CaseInsensitive) ||
	    blueprint.contains("house", Qt::CaseInsensitive)
		)
		return SceneryItem::Building;
	
	if (blueprint.contains("vehicle", Qt::CaseInsensitive) ||
	    blueprint.contains("car", Qt::CaseInsensitive) ||
	    blueprint.contains("truck", Qt::CaseInsensitive) ||
	    blueprint.contains("lorry", Qt::CaseInsensitive)
		)
		return SceneryItem::Vehicle;
	
	if (blueprint.contains("decal", Qt::CaseInsensitive)
	    )
		return SceneryItem::Decal;
	
	if (blueprint.contains("audio", Qt::CaseInsensitive) ||
	    blueprint.contains("ambient", Qt::CaseInsensitive) ||
	    blueprint.contains("sound", Qt::CaseInsensitive)
		)
		return SceneryItem::Sound;

	if (blueprint.contains("character", Qt::CaseInsensitive) ||
	    blueprint.contains("person", Qt::CaseInsensitive) ||
	    blueprint.contains("wildlife", Qt::CaseInsensitive)
		)
		return SceneryItem::Animals;
	
	
	if (blueprint.contains("tree", Qt::CaseInsensitive) ||
	    blueprint.contains("vegetation", Qt::CaseInsensitive) ||
	    blueprint.contains("vegitation", Qt::CaseInsensitive) ||
	    blueprint.contains("grass", Qt::CaseInsensitive) ||
	    blueprint.contains("foliage", Qt::CaseInsensitive))
		return SceneryItem::Plant;

	qxtLog->debug() << "Didn't classify asset: " << blueprint;

	return SceneryItem::Other;
}


QString SceneryItem::toString(SceneryItem::ScenicCategory c)
{
	static const char *text[] = {	
		"Water",
		"Building",
		"Factory",
		"Lineside",
		"Vehicle",
		"Plant",
		"Clutter",
		"Tunnel",
		"Decal",
		"Sound",
		"Animals",
		"Other"
	};

	if (c >= SceneryItem::Water && c <= SceneryItem::Other)
		return QString(text[(int)c]);
	else
		return QString("?");
}

QSize SceneryItem::toSize(SceneryItem::ScenicCategory c)
{
	static const QSize sizes[] = {	
		QSize(9,9),			// Water
		QSize(14,14),		// Building
		QSize(18,18),		// Factory
		QSize(4,4),			// Lineside
		QSize(3,3),			// Vehicle
		QSize(8,8),			// Plant
		QSize(3,3),			// Clutter
		QSize(10,10),		// Tunnel
		QSize(8,8),			// Decal
		QSize(2,2),			// Sound
		QSize(2,2),			// Animals
		QSize(2,2),			// Other
	};

	if (c >= SceneryItem::Water && c <= SceneryItem::Other)
		return QSize(sizes[(int)c]);
	else
		return QSize();
}

QColor SceneryItem::toColor(SceneryItem::ScenicCategory c)
{
	static const QColor colours[] = {	
		QColor("blue"),			// Water
		QColor(130,117,63),		// Building = brown
		QColor(99,99,99),		// Factory = grey
		QColor("black"),		// Lineside
		QColor(255,65,40),		// Vehicle = red
		QColor(67,100,0),		// Plant = khaki green
		QColor(86,67,29),		// Clutter
		QColor("black"),		// Tunnel
		QColor(138,114,72),		// Decal = mud
		QColor("black"),		// Sound
		QColor(255,221,245),	// Animals = pink
		QColor(48,48,48,.5),	// Other == transparent grey
	};

	if (c >= SceneryItem::Water && c <= SceneryItem::Other)
		return QColor(colours[(int)c]);
	else
		return QColor();
}
