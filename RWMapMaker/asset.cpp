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
#include "asset.h"
#include "qxtlogger.h"

static const QString catnames[] = {
	"eBridgesViaductsTunnels",
	"eClutter",
	"eDomesticBuildings",
	"eFoliage",
	"eFoliageWater",
	"eGenericBuildings",
	"eMisc",
	"eNone",
	"ePeople",
	"ePeopleAnimalsVehicles",
	"eRoads",
	"eSignage",
	"eSound",
};

void deleteAssetDict(AssetDict *items)
{
	if (items == NULL)
		return;

	for (AssetDict::const_iterator it = items->begin(); it != items->end(); ++it)
    {
		Asset *item = *it;
		delete item;
	}
}

Asset::Asset()
	: inscenarios(false), pickable(false), instancable(false),
	  cat(eNone), shadow(eShadowType_None), view(eView_External)
{
}

Asset::Asset(AssetCategory c, QString disp, QString geom, QString mach)
	: inscenarios(false), pickable(false), instancable(false),
	  cat(c), shadow(eShadowType_None), view(eView_External),
	  geometry(geom), displayname_en(disp), machinename(mach)
{
}

Asset::~Asset(void)
{
}


QString Asset::toString(Asset::AssetCategory cat)
{
	int c;
	if (c >= Asset::eNone && c <= Asset::eSound)
		return QString(catnames[(int)c]);
	else
		return QString("?");
}

Asset::AssetCategory Asset::fromString(QString key)
{
	int nelem = sizeof(catnames) / sizeof(catnames[0]);
	int upperbound = nelem-1, lowerbound = 0;
	int position;

	// To start, find the subscript of the middle position.
	position = ( lowerbound + upperbound) / 2;
	while((catnames[position] != key) && (lowerbound <= upperbound))
	{
		if (catnames[position] > key)     // If the number is > key, ..
			upperbound = position - 1;    // decrease position by one.
		else                                                
			lowerbound = position + 1;    // Else, increase position by one.
		position = (lowerbound + upperbound) / 2;
	}
	if (lowerbound <= upperbound)
		return (Asset::AssetCategory)position;
	else
		return (Asset::AssetCategory)eNone;
}
