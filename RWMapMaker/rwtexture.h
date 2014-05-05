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

#ifndef RWTEXTURE_H
#define RWTEXTURE_H

#include <QtGui/QPixmap>
#include <QtCore/QList>
#include <QtCore/QMap>

typedef QHash<QSize, QPixmap *> RWTexImage;

class RWTexture
{
	RWTexImage texMap;
	QString texName;
	QSize baseSz;

public:
	RWTexture() {};
	RWTexture(QString name, QSize sz);
	~RWTexture();

	QPixmap *at(int);
	QPixmap *findMatch(QSize);
	QPixmap *findLarger(QSize);
	void add(QSize, QPixmap *);

	QString name() const { return texName; }
	QSize baseSize() const { return baseSz; }
	int texMips() const { return texMap.count(); }

private:
	
};

typedef QList<RWTexture> RWTextureList;
typedef QHashIterator<QSize, QPixmap *> RWTexImageIterator;

#endif // RWTEXTURE_H
