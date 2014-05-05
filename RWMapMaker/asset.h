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

#ifndef ASSET_H
#define ASSET_H

#include <QtCore/QString>

class Asset
{
public:
	enum AssetCategory		// must be synced with catnames[] and kept storted
	{
		eBridgesViaductsTunnels,
		eClutter,
		eDomesticBuildings,
		eFoliage,
		eFoliageWater,
		eGenericBuildings,
		eMisc,
		eNone,
		ePeople,
		ePeopleAnimalsVehicles,
		eRoads,
		eSignage,
		eSound,
	};
	enum ShadowType
	{
		eShadowType_None,
		eShadowType_Blobby
	};
	enum ViewType
	{
		eView_Cab,
		eView_External,
	};

private:
	QString texset_pri;  // PrimaryNamedTextureSet
	QString texset_sec;
	QString machinename;
	QString displayname_en;
	QString description_en;
	QString geometry;
	QString collisiongeometry;
	
	bool inscenarios;
	bool pickable;
	bool instancable;

	AssetCategory cat;
	ShadowType shadow;
	ViewType view;

	int mindetail;
	int maxdetail;

public:
	Asset();
	Asset(AssetCategory cat, QString display, QString geom, QString machine = QString());
	~Asset();

	QString getMachineName() { return machinename; }
	void setMachineName(QString n) { machinename = n; }

	QString getDisplayName() { return displayname_en; }
	void setDisplayName(QString n) { displayname_en = n; }

	QString getGeometry() { return geometry; }
	void setGeometry(QString bp) { geometry = bp; }

	QString getCollisionGeometry() { return collisiongeometry; }
	void setCollisionGeometry(QString c) { collisiongeometry = c; }

	bool getValidScenarios() { return inscenarios; }
	void setValidScenarios(bool v) { inscenarios = v; }

	bool getPickable() { return pickable; }
	void setPickable(bool d) { pickable = d; }

	bool getInstancable() { return instancable; }
	void setInstancable(bool d) { instancable = d; }

	AssetCategory getCategory() { return cat; }
	void setCategory(AssetCategory c) { cat = c; }

	ShadowType getShadowType() { return shadow; }
	void setShadowType(ShadowType s) { shadow = s; }

	ViewType getViewType() { return view; }
	void setViewType(ViewType v) { view = v; }

	static AssetCategory fromString(QString cat);
	static QString toString(AssetCategory cat);
};

typedef QHash<QString, Asset*> AssetDict;
void deleteAssetDict(AssetDict *items);

#endif
