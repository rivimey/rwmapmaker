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
#include "rwtexture.h"


uint qHash(const QSize &s)
{
	return qHash(s.height()) ^ qHash(s.width());
}

RWTexture::RWTexture(QString n, QSize sz)
	: texName(n), baseSz(sz)
{
}

RWTexture::~RWTexture()
{
}

QPixmap *RWTexture::at(int i)
{
	return texMap.values().at(i);
}

QPixmap *RWTexture::findMatch(QSize s)
{
	return texMap.find(s).value();
}

QPixmap *RWTexture::findLarger(QSize s)
{
	QPixmap *result = NULL;
	RWTexImageIterator it(texMap);
	while(it.hasNext())
	{
		it.next();
		if (it.key().isNull())
			continue;

		if (it.key().width() > s.width())
		{
			if (result == NULL || (result && result->width() < it.key().width()))
				result = it.value();
		}
	}
	return result;
}

void RWTexture::add(QSize s, QPixmap *i)
{
	texMap.insert(s, i);
}
