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

#ifndef BLUEPRINT_H
#define BLUEPRINT_H

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QPoint>
#include <QtCore/QFlags>
#include <QtGui/QPixmap>

enum BPMetadataClass
{
	BPClass_None = 0,
	BPClass_Route = 1
};

class BPMetadata
{
	BPMetadataClass bpclass;
	QString name;

public:
	BPMetadata() : bpclass(BPClass_None) {};
	BPMetadata(BPMetadataClass bpc) : bpclass(bpc)  {};
	BPMetadata(BPMetadataClass bpc, QString n) : bpclass(bpc), name(n) { };
	virtual ~BPMetadata() {};

	virtual QString toString() const { return name; };

	virtual BPMetadataClass getClass() const { return bpclass; };
	virtual QString getName() const { return name; };
	virtual void setName(QString n) { name = n; };
};

//
// Container for info relating to the tile extents.
// The image contained here is a reference, and should
// not be copied deeply.
class RouteMetadata : public BPMetadata
{
public:
	enum BrowseFilterFlag
	{
		Region_Africa		= 0x0001,
		Region_Asia			= 0x0002,
		Region_Australia	= 0x0004,
		Region_Europe		= 0x0008,
		Region_NorthAmerica	= 0x0010,
		Region_SouthAmerica	= 0x0020,

		Era_Modern			= 0x0040,
		Era_Historic		= 0x0080,
		Era_Heritage		= 0x0100,
	};
	Q_DECLARE_FLAGS(BrowseFilter, BrowseFilterFlag);


private:
	QString guid;
	QString textureName;
	QRectF textureCoords;
	BrowseFilter filter;

public:
	RouteMetadata();

	virtual ~RouteMetadata();

    RouteMetadata(QString name, QString gid, QString tex, BrowseFilter f);
    
	QString toString() const;

	BrowseFilter getBrowseFilter() const { return filter; };
	void setBrowseFilter(BrowseFilter f) { filter = f; };

	QString getGUID() const { return guid; };
	void setGUID(QString n) { guid = n; };

	QString getTextureName() const { return textureName; };
	void setTextureName(QString n) { textureName = n; };

	QRectF getTextureCoords() const { return textureCoords; };
	void setTextureCoords(QRectF c) { textureCoords = c; };

	QImage* getImage(QString routeFolder);

public:
	QString getImageFilename(QString routeFolder);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(RouteMetadata::BrowseFilter);

typedef QList<BPMetadata*> BPMetadataList;
void deleteBPMetadataList(BPMetadataList *metadata);

#endif
