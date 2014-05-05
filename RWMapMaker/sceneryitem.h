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

#ifndef SCENERYITEM_H
#define SCENERYITEM_H

#include <QtCore/QString>
#include "rwposition.h"

class SceneryItem
{
public:
	enum ScenicCategory
	{	// keep in sync with toString()
		Water,
		Building,
		Factory,
		Lineside,
		Vehicle,
		Plant,
		Clutter,
		Tunnel,
		Decal,
		Sound,
		Animals,
		Other
	};

private:
	ScenicCategory cat;
	int detail;
	QString entityid;
	QString blueprint;
	QString reskinblueprint;
	RWPosition location;
	//matrix
	double height;
	double width;
	double length;

public:
	SceneryItem();
	SceneryItem(QString bp, RWPosition coord, double height);
	~SceneryItem();

	QString getID() { return entityid; }
	void setID(QString id) { entityid = id; }

	QString getBlueprint() { return blueprint; }
	void setBlueprint(QString bp) { blueprint = bp; }

	QString getReskinBlueprint() { return reskinblueprint; }
	void setReskinBlueprint(QString bp) { reskinblueprint = bp; }

	int getDetailLevel() { return detail; }
	void setDetailLevel(int d) { detail = d; }

	RWPosition getLocation() { return location; }
	void setLocation(RWPosition c) { location = c; }

	double getHeight() { return height; }
	void setHeight(double h) { height = h; }

	double getWidth() { return width; }
	void setWidth(double w) { width = w; }

	double getLength() { return length; }
	void setLength(double l) { length = l; }

	ScenicCategory getCategory() { return cat; }
	void setCategory(ScenicCategory c) { cat = c; }
    double getTx();
    double getRx();
    double getTz();
    double getRz();
    double getX();
    double getZ();

	static QString toString(ScenicCategory c);
	static QSize toSize(ScenicCategory c);
	static QColor toColor(ScenicCategory c);

private:
	ScenicCategory guessCategory();
};

typedef QList<SceneryItem*> SceneryItemList;
void deleteSceneryItemList(SceneryItemList *items);

#endif
