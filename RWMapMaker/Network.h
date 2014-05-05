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

#ifndef RWNETWORK_H
#define RWNETWORK_H

#include <QtCore/QString>
#include <QtCore/QList>

#include "RWPosition.h"

/*
*/
enum CurveType
{
    CurveTypeBase,
    CurveTypeStraight,
    CurveTypeArc,
    CurveTypeEasement
};

/*
	"Interface" definition for the Network classes
*/
class INetwork
{
public:
	virtual ~INetwork() {};
    virtual CurveType curveType() = 0;
    virtual QString toString() = 0;
    virtual double getLen() = 0;
    virtual double getX() = 0;
    virtual double getZ() = 0;
    virtual int getId() = 0;
};

typedef QList<INetwork*> NetworkList;
void deleteNetworkList(NetworkList *net);

/*
	"Interface" definition for the Ribbon classes.
*/
class IRibbon
{
public:
	virtual ~IRibbon() {};
    virtual QString toString() = 0;
    virtual QString ribbonid() = 0;
    virtual NetworkList *segments() = 0;
    virtual void append(INetwork *c) = 0;
};

typedef QList<IRibbon*> RibbonList;
void deleteRibbonList(RibbonList *net);


/*
	Description of the track network as a list of
	ribbons - as yet not explicitly connected together.
*/
class TrackNetworkRibbon : public IRibbon
{
private:
    QString ribid;
    QString netid;
    NetworkList *curves;

public:
	TrackNetworkRibbon(QString &rid, QString &nid);
	virtual ~TrackNetworkRibbon();
    virtual QString toString();
    virtual QString ribbonid();
    QString networkid();
    virtual NetworkList *segments();
    virtual void append(INetwork *curve);
};


/*
	Description of the road network as a list of
	ribbons - not explicitly connected together.
*/
class RoadNetworkRibbon : public IRibbon
{
    QString ribid;
    NetworkList *curves;

public:
	RoadNetworkRibbon(QString &rid);
	virtual ~RoadNetworkRibbon();
    virtual QString toString();
    virtual QString ribbonid();
    virtual NetworkList *segments();
    virtual void append(INetwork *curve);
};

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
class NetworkCurve : public INetwork
{
protected:
	RWPosition tilepos;
    CurveType type;
    int ident;
    double length;
    double tan1;
    double tan2;

public:
	NetworkCurve(RWPosition pos, double l, double t1, double t2);
	virtual ~NetworkCurve();
    virtual CurveType curveType();
    virtual QString toString();
    double getLen();

    double getTx();
    double getRx();
    double getTz();
    double getRz();
    double getTan1();
    double getTan2();
    double getATan2();
    double getX();
    double getZ();
    int getId();
    void setId(int i);
};


/*
	Class describing a simple Curve in a network
*/
class NetworkCurveArc : public NetworkCurve
{
    double arc;
    int sign;

    //Derived class describing a curved segment of a ribbon.
    //New functions getArc and getSign return the curvature.
    //
public:
	NetworkCurveArc(RWPosition pos, double l, double t1, double t2, double ac, int sg);

    virtual QString toString();
    double getArc();
    int getSign();
};

/*
	Class describing a Easement Curve in a network
	New functions getArc and getSign return the curvature.
*/
class NetworkCurveEasement : public NetworkCurve
{
    double sharp;
    double sign;

public:
	NetworkCurveEasement(RWPosition pos, double l, double t1, double t2, double sh, double sg);

    virtual QString toString();
    double getSharpness();
    double getSign();
};

/*
	Class describing a straight section of a network
*/
class NetworkCurveStraight : public NetworkCurve
{
public:
	NetworkCurveStraight(RWPosition pos, double l, double t1, double t2);

	virtual QString toString();
};      

#endif
