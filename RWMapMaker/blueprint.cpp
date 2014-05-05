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

#include "blueprint.h"

void deleteBPMetadataList(BPMetadataList *metadata)
{
	if (metadata == NULL)
		return;

	for (BPMetadataList::const_iterator it = metadata->begin(); it != metadata->end(); ++it)
    {
		BPMetadata *d = *it;
		delete d;
	}
}


//
// Container for info relating to the tile extents.
// The image contained here is a reference, and should
// not be copied deeply.
RouteMetadata::RouteMetadata()
	: BPMetadata(BPClass_Route), guid(), textureName(), filter(0), textureCoords()
{ }

RouteMetadata::~RouteMetadata()
{
}

RouteMetadata::RouteMetadata(QString nm, QString gid, QString tex, BrowseFilter f)
	: BPMetadata(BPClass_Route, nm), guid(gid), textureName(tex), filter(f)
{
}
    
QString RouteMetadata::toString() const
{
	return QString("RouteMetadata %1 : %2 => %3")
					.arg(getName())
					.arg(getGUID())
					.arg(getTextureName()) ;
}

QString RouteMetadata::getImageFilename(QString routeFolder)
{
	if (textureName.isEmpty())
		return textureName;

	QString filename = routeFolder + "/" + textureName;
	QFileInfo file(filename);
	filename = file.fileName();
	if (filename.startsWith("[00]"))
		filename = filename.mid(4);		// ignore filename 'style'
	filename = file.absolutePath() + "/" + filename + ".TgPcDx";

	return filename;
}

QImage* RouteMetadata::getImage(QString routeFolder)
{
	QString filename = getImageFilename(routeFolder);
	return NULL;
}
