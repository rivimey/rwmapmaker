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

#include "RWMapMaker.h"
#include "Network.h"
#include "RWPosition.h"
#include <assert.h>

void deleteRibbonList(RibbonList *rib)
{
	if (rib == NULL)
		return;

	//assert(_CrtIsValidHeapPointer(net));
	for (RibbonList::const_iterator it = rib->begin(); it != rib->end(); ++it)
    {
		IRibbon *ribbon = *it;
		deleteNetworkList(ribbon->segments());
		delete ribbon;
	}
}

void deleteNetworkList(NetworkList *net)
{
	if (net == NULL)
		return;

	for (NetworkList::const_iterator it = net->begin(); it != net->end(); ++it)
    {
		INetwork *segment = *it;
		//assert(_CrtIsValidHeapPointer(segments));
		delete segment;
	}
}



TrackNetworkRibbon::TrackNetworkRibbon(QString &rid, QString &nid)
	: curves(new NetworkList()), ribid(rid), netid(nid)
{
}

TrackNetworkRibbon::~TrackNetworkRibbon()
{
	delete curves;
}
        
/*
	Return a QString represention of this ribbon
*/
QString TrackNetworkRibbon::toString()
{
    return QString("Track Ribbon %1").arg(ribid);
}
    
/*
	Get the ribbon id for this ribbon
*/
QString TrackNetworkRibbon::ribbonid()
{
    return ribid;
}
    
/*
	Get the network id for this ribbon
*/
QString TrackNetworkRibbon::networkid()
{
    return netid;
}

/*
	Return a list of curves in this ribbon
*/
NetworkList *TrackNetworkRibbon::segments()
{
    return curves;  // list of TrackNetworkRibbon
}
    
/*
	Add a road network curve to this ribbon
*/
void TrackNetworkRibbon::append(INetwork *curve)
{
    curves->append(curve);     // append NetworkCurve
}


/*
	
*/
RoadNetworkRibbon::RoadNetworkRibbon(QString &rid)
	: curves(new NetworkList()), ribid(rid)
{
}

RoadNetworkRibbon::~RoadNetworkRibbon()
{
	delete curves;
}

/*
	Return a QString represention of this ribbom
*/
QString RoadNetworkRibbon::toString()
{
    return QString("Road Ribbon %1").arg(ribid);
}
    
/*
	Get the ribbom id for this ribbon
*/
QString RoadNetworkRibbon::ribbonid()
{
    return ribid;
}
    
/*
	Return a list of curves in this ribbon
*/
NetworkList *RoadNetworkRibbon::segments()
{
    return curves;  // list of TrackNetworkRibbon
}
    
/*
	Add a road network curve to this ribbon
*/
void RoadNetworkRibbon::append(INetwork *curve)
{
    curves->append(curve);     // append NetworkCurve
}



/*
	Base class describing elements of an RW Network Ribbon.
	A Ribbon is a linear feature of the network and is split into
	curves and straight segments. Each such segment is described
	by one NetworkCurve object.
	Networks are spread across a grid; the route is anchored at an origin and
	is composed of grid squares (tiles). Networks are spread across a grid; tiles
	are the grid squares.
	The RW files refer to X and Z coordinates: I am assuming that X is East-West
	and Z is North-South, and will refer to X and Y respectively.
*/
NetworkCurve::NetworkCurve(RWPosition pos, double l, double t1, double t2)
{
    type = CurveTypeBase;
    ident = 0;
    tilepos = pos;
    length = l;
    tan1 = t1;
    tan2 = t2;
}

NetworkCurve::~NetworkCurve()
{}

/*
	return the type of the curve, useful when calling via iRibbon
*/
CurveType NetworkCurve::curveType()
{
    return this->type;
}

/*
	Return a QString representaion of the curve.
*/
QString NetworkCurve::toString()
{
    return QString("%1 Curve %2, %3").arg(type)
									 .arg(tilepos.toString())
									 .arg(length);
}

/*
	Return the length of the curve.
*/
double NetworkCurve::getLen()
{
    //Return the length of the object
    return length;
}

/*
	Return the within-tile X coordinate of the start of the curve
*/
double NetworkCurve::getTx()
{
    return tilepos.getX().getTile();
}

/*
	Return the X coordinate in the route of the start of the curve
*/
double NetworkCurve::getRx()
{
    return tilepos.getX().getRoute();
}
    
/*
	Return the within-tile Y(Z) coordinate of the start of the curve
*/
double NetworkCurve::getTz()
{
    return tilepos.getZ().getTile();
}
    
/*
	Return the Y(Z) coordinate in the route of the start of the curve
*/
double NetworkCurve::getRz()
{
    return tilepos.getZ().getRoute();
}
    
/*
	Return the first value of the StartTangents array
*/
double NetworkCurve::getTan1()
{
    return tan1;
}
    
/*
	Return the second value of the StartTangents array
*/
double NetworkCurve::getTan2()
{
    return tan2;
}

/*
	Return the angle of the network element by converting the StartTangents
	values from the XML into a value in Radians.
*/
double NetworkCurve::getATan2()
{
    return atan2(tan2, tan1);
}

/*
	Return the X coordinate of the point in global terms.
*/
double NetworkCurve::getX()
{
    return getTx() + TILE_SIZE_METRES * getRx();
}

/*
	Return the Y (Z) coordinate of the point in global terms.
*/
double NetworkCurve::getZ()
{
    return getTz() + TILE_SIZE_METRES * getRz();
}

/*
	Return the element ID for the curve in question.
*/
int NetworkCurve::getId()
{
    return this->ident;
}

/*
	Set the element ID for the curve in question.
*/
void NetworkCurve::setId(int i)
{
    this->ident = i;
}



NetworkCurveArc::NetworkCurveArc(RWPosition pos, double l, double t1, double t2, double ac, int sg) :
            NetworkCurve(pos, l, t1, t2)
{
    type = CurveTypeArc;
    arc = ac;
    sign = sg;
}

QString NetworkCurveArc::toString()
{
    return QString("Arc %1, L %2, Ac %3, Ag %4").arg(tilepos.toString())
												.arg(getLen())
												.arg(arc)
												.arg(getATan2());
}
    
double NetworkCurveArc::getArc()
{
    return arc;
}
        
int NetworkCurveArc::getSign()
{
    return sign;
}



NetworkCurveEasement::NetworkCurveEasement(RWPosition pos, double l, double t1, double t2, double sh, double sg) :
                            NetworkCurve(pos, l, t1, t2)
{
    type = CurveTypeEasement;
    sharp = sh;
    sign = sg;
}

QString NetworkCurveEasement::toString()
{
    return QString("Easement %1, L %2, A %3, Sh %4, Sg %5").arg(tilepos.toString())
															.arg(length)
															.arg(sharp)
						                                    .arg(sign)
															.arg(getATan2());
}
    
double NetworkCurveEasement::getSharpness()
{
    return sharp;
}
        
double NetworkCurveEasement::getSign()
{
    return sign;
}


NetworkCurveStraight::NetworkCurveStraight(RWPosition pos, double l, double t1, double t2) :
                            NetworkCurve(pos, l, t1, t2)
{
    type = CurveTypeStraight;
}

QString NetworkCurveStraight::toString()
{
    return QString("Straight %1, L %2, A %3").arg(tilepos.toString())
		                                     .arg(length)
											 .arg(getATan2());
}
