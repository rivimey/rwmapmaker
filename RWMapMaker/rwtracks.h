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

#ifndef RWTRACKS_H
#define RWTRACKS_H

#include <QtGui/QPixmap>
#include <QtCore/QList>
#include <QtCore/QMap>

typedef unsigned long IDType;

enum PatternType
{
	None,
	Reference,
	Pat2WTurnout,
	Pat3WTurnout,
	PatSingleSlip,
	PatDoubleSlip,
	PatCrossing
};

enum PointDirection
{
	PointDir_Forwards,
	PointDir_Backwards
};

enum PointDirectionType
{
	PointDir_1way,
	PointDir_2way
};

/*
  A track pattern defines how tracks join together - for example
  in a point or crossing. The XML includes references to patterns,
  so some instances of a pattern are a reference not the actual one.

  This is implemented using a real base class that implements a reference,
  with a subclasses that implements an abstract base class for the
  various actual patterns.
*/
class TrackPattern
{
	IDType refID;
	TrackPattern *ref;
	bool isRef;

public:
	TrackPattern() : refID(0), ref(0), isRef(false) {}
	virtual ~TrackPattern() {}
	virtual PatternType type() { return (isRef && ref != 0) ? ref->type() : Reference; }

	IDType getRefID() { return refID; }
	void setRefID(IDType id) { refID = id; isRef = true; }

	bool hasRefTarget() { return isRef && ref != 0; }
	TrackPattern *getRefTarget() { return isRef ? ref : 0; }
	void setRefTarget(TrackPattern *t) { ref = t; isRef = true; }

	virtual bool isReference() { return isRef; }
};

/*
  Abstract base class for actual patterns (rather than references)
*/
class TrackPatternBase : public TrackPattern
{
	QString state;
	int detail;
	bool global;
	QString blueprnt;
	QString blueprntName;
	QString entity;

public:
    TrackPatternBase() : detail(0), global(false) {}
    virtual ~TrackPatternBase() {}
	virtual PatternType type() { return PatCrossing; }

	QString getState() { return state; }
	void setState(QString s) { state = s; }

	QString getBlueprint() { return blueprnt; }
	void setBlueprint(QString b) { blueprnt = b; }

	QString getBlueprintName() { return blueprntName; }
	void setBlueprintName(QString n) { blueprntName = n; }

	QString getEntityID() { return entity; }
	void setEntityID(QString id) { entity = id; }

	int getDetailLevel() { return detail; }
	void setDetailLevel(int level) { detail = level; }

	bool getIsGlobal() { return global; }
	void setIsGlobal(bool g) { detail = global; }
};


/*
  Pattern for a simple crossover of two lines.
*/
class CrossingPattern : public TrackPatternBase
{
public:
	CrossingPattern() {}
	virtual ~CrossingPattern() {}
	PatternType type() { return PatCrossing; }
};


/*
  Pattern for a 2-to-1 junction
*/
class TurnoutPattern : public TrackPatternBase
{
public:
	TurnoutPattern() {}
	virtual ~TurnoutPattern() {}
	PatternType type() { return Pat2WTurnout; }
};


/*
  Pattern for a 3-to-1 junction
*/
class ThreeWayPattern : public TrackPatternBase
{
public:
	ThreeWayPattern() {}
	virtual ~ThreeWayPattern() {}
	PatternType type() { return Pat3WTurnout; }
};


/*
  Pattern for a single slip - that is, a crossover (X)
  that permits travel along one side as well as diagonally.
*/
class SingleSlipPattern : public TrackPatternBase
{
public:
	SingleSlipPattern() {}
	virtual ~SingleSlipPattern() {}
	virtual PatternType type() { return PatSingleSlip; }
};


/*
  Pattern for a double slip - that is, a crossover (X)
  that permits travel along two sides as well as diagonally.
*/
class DoubleSlipPattern : public TrackPatternBase
{
public:
	DoubleSlipPattern() {}
	virtual ~DoubleSlipPattern() {}
	virtual PatternType type() { return PatDoubleSlip; }
};


/*
	Base class for track properties. The only item that seems
	to be global is whether this item is owned by a scenario or
	not.
*/
class TrackProperty
{
	bool scenario;
public:
	TrackProperty() : scenario(0) {}
	virtual ~TrackProperty() {}

	bool isScenario() { return scenario; }
	void setScenario(bool value) { scenario = value; }
};

/*
	Base class for track properties located at a single point and
	thus are located using an offset from the start of the ribbon.
*/
class TrackPropertySingle : public TrackProperty
{
	double offset;

public:
	TrackPropertySingle() : offset(0) {}
	virtual ~TrackPropertySingle() {}

	double getOffset() { return offset; }
	void setOffset(double value) { offset = value; }
};

/*
	Base class for track properties that affect a region of track
	and are located using start and end values, from the start of
	the ribbon.
*/
class TrackPropertyRange : public TrackProperty
{
	double start;
	double end;

public:
	TrackPropertyRange() : start(0), end(0) {}
	virtual ~TrackPropertyRange() {}

	double getStart() { return start; }
	double getEnd() { return end; }
	void setRange(double s, double e) { start = s; end = e; }
	void setStart(double s) { start = s; }
	void setEnd(double e) { end = e; }
};

/*
	Definition of a Signal attached to the track.
*/
class SignalProperty : public TrackPropertySingle
{
	QString name;
	QString blueprint;

public:
	SignalProperty() {};
	virtual ~SignalProperty() {};

};

/*
	Generic property type, referencing a blueprint. There are up
	to 3 blueprints referenced here!
	What does LoftScale do here?
*/
class GenericProperty : public TrackPropertyRange
{
	QString name;
	QString blueprint;
	QString blueprint_2;
	QString blueprint_elec;

	float loftScale;

public:
	GenericProperty() {};
	virtual ~GenericProperty() {};

	QString getName() { return name; }
	void setName(QString value) { name = value; }

	double getLoftScale() { return loftScale; }
	void setLoftScale(double value) { loftScale = value; }

	QString getBlueprint() { return blueprint; }
	void setBlueprint(QString value) { blueprint = value; }

	QString getBlueprint2() { return blueprint_2; }
	void setBlueprint2(QString value) { blueprint_2 = value; }

	QString getElecBlueprint() { return blueprint_elec; }
	void setElecBlueprint(QString value) { blueprint_elec = value; }
};

/*
	Definition of a Portal property.
*/
class PortalProperty : public TrackPropertySingle
{
	QString blueprint;
	PointDirectionType facingDirType;

public:
	PortalProperty() {};
	virtual ~PortalProperty() {};

	QString getBlueprint() { return blueprint; }
	void setBlueprint(QString value) { blueprint = value; }

	PointDirectionType getPointDirection() { return facingDirType; }
	void setPointDirection(PointDirectionType value) { facingDirType = value; }
};

typedef QList<TrackProperty*> TrackPropertyList;


/*
	A Track Connection, as used by a TrackNode to "join" track ribbons
	together using the ribbonId. The connection reflects the Network-
	cNetworkNode-sRConnection element in the xml
	
*/
struct TrackConn
{
	QString id;
	bool end; // xml: float
	bool linkable;
};

typedef QList<TrackConn*> TrackNodeConnList;

struct HeightPoint
{
	double height;
	double position;
	bool manual;
};

typedef QList<HeightPoint*> HeightPointList;

/*
	Represents a Network-cTrackRibbon object, which describes a ribbon of
	track including overall length, heights and other track
	properties along that length.

	TrackRibbon nodes are referenced using the node ID in the sRConnection
	of the TrackNode.
*/
class TrackRibbon
{
	QString ribbonID;
	double length;
	bool lockCWhenMod;
	bool superElev;
	HeightPointList heights;
	TrackPropertyList props;

public:
	TrackRibbon() {};
	~TrackRibbon();

	QString getRibbonID() { return ribbonID; }
	void setRibbonID(QString value) { ribbonID = value; }

	double getRibbonLength() { return length; }
	void setRibbonLength(double value) { length = value; }

	bool getSuperElevated() { return superElev; }
	void setSuperElevated(bool value) { superElev = value; }

	bool getLockCounterWhenMod() { return lockCWhenMod; }
	void setLockCounterWhenMod(bool value) { lockCWhenMod = value; }

	void addHeight(HeightPoint *h) { heights.append(h); }
	void addProperty(TrackProperty *p) { props.append(p); }

	HeightPointList &getHeights() { return heights; }
	TrackPropertyList &getProperties() { return props; }
};

typedef QList<TrackRibbon*> RWTrackRibbonList;


/*
	Represents one track Node from the tracks.bin file. These nodes
	contain one or more Connections, two pattern references, and a route
	vector that doesn't appear to be used.

	Each Connection is represented by a TrackNodeConn structure.
*/
class TrackNode
{
	TrackNodeConnList connections;
	int fixedPatIdx;
	int patIdx;

public:
	TrackNode() : fixedPatIdx(-1), patIdx(-1) {};
	~TrackNode();

	int fixedPatternIdx() const { return fixedPatIdx; }
	int patternIdx() const { return patIdx; }

	void setFixedPatternIdx(int v) { fixedPatIdx = v; }
	void setPatternIdx(int v) { patIdx = v; }

	void addConn(TrackConn *n) { connections.append(n); }
	
	TrackNodeConnList &getConnections() { return connections; }

private:
	
};

typedef QList<TrackNode*> RWTrackNodeList;


/*
	Represents the contents of a tracks.bin file. 
	Contains the NetworkID (from Network-cTrackNetwork), the Ribbon and
	Node lists from RibbonContainer.
*/
class RWTrackInfo
{
	QString NetworkID;
	RWTrackNodeList nodes;
	RWTrackRibbonList ribbons;

public:
	RWTrackInfo(QString id) : NetworkID(id) {};
	~RWTrackInfo();

	void addNode(TrackNode *n);
	void addRibbon(TrackRibbon *n);

	QString networkId() { return NetworkID; }
	const RWTrackNodeList &trackNodes() const { return nodes; }
	const RWTrackRibbonList &trackRibbons() const { return ribbons; }

private:
	
};

typedef QList<RWTrackInfo*> RWTrackInfoList;




#endif // RWTRACKS_H
