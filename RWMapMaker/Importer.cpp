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

#undef PUGIXML_NO_STL
#include "pugixml.hpp"

#include "Pair.h"
#include "TileMaps.h"
#include "scale.h"
#include "asset.h"
#include "range.h"
#include "importer.h"
#include "qxtlogger.h"
#include <assert.h>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream, std::stringbuf

#define min(x,y)   ((x < y) ? (x) : (y))
#define max(x,y)   ((x > y) ? (x) : (y))


uint qHash(QPoint p)
{
	return qHash( p.x()) ^ qHash(p.y() );
}

QString abbreviateFilename(QString fn)
{
	QFileInfo fi(fn);
	QFileInfo pi(fi.path());
	return pi.fileName() + "\\" + fi.fileName();
}


QString print_xnode(pugi::xml_node node)
{
	std::stringstream strm;
	node.print(strm, "  ");
	std::string str = strm.str();	// stream => string
	QString qs(str.data());			// string => QString
	return qs;
}

QString xmlLoc(pugi::xml_node &node)
{
	return QString(node.path().data());
}


// pointer-ish things must be NULL at start; clear()
// will try to delete non-zero pointers, but will also
// init the rest properly
RWImporter::RWImporter() : 
			terrainTileMap(0),
			trackTileMap(0),
			roadTileMap(0),
			roadRibbons(0),
			trackRibbons(0),
			preloadList(0),
			terrain(0),
			scenery(0),
			tileFileRegex("[+-][0-9]{6}[+-][0-9]{6}(.xml|.bin)$"),
			serzAuto(true)
{
	setupSerzPaths();
	serzFileAccess = new SerzFile("trsim:serz.exe");
	resetAbort();
	clear();
}

RWImporter::~RWImporter()
{
	// these are our own... delete if non-zero
	deleteRibbonList(trackRibbons); trackRibbons = NULL;
	deleteRibbonList(roadRibbons); roadRibbons = NULL;
	deleteTerrainArrayList(terrain); terrain = NULL;
	deleteSceneryItemList(scenery); scenery = NULL;
	deleteBPMetadataList(preloadList); preloadList = NULL;

	delete terrainTileMap; 	terrainTileMap = NULL;
	delete trackTileMap; 	trackTileMap = NULL;
	delete roadTileMap; 	roadTileMap = NULL;
}

void RWImporter::resetAbort()
{
	abortFlag = false;
}

bool RWImporter::isAborting()
{
	return abortFlag;
}

void RWImporter::abort()
{
	abortFlag = true;
}

void RWImporter::setSerzPath(QString &p)
{
	serzPath = p;
	delete serzFileAccess;
	serzFileAccess = new SerzFile(p);
}

QString RWImporter::getSerzPath() const
{
	return serzPath;
}

void RWImporter::setTempPath(QString &p)
{
    if (p.isNull())
        serzFileAccess->setUseTempFolder(false);
    else
    {
        serzFileAccess->setUseTempFolder(true);
        serzFileAccess->setTempFolder(p);
    }
}

QString RWImporter::getTempPath() const
{
	return serzFileAccess->getTempFolder();
}

QString RWImporter::getSerzExe() const
{
	return serzFileAccess->serzExePath();
}

void RWImporter::setSerzAuto(bool a)
{
	if (serzAuto != a)
	{
		serzAuto = a;
		if (serzAuto)
		{
			setupSerzPaths();
			delete serzFileAccess;
			serzFileAccess = new SerzFile("trsim:serz.exe");
		}
	}
}

bool RWImporter::getSerzAuto()
{
	return serzAuto;
}

void RWImporter::setupSerzPaths()
{
    // try to work out where the railworks folder is.
	QStringList paths = QStringList();
	QString rwPath;

	QSettings registryLM("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 24010", QSettings::NativeFormat);
	rwPath = registryLM.value("InstallLocation", "").toString();
    if (!rwPath.isEmpty())
		paths << rwPath;

	paths << QString(QDir::rootPath() + "/program files/steam/steamapps/common/railworks");
	paths << QString(QDir::rootPath() + "/program files (x86)/steam/steamapps/common/railworks"); 

	qxtLog->debug() << "setupSerzPaths" << paths;
	QDir::setSearchPaths("trsim", paths);
}

/*!
	
*/
void RWImporter::clearPreloads()
{
	deleteBPMetadataList(preloadList);
	preloadList = new BPMetadataList();
}
/*!
	
*/
void RWImporter::clear()
{
	//qxtLog->debug("RWImporter::clear()"); 

	// these are our own... delete if non-zero
	delete terrainTileMap; 	terrainTileMap = NULL;
	delete trackTileMap; 	trackTileMap = NULL;
	delete roadTileMap; 	roadTileMap = NULL;

	deleteRibbonList(trackRibbons);
	trackRibbons = new RibbonList();
	
	deleteRibbonList(roadRibbons);
	roadRibbons = new RibbonList();

	deleteTerrainArrayList(terrain);
	terrain = new TerrainArrayList();

	deleteSceneryItemList(scenery);
	scenery = new SceneryItemList();

	resetAbort();
	routeFilename = QString();
	RouteName = QString();
	RouteBlueprint = QString();
	TerrainBlueprint = QString();
	OriginLatitude = 0;
	OriginLongitude = 0;
	dirTrack = QString();
	dirRoads = QString();
	dirLofts = QString();
	dirScenery = QString();
	dirTerrain = QString();
	dirMixMap = QString();
	tileWidth = 0;
	tileHeight = 0;
	
	overallRect = QRectF();
	overallMinHeight=  10000;
	overallMaxHeight= -10000;
	minHeightTx= 0;
	maxHeightTx= 0;
	minHeightTy= 0;
	maxHeightTy= 0;
}

/*!
	Convert a value specified in hex to a 32-bit IEEE float, with byte
	order conversion as required.
*/
float RWImporter::convertHexFloat(QString &hex)
{
	union {
		int L;
		char B[4];
		float F;
	} bits;
	
	bool ok;
	ulong v = hex.toULong(&ok, 16);
	if (!ok)
	{
		qxtLog->error() << "RWImporter::convertHexFloat(" << hex << ") conversion failure";
		return 0;
		//throw std::logic_error("Hex to int conversion failure");
	}
	// assume byte order is intel...
	bits.B[3] = v & 0xFF;
	bits.B[2] = (v >>  8) & 0xFF;
	bits.B[1] = (v >> 16) & 0xFF;
	bits.B[0] = (v >> 24) & 0xFF;

	return bits.F;
}

/*!
	Convert a value specified in hex to a 32-bit long, with byte
	order conversion as required.
*/
long RWImporter::convertHexLong(QString &hex)
{
	union {
		int L;
		char B[4];
	} bits;
	
	bool ok;
	ulong v = hex.toULong(&ok, 16);
	if (!ok)
	{
		qxtLog->error() << "RWImporter::convertHexFloat(" << hex << ") conversion failure";
		return 0;
		//throw std::logic_error("Hex to int conversion failure");
	}

	// byte order is intel...
	bits.B[3] = v & 0xFF;
	bits.B[2] = (v >>  8) & 0xFF;
	bits.B[1] = (v >> 16) & 0xFF;
	bits.B[0] = (v >> 24) & 0xFF;

	return bits.L;
}

/*!
	Return an RWCoordinate for the referenced XML Node containing
	a RouteCoordinate and a TileCoordinate
*/
RWCoordinate RWImporter::getCoordinate(pugi::xml_node& node)
{
	if (strcmp(node.name(), "cFarCoordinate") == 0)
	{
		pugi::xpath_node rnode = node.select_single_node("RouteCoordinate/cRouteCoordinate/Distance");
		pugi::xpath_node tnode = node.select_single_node("TileCoordinate/cTileCoordinate/Distance");
		if (!rnode.node() || !tnode.node() )
		{
			qxtLog->error() << "getCoordinate(" << QString::fromStdString(node.path()) << ") failed to find Distance node(s)";
			return RWCoordinate();
		}
		else
		{
			QString r(rnode.node().child_value());
			QString t(tnode.node().child_value());

			RWCoordinate c = RWCoordinate(r, t);
			if (c.isValid())
				return c;
			else
			{
				pugi::xml_node np = node.parent().parent().parent().parent().parent().parent().parent().parent();
				qxtLog->error() << "getCoordinate(" << xmlLoc(node) << ") location (" << c.toString() << ") invalid in:\n" << print_xnode(np);
				return c;
			}
		}
	}
	else
	{
		qxtLog->error() << "getCoordinate(" << QString::fromStdString(node.path()) << ") failed to find cFarCoordinate node";
		return RWCoordinate();
	}
}

/*!
	Return an RWPosition value for the XML cFarVector3 structure referenced by
	the \a node provided. 
*/
RWPosition RWImporter::getPosition(pugi::xml_node& node)
{
	if (strcmp(node.name(), "cFarVector2") == 0)
	{
		pugi::xpath_node xnode = node.select_single_node("X/cFarCoordinate");
		pugi::xpath_node znode = node.select_single_node("Z/cFarCoordinate");
		if (!xnode.node() || !znode.node() )
		{
			qxtLog->error() << "getPosition(" << QString::fromStdString(node.path()) << ") failed to find coordinate nodes";
			return RWPosition();
		}
		else
		{
			RWCoordinate xcoord = getCoordinate(xnode.node());
			RWCoordinate zcoord = getCoordinate(znode.node());
			return RWPosition(xcoord, zcoord);
		}
	}
	else
	{
		qxtLog->error() << "getPosition(" << QString::fromStdString(node.path()) << ") failed to find StartPos node";
		return RWPosition();
	}
}

/*!
	Parse a StartTangent element. Older routes use a single element with a pair of
	space-separated float values. Newer routes use a cHcRVector array with two 'e'
	elements each containing a float. The two values represent the x and y arguments
	to the atan2(x,y) function, encoding an angle.
*/
TangentPair RWImporter::getStartTangent(pugi::xml_node &root)
{
	TangentPair t;
	pugi::xpath_node tang = root.select_single_node("StartTangent");
	QString tang_arr(tang.node().child_value());
	if (tang_arr.isEmpty())
	{
		pugi::xpath_node tang_e1 = tang.node().select_single_node("cHcRVector2/Element/e");
		pugi::xpath_node tang_e2 = tang_e1.node().next_sibling();
		if (!tang_e1.node() || !tang_e2.node() )
		{
			qxtLog->error() << "getStartTangent(" << QString::fromStdString(tang.node().path()) << ") failed to find Element/e nodes";
			t = TangentPair(0.5,0.5);
		}
		else
		{
			QString tang1(tang_e1.node().child_value());
			QString tang2(tang_e2.node().child_value());
			t = TangentPair(tang1.toDouble(), tang2.toDouble());
		}
	}
	else
	{
		QStringList &t12 = tang_arr.split(QChar(' '));
		t = TangentPair(t12[0].toDouble(), t12[1].toDouble());
	}
	return t;
}

/*!
	Parse an array element containing two values.
*/
Pair RWImporter::getPair(const QString &filename, pugi::xml_node &root, char *name)
{
	pugi::xpath_node val = root.select_single_node(name);
	if (!val)
	{
		qxtLog->error() << "getPair(" << abbreviateFilename(filename) << ", " << name << ") not found at " << xmlLoc(root);
		return Pair();
	}
	QString arr(val.node().child_value());
	QStringList &p = arr.split(QChar(' '));
	Pair t = Pair(p[0].toDouble(), p[1].toDouble());
	return t;
}

/*!
	Returns a double value from an xml node rooted at "node" with the name "name".
*/
bool RWImporter::getNamedBool(const QString &filename, pugi::xml_node &node, char *name, bool missingOK)
{
	pugi::xpath_node xval = node.select_single_node(name);
	if (!xval)
	{
		if (!missingOK)
			qxtLog->error() << "getBool(" << abbreviateFilename(filename) << ", " << name << ") not found at " << xmlLoc(node);
		return false;
	}
	QString bval(xval.node().child_value());
	if (bval == "eTrue" || bval == "1")
		return true;
	else if (bval == "eFalse" || bval == "0")
		return false;
	else
	{
		qxtLog->error() << "getBool(" << abbreviateFilename(filename) << ") could not convert \'" << bval << "\' to a value";
		return false;
	}
}

/*!
	Returns a double value from an xml node rooted at "node" with the name "name".
*/
QString RWImporter::getNamedString(const QString &filename, pugi::xml_node &node, char *name, bool missingOK)
{
	pugi::xpath_node xval = node.select_single_node(name);
	if (!xval)
	{
		if (!missingOK)
			qxtLog->error() << "getString(" << abbreviateFilename(filename) << ", " << name << ") not found at " << xmlLoc(node);
		return QString();
	}
	return QString(xval.node().child_value());
}

/*!
	Returns a double value from an xml node rooted at "node" with the name "name".
*/
double RWImporter::getNamedDouble(const QString &filename, pugi::xml_node &node, char *name, bool missingOK)
{
	pugi::xpath_node xval = node.select_single_node(name);
	if (!xval)
	{
		if (!missingOK)
			qxtLog->error() << "getDouble(" << abbreviateFilename(filename) <<  ", " << name << ") not found at " << xmlLoc(node);
		return 0;
	}
	return QString(xval.node().child_value()).toDouble();
}

/*!
	Returns a double value from an xml node rooted at "node" with the name "name".
*/
int RWImporter::getNamedInt(const QString &filename, pugi::xml_node &node, char *name, bool missingOK)
{
	pugi::xpath_node xval = node.select_single_node(name);
	if (!xval)
	{
		if (!missingOK)
			qxtLog->error() << "getInt(" << abbreviateFilename(filename) <<  ", " << name << ") not found at " << xmlLoc(node);
		return 0;
	}
	return QString(xval.node().child_value()).toInt();
}

/*!
	Protected overridable function used to call the loading track tiles.
*/
void RWImporter::emitLoadTile(TileType ty, int x, int y)
{
	loadTile(ty, x, y);
}

/*!
	Protected overridable function used to call the scan Terrain signal.
*/
void RWImporter::emitScanTerrain()
{
	scanTerrain();
}

/*!
	Protected overridable function used to call the scan Tracks signal.	
*/
void RWImporter::emitScanTracks()
{
	scanTracks();
}

/*!
	Protected overridable function used to call the scan Roads signal.	
*/
void RWImporter::emitScanRoads()
{
	scanRoads();
}

/*!
	Protected overridable function used to call the scan Roads signal.	
*/
void RWImporter::emitScanScenery()
{
	scanScenery();
}

/*!
	Start the process of loading a route into the importer. The
	given filename is the path to the RouteProperties.xml file. Once
	this is done, call one or more of the following to return info
	about the route:

		importRWRouteProps()
		importRWTileExtents()
		importRWPreloads()
		importRWTrack()
		importRWRoads()
		importRWTerrain()
*/
void RWImporter::loadRoute(QString &filename)
{
	clear();
	QString routprop = QFileInfo(filename).fileName();
	if (routprop.toLower() == QString("RouteProperties.xml").toLower())
	{
		QString basedir = QFileInfo(filename).path();
		routeFilename = filename ;
		setupLocations(basedir);
	}
}

/*!
	Return the Route Name, as provided in the RouteProperties file.

	Requires that importRWRouteProps() has been called.
*/
QString RWImporter::getRouteName()
{
	return RouteName;
}

/*!
	Return the file path to the route blueprint.

	Requires that importRWRouteProps() has been called.
*/
QString RWImporter::getRouteBlueprint()
{
	return RouteBlueprint;
}

/*!
	Return the file path to the terrain tile blueprint.

	Requires that importRWRouteProps() has been called.
*/
QString RWImporter::getTerrainBlueprint()
{
	return TerrainBlueprint;
}

/*!
	Return the location of the route Origin in decimal degrees.

	Requires that importRWRouteProps() has been called.
*/
QPointF RWImporter::getOrigin()
{
	return QPointF(OriginLatitude, OriginLongitude);
}

/*!
	Return the height of the lowest and higest terrain points
	in the terrain tiles of the route.

	Requires that importRWTerrain() has been called.
*/
QRange RWImporter::getMinMaxHeight()
{
	return QRange(overallMinHeight, overallMaxHeight);
}

/*!
	Return the extent of the route in tiles, for all 3 types of tile
	imported. That is, the width and height needed to represent the
	map.

	Requires that importRWTileExtents() has been called.
*/
QSize RWImporter::getTileExtent()
{
	return QSize(tileWidth, tileHeight);
}

/*!
	Return the extent of the route in tiles, for all 3 types of tile
	imported. That is, the width and height needed to represent the
	map.

	Requires that importRWTileExtents() has been called.
*/
QRectF RWImporter::getTiledRegion()
{
	return overallRect;
}

/*!
	Return the terrain tile list.

	Requires that importRWTerrain() has been called.
*/
TerrainArrayList *RWImporter::getTerrain()
{
	return terrain;
}

/*!
	Return the number of terrain tiles in the route.

	Requires that importRWTileExtents() has been called.
*/
int RWImporter::getTerrainTiles()
{
	if (terrainTileMap == NULL)
		throw std::logic_error("getTerrainTiles called with no map");
	return terrainTileMap->getCount();
}

/*!
	Return the map of terrain tiles, indicating which tiles have a
	tile file. Note that if there is a file, but the file is empty,
	the map will indicate a file but there will be no terrain image
	for that tile imported.

	Requires that importRWTileExtents() has been called.
*/
TileExtentMap *RWImporter::getTerrainTileMap()
{
	// Return the map of which terrain tiles exist 
	return terrainTileMap;
}


/*!
	Return the number of track tiles in the route.

	Requires that importRWTileExtents() has been called.
*/
int RWImporter::getTrackTiles()
{
	if (trackTileMap == NULL)
		throw std::logic_error("getTrackTiles called with no map");
	return trackTileMap->getCount();
}

/*!
	Return the map of track tiles, indicating which tiles have a
	tile file. Note that if there is a file, but the file is empty,
	the map will indicate its presence although no track is there.

	Requires that importRWTileExtents() has been called.
*/
TileExtentMap *RWImporter::getTrackTileMap()
{
	// Return the map of which track tiles exist 
	return trackTileMap;
}

/*!
	Return the list of ribbons of track. A ribbon is a group of track
	segments describing the straights and curves.

	Requires that importRWTrack() has been called.
*/
RibbonList *RWImporter::getTrackRibbons()
{
	// Return the track tiles 
	return trackRibbons;
}

/*!
	Return the number of road tiles in the route.

	Requires that importRWTileExtents() has been called.
*/
int RWImporter::getRoadTiles()
{
	if (roadTileMap == NULL)
		throw std::logic_error("getRoadTiles called with no map");
	return roadTileMap->getCount();
}

/*!
	Return the map of road tiles, indicating which tiles have a
	tile file. Note that if there is a file, but the file is empty,
	the map will indicate its presence although no road is there.

	Requires that importRWTileExtents() has been called.
*/
TileExtentMap *RWImporter::getRoadTileMap()
{
	return roadTileMap;
}

/*!
	Return the terrain tile list.

	Requires that importRWTerrain() has been called.
*/
SceneryItemList *RWImporter::getScenery()
{
	return scenery;
}

/*!
	Return the number of terrain tiles in the route.

	Requires that importRWTileExtents() has been called.
*/
int RWImporter::getSceneryTiles()
{
	if (sceneryTileMap == NULL)
		throw std::logic_error("getTerrainTiles called with no map");
	return sceneryTileMap->getCount();
}

/*!
	Return the map of terrain tiles, indicating which tiles have a
	tile file. Note that if there is a file, but the file is empty,
	the map will indicate a file but there will be no terrain image
	for that tile imported.

	Requires that importRWTileExtents() has been called.
*/
TileExtentMap *RWImporter::getSceneryTileMap()
{
	// Return the map of which terrain tiles exist 
	return sceneryTileMap;
}
/*!
	Return the list of ribbons of road. A ribbon is a group of road
	segments describing the straights and curves.

	Requires that importRWRoads() has been called.
*/
RibbonList *RWImporter::getRoadRibbons()
{
	return roadRibbons;
}


BPMetadataList *RWImporter::getPreloads()
{
	// Return the track tiles 
	return preloadList;
}


/*!
	Tile files are of the form +000055-000010.bin where the numbers
	indicate the tile coordinates. Parse a filename to return these
	coordinates. Only files with .xml or .bin extensions are parsed.

	It is permissible to pass in a full pathname.

	[[If the function cannot parse the filename, it returns point(999,999).]]
*/
QPoint RWImporter::filenameToLocation(QString &filename)
{
	// reduce to just the filename
	QString fname = QFileInfo(filename).fileName();
	
	// cope with the translation serzfile does for temporary xml files.
	if (fname[0] == '_')
		fname = fname.mid(1);

	// Convert filename from +000000+000000.xml form to tile coordinates. 
	if (fname.contains(tileFileRegex))
	{
		int xc, yc;
		xc = fname.left(7).toDouble();
		yc = fname.mid(7, 7).toDouble();
		return QPoint(xc, yc);
	}
	else
	{
		qxtLog->error() << "filenameToLocation(" << abbreviateFilename(filename) << "): could not parse name.";
		//throw std::logic_error("File format unexpected.");
		return QPoint();
	}
}

/*!
	Use the files in a directory to determine the extent of the information
	of the given type - for example, track tiles, terrain - and return the bounds
	of that information
	
	The resulting map is saved as a Qt Bitmap and returned along with the bounds
	of the map.
	
	The 0,0 point is at the map centre: QPoint(EXTENT_BITMAP_SIZE / 2, EXTENT_BITMAP_SIZE / 2).
*/
TileExtentMap *RWImporter::findTileExtent(QString &tileDir)
{
	QSize sizeXY = QSize(EXTENT_BITMAP_SIZE, EXTENT_BITMAP_SIZE);
	QPoint centreOffset = QPoint(EXTENT_BITMAP_SIZE / 2, EXTENT_BITMAP_SIZE / 2);
	int minX, minY;
	int maxX, maxY;

	// using a Qt bitmap, so make it and its painter
	QImage *bitmap = new QImage(sizeXY, QImage::Format_Mono);
	bitmap->setColor(INDEX_NOTILE, (uint)Qt::white);
	bitmap->setColor(INDEX_TILE, (uint)Qt::black);
	bitmap->fill((uint)INDEX_NOTILE);
	int numTiles = 0;

	//_CrtCheckMemory();

	// list the files in the folder given
	QDir folder(tileDir);
	folder.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	QStringList files = folder.entryList();
	minX = EXTENT_BITMAP_SIZE;
	minY = EXTENT_BITMAP_SIZE;
	maxX = -EXTENT_BITMAP_SIZE;
	maxY = -EXTENT_BITMAP_SIZE;
	for(int i = 0; i < files.size() && !isAborting(); i++)
	{
		QString &file = files[i];

		if (file.contains(tileFileRegex))
		{
			numTiles += 1;

			// convert filename to tile coords
			QPoint xyc = filenameToLocation(file);

			// calculate min and max
			minX = min(xyc.x(), minX);
			minY = min(xyc.y(), minY);
			maxX = max(xyc.x(), maxX);
			maxY = max(xyc.y(), maxY);

			// adjust to start in centre to allow negative coords
			//xyc += centreOffset;
			xyc += QPoint(EXTENT_BITMAP_SIZE / 2, EXTENT_BITMAP_SIZE / 2);

			// plot a point there
			bitmap->setPixel(xyc, INDEX_TILE);
		}
		else
			qxtLog->warning() << "findTileExtent(" << abbreviateFilename(tileDir) << "): unexpected file \'" << file << "\' in folder.";
	}

	QRect rect = QRect();
	rect.setCoords(minX, minY, maxX, maxY);

	return new TileExtentMap(numTiles, rect, bitmap);
}

/*!
	
*/
void RWImporter::setupLocations(QString &basedir)
{
	// Given the location of the route folder, precalculate the
	//subsidiary folder locations for that route 
	dirTrack = basedir + "/Networks/Track Tiles";
	dirRoads = basedir + "/Networks/Road Tiles";
	dirLofts = basedir + "/Networks/Loft Tiles";
	dirScenery = basedir + "/Scenery";
	dirTerrain = basedir + "/Terrain";
	dirMixMap = basedir + "/MixMap";
	dirAssets = basedir + "/../../../Assets";
	dirAssets = QFileInfo(dirAssets).canonicalFilePath();	// might be "" if route is outside RW folder.
	if (dirAssets.isEmpty())
		qxtLog->warning() << "For route " << basedir << " no assets folder could be found";
}

/*!
	Return the asset pathname for the blueprint referenced by \a node as a string.
	If the emptyOK flag is defined, it is OK for the elements to be undefined or
	empty; otherwise, a Warning is logged.
*/
QString RWImporter::blueprintLocation(const QString &filename, pugi::xml_node& node, bool emptyOK)
{
	// given a node referring to an iBlueprintLibrary-cAbsoluteBlueprintID, return a
	// relative filename for the referenced asset 
	pugi::xpath_node pv = node.select_single_node("BlueprintSetID/iBlueprintLibrary-cBlueprintSetID/Provider");
	pugi::xpath_node pd = node.select_single_node("BlueprintSetID/iBlueprintLibrary-cBlueprintSetID/Product");
	pugi::xpath_node nid = node.select_single_node("BlueprintID");
	if (nid && pd && pv)
	{
		QString pv_s = pv.node().child_value();
		QString pd_s = pd.node().child_value();
		QString nid_s = nid.node().child_value();

		if (!pv_s.isEmpty() && !pd_s.isEmpty() && !nid_s.isEmpty())
			return QString("Assets\\%1\\%2\\%3").arg(pv_s).arg(pd_s).arg(nid_s);
		else if (!pv_s.isEmpty() && !pd_s.isEmpty())
			return QString("Assets\\%1\\%2").arg(pv_s).arg(pd_s);
		else if (emptyOK)
			return QString();
	}
	else if (pd && pv)
	{
		QString pv_s = pv.node().child_value();
		QString pd_s = pd.node().child_value();

		if (!pv_s.isEmpty() && !pd_s.isEmpty())
			return QString("Assets\\%1\\%2").arg(pv_s).arg(pd_s);
		else if (emptyOK)
			return QString();
	}

	node = node.parent().parent();
	qxtLog->warning() << "blueprintLocation(" << abbreviateFilename(filename) << ", " <<
						xmlLoc(node) << "): missing or empty blueprint information in: \n" << print_xnode(node);;
	return QString();
}

/*!
	Update the variables that keep track of the lowest and highest
	heights recorded in the scene and which tile they fall in.
*/
void RWImporter::updateMinMaxHeight(double v, int x, int y)
{
	if (v < overallMinHeight)
	{
		overallMinHeight = v;
		minHeightTx = x;
		minHeightTy = y;
	}
	if (v > overallMaxHeight)
	{
		overallMaxHeight = v;
		maxHeightTx = x;
		maxHeightTy = y;
	}
}

/*!
	Read in information from the RouteProperties file, such as the
	Route Name, the Lat/Long Origin, and the Route and Terrain Blueprint
	locations.
*/
void RWImporter::importRWRouteProps()
{
	// load the track ribbons from the track folder
	if (!routeFilename.isEmpty())
	{
		pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(routeFilename);
		if (xmlDoc == NULL)
		{
			qxtLog->warning() << "importRWRouteProps(" << abbreviateFilename(routeFilename) << "): unable to load xml file " << serzFileAccess->lastResultStr();
			return;
		}

		pugi::xpath_node xpnode;
		xpnode = xmlDoc->select_single_node("/cRouteProperties/DisplayName/Localisation-cUserLocalisedString/English");
		if (xpnode)
			RouteName = xpnode.node().child_value();

		xpnode = xmlDoc->select_single_node("/cRouteProperties/BlueprintID/iBlueprintLibrary-cAbsoluteBlueprintID");
		if (xpnode)
			RouteBlueprint = this->blueprintLocation(routeFilename, xpnode.node());

		xpnode = xmlDoc->select_single_node("/cRouteProperties/TerrainBlueprint/iBlueprintLibrary-cAbsoluteBlueprintID");
		if (xpnode)
			TerrainBlueprint = this->blueprintLocation(routeFilename, xpnode.node());

		xpnode = xmlDoc->select_single_node("/cRouteProperties/MapProjection/cMapProjectionOwner/MapProjection/cUTMMapProjection/Origin/sGeoPosition");
		if (xpnode)
		{
			OriginLatitude = getNamedDouble(routeFilename, xpnode.node(), "Lat");
			OriginLongitude = getNamedDouble(routeFilename, xpnode.node(), "Long");

			if ((OriginLatitude < -180 || OriginLatitude >180) ||
			    (OriginLongitude < -180 || OriginLongitude >180) )
			{
				qxtLog->error() << "importRWRouteProps(" << abbreviateFilename(routeFilename) << ") Invalid longitude or latitude for route.";
			}
		}
		else
		{
			qxtLog->error() << "importRWRouteProps(" << abbreviateFilename(routeFilename) << ") failed to find Origin/sGeoPosition nodes";
			OriginLatitude = OriginLongitude = 0; 
		}
		delete xmlDoc;
		//_CrtCheckMemory();
	}
}

/*!
	The Preload files store metadata about routes and assets such as the
	locational category and an example image. They are associated with the
	item only by a GUID.

	Locate the preload files for an installation, load them, and store them
	in a mapping by GUID.
*/
void RWImporter::importRWPreloads(RWLocations &rwloc)
{
	// Given filename is the path to the RouteProperties.xml file, load the track ribbons
	// from the track folder
	if (routeFilename != NULL && !dirAssets.isEmpty())
	{
		// list the files in the folder given
		QStringList folders = rwloc.search_Preloadfolders(dirAssets);
		
		emit scanPreloads(folders.size());

		for(int i = 0; i < folders.size() && !isAborting(); i++)
		{
			QDir folder(folders[i]);

		    emit loadPreload(folders[i]);
			
			folder.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
			QStringList files = folder.entryList(QDir::Files | QDir::Readable);

			for(int j = 0; j < files.size() && !isAborting(); j++)
			{
				QString file = folders[i] + "/" + files[j];

				// not tile files!
				if (file.endsWith(".xml") || file.endsWith(".bin"))
				{
					BPMetadataList *md = importRWPreloadFile(file);
					if (md != NULL)
						preloadList->append(*md);
				}
				else if (file.endsWith(".TgPcDx"))
				{
				}
				else
					qxtLog->warning() << "importRWPreloads(" << abbreviateFilename(folders[i]) << "): unexpected file \'" << file << "\' in folder.";
			}
		}
	}
}


/*!
	Keep track, using a mapping image, of the tiles of each type tha are
	present in a route. "Present" here means simply "has a file" for the tile,
	so files that have no content are still counted here.

	Additionally, maintain overallRect, recording the total tile area that the
	route occupies.

	loadRoute and setupLocations() must have been called for this to work.
*/
void RWImporter::importRWTileExtents()
{
	// Given filename is the path to the RouteProperties.xml file, load the track ribbons
	//from the track folder
	if (!routeFilename.isEmpty())
	{
		// get the extent of terrain and base map size on that
		terrainTileMap = findTileExtent(dirTerrain);
		trackTileMap = findTileExtent(dirTrack);
		roadTileMap = findTileExtent(dirRoads);
		sceneryTileMap = findTileExtent(dirScenery);

		// if the track occupies more tiles than terrain, adjust
		overallRect.setWidth(0);
		overallRect.setHeight(0);

		if (terrainTileMap != NULL)
			overallRect = terrainTileMap->getRegion();

		if (trackTileMap != NULL && trackTileMap->getCount() > 0)
			overallRect = overallRect.united(trackTileMap->getRegion());

		if (roadTileMap != NULL && roadTileMap->getCount() > 0)
			overallRect = overallRect.united(roadTileMap->getRegion());

		if (sceneryTileMap != NULL && sceneryTileMap->getCount() > 0)
			overallRect = overallRect.united(sceneryTileMap->getRegion());

		tileWidth = (int)overallRect.width();
		tileHeight = (int)overallRect.height();
	}
}


/*!
	
*/
void RWImporter::importRWTrack()
{
	// Given filename is the path to the RouteProperties.xml file, load the track ribbons
	// from the track folder
	if (!routeFilename.isEmpty())
	{
		this->emitScanTracks();
		// list the files in the folder given
		QDir folder(dirTrack);
		folder.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		QStringList files = folder.entryList(QDir::Files | QDir::Readable);

		trackRibbons->reserve(files.size());
		
		for(int i = 0; i < files.size() && !isAborting(); i++)
		{
			QString &file = files[i];

			if (file.contains(tileFileRegex))
			{
				QString filepath = dirTrack + "/" + file;

				// load the track tile...
				RibbonList *ribbons = importRWTrackTile(filepath);

				// append contents to current ribbons list
				if (ribbons)
					trackRibbons->append(*ribbons);
				
				// delete the container now its contents have moved...
				delete ribbons;
			}
			else
				qxtLog->warning() << "importRWTrack(" << abbreviateFilename(dirTrack) << "): unexpected file \'" << file << "\' in folder.";
		}
	}
}

/*!
	Given filename is the path to the RouteProperties.xml file, load the track ribbons
	from the track folder
*/
void RWImporter::importRWRoads()
{
	if (routeFilename != NULL)
	{
		emitScanRoads();
		// list the files in the folder given
		QDir folder(dirRoads);
		folder.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		QStringList files = folder.entryList(QDir::Files | QDir::Readable);

		roadRibbons->reserve(files.size());
		
		for(int i = 0; i < files.size() && !isAborting(); i++)
		{
			QString &file = files[i];

			if (file.contains(tileFileRegex))
			{
				QString filepath = dirRoads + "/" + file;
				// load the road tile...
				RibbonList *ribbons = importRWRoadTile(filepath);

				// append contents to current ribbons list
				if (ribbons)
					roadRibbons->append(*ribbons);
				
				// delete the container now its contents have moved...
				delete ribbons;
			}
			else
				qxtLog->warning() << "importRWRoads(" << abbreviateFilename(dirRoads) << "): unexpected file \'" << file << "\' in folder.";
		}
	}
}

/*!
	Given filename is the path to the RouteProperties.xml file, load the track ribbons
	from the track folder
*/
void RWImporter::importRWScenery()
{
	if (routeFilename != NULL)
	{
		emitScanScenery();
		// list the files in the folder given
		QDir folder(dirScenery);
		folder.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		QStringList files = folder.entryList(QDir::Files | QDir::Readable);

		scenery->reserve(files.size());
		
		for(int i = 0; i < files.size() && !isAborting(); i++)
		{
			QString &file = files[i];

			if (file.contains(tileFileRegex))
			{
				QString filepath = dirScenery + "/" + file;
				// load the scenery tile...
				SceneryItemList *items = importRWSceneryTile(filepath);

				// append contents to current ribbons list
				if (items)
					scenery->append(*items);
				
				// delete the container now its contents have moved...
				delete items;
			}
			else
				qxtLog->warning() << "importRWRoads(" << abbreviateFilename(dirRoads) << "): unexpected file \'" << file << "\' in folder.";
		}
	}
}

/*!
	Load each terrain tile in the route, adding them to the
	list "terrain". 
*/
void RWImporter::importRWTerrain()
{
	if (routeFilename != NULL)
	{
		emitScanTerrain();
		// list the files in the folder given
		QDir folder(dirTerrain);
		folder.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		QStringList files = folder.entryList(QDir::Files | QDir::Readable);

		terrain->reserve(files.size());
			
		for(int i = 0; i < files.size() && !isAborting(); i++)
		{
			QString &file = files[i];

			if (file.contains(tileFileRegex))
			{
				QString filepath = dirTerrain + "/" + file;
				TerrainArray *ta = importRWTerrainTile(filepath);  // => x,y,tile
				if (ta != NULL)
					terrain->insert(ta->getPosition(), ta);
			}
			else
				qxtLog->warning() << "importRWTerrain(" << abbreviateFilename(dirTerrain) << "): unexpected file \'" << file << "\' in folder.";
		}
	}
}

/*!
	Load a Preload Blueprint file (an xml doc loaded from \a filename and referenced by
	\a node) and read the Route Metadata blueprint (one of the blueprints that can be
	found in such files), storing the information in the returned RouteMetadata object.

	The \a filename is passed for the purposes of error logging.
*/
RouteMetadata *RWImporter::importRWRouteMetadataBlueprint(const QString filename, pugi::xml_node node)
{
	QString tex;
	QRect coordsRect;
	QString nam = getNamedString(filename, node, "Name");
	QString gid = getNamedString(filename, node, "RouteGUID");

	pugi::xpath_node txnode = node.select_single_node("ThumbnailTexture/cMetadataBlueprint-cMetaDataThumbnailTexture");
	if (txnode)
	{
		tex = getNamedString(filename, txnode.node(), "ThumbnailTextureID");
		if (tex.isEmpty())
			qxtLog->warning() << "importRWPreloadFile(" << abbreviateFilename(filename) << "): Metadata ThumbnailTextureID is empty.";
		int texLeft = getNamedInt(filename, txnode.node(), "CoordsLeft");
		int texTop  = getNamedInt(filename, txnode.node(), "CoordsTop");
		int texWidth = getNamedInt(filename, txnode.node(), "CoordsWidth");
		int texHeight= getNamedInt(filename, txnode.node(), "CoordsHeight");
		coordsRect.setTopLeft(QPoint(texLeft, texTop));
		coordsRect.setSize(QSize(texWidth, texHeight));
	}
	else
		qxtLog->warning() << "importRWPreloadFile(" << abbreviateFilename(filename) << "): cMetadataBlueprint not found.";

	pugi::xpath_node flt = node.select_single_node("BrowseFiltering/cBrowseFiltering");
	RouteMetadata::BrowseFilter flags;
	if (flt)
	{
		if (getNamedBool(filename, flt.node(), "Africa"))
			flags |= RouteMetadata::Region_Africa;
		if (getNamedBool(filename, flt.node(), "Asia"))
			flags |= RouteMetadata::Region_Asia;
		if (getNamedBool(filename, flt.node(), "Australia"))
			flags |= RouteMetadata::Region_Australia;
		if (getNamedBool(filename, flt.node(), "Europe"))
			flags |= RouteMetadata::Region_Europe;
		if (getNamedBool(filename, flt.node(), "NorthAmerica"))
			flags |= RouteMetadata::Region_NorthAmerica;
		if (getNamedBool(filename, flt.node(), "SouthAmerica"))
			flags |= RouteMetadata::Region_SouthAmerica;
		if (getNamedBool(filename, flt.node(), "Modern"))
			flags |= RouteMetadata::Era_Modern;
		if (getNamedBool(filename, flt.node(), "Historic"))
			flags |= RouteMetadata::Era_Historic;
		if (getNamedBool(filename, flt.node(), "Heritage"))
			flags |= RouteMetadata::Era_Heritage;
	}
	else
		qxtLog->warning() << "importRWPreloadFile(" << abbreviateFilename(filename) << "): BrowseFiltering/cBrowseFiltering not found.";

	RouteMetadata *rm = new RouteMetadata(nam, gid, tex, flags);
	rm->setTextureCoords(coordsRect);
	return rm;
}

/*!
	
*/
BPMetadataList *RWImporter::importRWPreloadFile(const QString filename)
{
	BPMetadataList *bplist = new BPMetadataList;
	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importRWPreloadFile(" << abbreviateFilename(filename) << "): unable to load: " << serzFileAccess->lastResultStr();
		return bplist;
	}

	pugi::xpath_node_set nodes = xmlDoc->select_nodes("/cBlueprintLoader/Blueprint/*");
	for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end() && !isAborting(); ++it)
	{
		pugi::xpath_node xpnode = *it;
		pugi::xml_node node = xpnode.node();

		if (strcmp(node.name(), "cMetadataBlueprint") == 0)
		{
			pugi::xpath_node mdnode = node.select_single_node("MetaData");
			if (!mdnode)
			{
				qxtLog->error() << "importRWPreloadFile(" << abbreviateFilename(filename) << "): MetaData not found within cMetadataBlueprint.";
				continue;
			}

			pugi::xpath_node_set mdnodes = node.select_nodes("MetaData/*");
			for (pugi::xpath_node_set::const_iterator it_m = mdnodes.begin(); it_m != mdnodes.end() && !isAborting(); ++it_m)
			{
				pugi::xpath_node mdenode = *it_m;

				if (strcmp(mdenode.node().name(), "cMetadataBlueprint-cRouteMetaDataEntry") == 0)
				{
					RouteMetadata *rm = importRWRouteMetadataBlueprint(filename, mdenode.node());
					bplist->append(rm);
				}
				else if (strcmp(mdenode.node().name(), "cMetadataBlueprint-cBlueprintGroupMetaDataEntry") == 0)
				{
				}
				else
					qxtLog->warning() << "importRWPreloadFile(" << abbreviateFilename(filename) << "): Unknown MetaData entry " << mdenode.node().name();
			}
		}
		else if (strcmp(node.name(), "cConsistBlueprint") == 0)
		{}
		else if (strcmp(node.name(), "cConsistFragmentBlueprint") == 0)
		{}
	}
	return bplist;
}

/*!
	Import the contents of a Track tile, returning a list of Ribbons
	representing the track segments loaded.
*/
RibbonList *RWImporter::importRWTrackTile(const QString &filename)
{
	//Import a single track tile from the filesystem into Ribbon form//   
	RibbonList *ribbons = new RibbonList();
	IRibbon *ribbon;
	
	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importRWTrackTile(" << abbreviateFilename(filename) << "): unable to load: " << serzFileAccess->lastResultStr();
		return ribbons;
	}

	pugi::xpath_node_set nodes = xmlDoc->select_nodes("/cRecordSet/Record/Network-cNetworkRibbonUnstreamed-cCurveContainerUnstreamed/*");
	for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end() && !isAborting(); ++it)
	{
		pugi::xpath_node xpnode = *it;
		pugi::xml_node node = xpnode.node();

		if (strcmp(node.name(), "RibbonID") == 0)
		{
			pugi::xpath_node ds = node.select_single_node("Network-cNetworkRibbon-cCurveContainer-cID/RibbonID/cGUID/DevString");
			pugi::xpath_node nt = node.select_single_node("Network-cNetworkRibbon-cCurveContainer-cID/NetworkTypeID/cGUID/DevString");
			if (!ds || !nt)
			{
				if (!nt)
					qxtLog->warning() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): NetworkTypeID/cGUID/DevString element not found.";
				if (!ds)
					qxtLog->warning() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): RibbonID/cGUID/DevString element not found.";
				ribbon = new TrackNetworkRibbon(QString(), QString());
			}
			else
				ribbon = new TrackNetworkRibbon(QString(ds.node().child_value()),
					                            QString(nt.node().child_value()));
			ribbons->append(ribbon);
		}
		else if (strcmp(node.name(), "Curve") == 0)
		{
			pugi::xpath_node_set nodes = node.select_nodes("*");
			for (pugi::xpath_node_set::const_iterator itc = nodes.begin(); itc != nodes.end(); ++itc)
			{
				pugi::xpath_node cnode = *itc;
				pugi::xml_node curve = cnode.node();

				int id = QString(curve.attribute("d:id").value()).toInt();
				if (strcmp(curve.name(), "cCurveStraight") == 0)
				{
					double lng = getNamedDouble(filename, curve, "Length");
					TangentPair t = getStartTangent(curve);
					RWPosition xyc = getPosition(curve.select_single_node("StartPos/cFarVector2").node());

					NetworkCurveStraight *net = new NetworkCurveStraight(xyc, lng, t.Tx(), t.Ty());
					net->setId(id);
					ribbon->append(net);
				}
				else if (strcmp(curve.name(), "cCurveArc") == 0)
				{
					double lng = getNamedDouble(filename, curve, "Length");
					double arc = getNamedDouble(filename, curve, "Curvature");
					int sign = getNamedInt(filename, curve, "CurveSign");
					TangentPair t = getStartTangent(curve);
					RWPosition xyc = getPosition(curve.select_single_node("StartPos/cFarVector2").node());

					NetworkCurveArc *net = new NetworkCurveArc(xyc, lng, t.Tx(), t.Ty(), arc, sign);
					net->setId(id);
					ribbon->append(net);
				}
				else if (strcmp(curve.name(), "cCurveEasement") == 0)
				{
					double lng = getNamedDouble(filename, curve, "Length");
					int sign = getNamedInt(filename, curve, "CurveSign");
					double sharp = getNamedDouble(filename, curve, "Sharpness");
					TangentPair t = getStartTangent(curve);
					RWPosition &xyc = getPosition(curve.select_single_node("StartPos/cFarVector2").node());

					NetworkCurveEasement *net = new NetworkCurveEasement(xyc, lng, t.Tx(), t.Ty(), sharp, sign);
					net->setId(id);
					ribbon->append(net);
				}
			}
		}
		else
			qxtLog->warning() << "importRWTrackTile(" << abbreviateFilename(filename) << "): expecting node : RibbonID or Curve, got " << node.name() ;
	}
	delete xmlDoc;
	QString fname = QFileInfo(filename).fileName();
	QPoint pos = filenameToLocation(fname);
	this->emitLoadTile(TILETYPE_TRACK, pos.x(), pos.y());
	return ribbons;
}


/*!
	Import the contents of a Road tile, returning a list of Ribbons
	representing the road segments loaded.
*/
RibbonList *RWImporter::importRWRoadTile(const QString &filename)
{
	RibbonList *ribbons = new RibbonList();
	IRibbon *ribbon;

	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importRWRoadTile(" << abbreviateFilename(filename) << "): unable to load: " << serzFileAccess->lastResultStr();
		return ribbons;
	}

	pugi::xpath_node_set nodes = xmlDoc->select_nodes("/cRecordSet/Record/Network-cRoadRibbonContainer/Ribbon/*");
	for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end() && !isAborting(); ++it)
	{
		pugi::xpath_node xpnode = *it;
		pugi::xml_node node = xpnode.node();

		if (strcmp(node.name(), "Network-cRoadRibbon") == 0)
		{
			pugi::xpath_node ds = node.select_single_node("RibbonID/cGUID/DevString");
			ribbon = new RoadNetworkRibbon(QString(ds.node().child_value()));
			ribbons->append(ribbon);

			pugi::xpath_node_set nodes = node.select_nodes("Curves/Network-cNetworkRibbonStreamed-cCurveContainerStreamed/Curve/*");
			for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
			{
				pugi::xpath_node xpcurve = *it;
				pugi::xml_node curve = xpcurve.node();

				int id = QString(curve.attribute("d:id").value()).toInt();
				if (strcmp(curve.name(), "cCurveStraight") == 0)
				{
					double lng = getNamedDouble(filename, curve, "Length");
					TangentPair t = getStartTangent(curve);
					RWPosition xyc = getPosition(curve.select_single_node("StartPos/cFarVector2").node());

					NetworkCurveStraight *net = new NetworkCurveStraight(xyc, lng, t.Tx(), t.Ty());
					net->setId(id);
					ribbon->append(net);
				}
				else if (strcmp(curve.name(), "cCurveArc") == 0)
				{
					double lng = getNamedDouble(filename, curve, "Length");
					double arc = getNamedDouble(filename, curve, "Curvature");
					int sign = getNamedInt(filename, curve, "CurveSign");
					TangentPair t = getStartTangent(curve);
					RWPosition xyc = getPosition(curve.select_single_node("StartPos/cFarVector2").node());

					NetworkCurveArc *net = new NetworkCurveArc(xyc, lng, t.Tx(), t.Ty(), arc, sign);
					net->setId(id);
					ribbon->append(net);
				}
			}
		}
		else
			qxtLog->warning() << "importRWRoadTile(" << abbreviateFilename(filename) << "): expecting node : Network-cRoadRibbon, got " << node.name() ;
	}

	delete xmlDoc;
	QString fname = QFileInfo(filename).fileName();
	QPoint pos = filenameToLocation(fname);
	this->emitLoadTile(TILETYPE_ROAD, pos.x(), pos.y());
	return ribbons;
}

/*!
	
*/
SceneryItemList *RWImporter::importRWSceneryTile(const QString &filename)
{
	//Import a single track tile from the filesystem into Ribbon form//   
	SceneryItemList *scenery = new SceneryItemList();

	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importRWTrackTile(" << abbreviateFilename(filename) << "): unable to load: " << serzFileAccess->lastResultStr();
		return scenery;
	}

	pugi::xpath_node_set nodes = xmlDoc->select_nodes("/cRecordSet/Record/*");
	for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end() && !isAborting(); ++it)
	{
		pugi::xpath_node xpnode = *it;
		pugi::xml_node node = xpnode.node();

		if (strcmp(node.name(), "cDynamicEntity") == 0)
		{
			QString blueprint, reskinblueprint, name, eId;
			SceneryItem *si = NULL;
			int detailLevel = 9;

			eId = getNamedString(filename, node, "EntityID/cGUID/DevString", true);

			pugi::xpath_node bp = node.select_single_node("BlueprintID/iBlueprintLibrary-cAbsoluteBlueprintID");
			blueprint = blueprintLocation(filename, bp.node());
			
			pugi::xpath_node rbp = node.select_single_node("ReskinBlueprintID/iBlueprintLibrary-cAbsoluteBlueprintID");
			if (rbp)
				reskinblueprint = blueprintLocation(filename, rbp.node(), true);
			
			pugi::xpath_node cMat = node.select_single_node("Component/cPosOri/RFarMatrix/cFarMatrix");
			double cHeight = getNamedDouble(filename, cMat.node(), "Height");
				
			// there's a 3x4 Matrix here too! : RXAxis, RYAxis, RZAxis

			pugi::xpath_node cLoc = cMat.node().select_single_node("RFarPosition/cFarVector2");
			RWPosition entLoc = getPosition(cLoc.node());

			name = getNamedString(filename, node, "Name");
			//qxtLog->debug() << "Asset Name :\t" << name << " \t" << blueprint;

			// There's a variation: Components can have a CWaterComponent, cTunnelComponent, cDecalComponent,
			// containing Width & Length, instead of the otherwise-default cSceneryRender, cEntityContainer,
			// cCollisionComponent mix.
			// cSoundComponent and cAnalogClockUpdate don't have additional elements.
			//
			pugi::xpath_node cwc =  node.select_single_node("Component/cWaterComponent");
			pugi::xpath_node cabc = node.select_single_node("Component/cAssetBlockComponent");
			pugi::xpath_node cdc =  node.select_single_node("Component/cDecalComponent");
			pugi::xpath_node ctc =  node.select_single_node("Component/cTunnelComponent");
			pugi::xpath_node csc =  node.select_single_node("Component/cSoundComponent");
			pugi::xpath_node covc = node.select_single_node("Component/cOcclusionVolumeComponent");
			pugi::xpath_node cacu = node.select_single_node("Component/cAnalogClockUpdate");
			pugi::xpath_node cdcu = node.select_single_node("Component/cDigitalClockUpdate");
			pugi::xpath_node sr =   node.select_single_node("Component/cSceneryRender");
			pugi::xpath_node aor =  node.select_single_node("Component/cAnimObjectRender");
			pugi::xpath_node asr =  node.select_single_node("Component/cAnimSceneryRender");
			pugi::xpath_node esr =  node.select_single_node("Component/cEditorSceneryRender");
			pugi::xpath_node esar = node.select_single_node("Component/cEditorSoundAreaRender");

			/////// ============= ///////
			si = new SceneryItem(blueprint, entLoc, cHeight);

			if (cwc)
			{
				// has Width and Length.
				double cWidth = getNamedDouble(filename, cwc.node(), "Width");
				double cLength = getNamedDouble(filename, cwc.node(), "Length");
			
				si->setCategory(SceneryItem::Water);
				si->setWidth(cWidth);
				si->setLength(cLength);
			}
			else if (cdc)
			{
				// has Width and Length.
				double cWidth = getNamedDouble(filename, cdc.node(), "Width");
				double cLength = getNamedDouble(filename, cdc.node(), "Length");
			
				si->setCategory(SceneryItem::Decal);
				si->setWidth(cWidth);
				si->setLength(cLength);
			}
			else if (cabc)
			{
				// has Width and Length (and more: Seed, Density, AssetBP, ...)
				double cWidth = getNamedDouble(filename, cabc.node(), "Width");
				double cLength = getNamedDouble(filename, cabc.node(), "Length");
			
				si->setCategory(SceneryItem::Plant);
				si->setWidth(cWidth);
				si->setLength(cLength);
			}
			else if (ctc)
			{
				// has Width and Length
				double cWidth = getNamedDouble(filename, ctc.node(), "Width");
				double cLength = getNamedDouble(filename, ctc.node(), "Length");
			
				si->setCategory(SceneryItem::Tunnel);
				si->setWidth(cWidth);
				si->setLength(cLength);
			}
			else if (csc)
			{
				si->setCategory(SceneryItem::Sound);
			}
			else if (cacu || cdcu || covc)
			{
				si->setCategory(SceneryItem::Other);
			}
			// if not the above, then it's a standard asset. I think!
			else if (!(sr || asr || esr || esar || aor))
			{
				delete si;
				qxtLog->warning() << "Didn't handle node: " << print_xnode(node);
				continue;
			}
			else
			{
				// don't override category set by constructor.
			}

			/////// ============= ///////

			if (sr || asr || esr || esar || aor)
			{
				detailLevel = getNamedInt(filename, sr.node(), "DetailLevel", false);
				si->setDetailLevel(detailLevel);
			}

			/////// ============= ///////

			if (!reskinblueprint.isNull())
				si->setReskinBlueprint(reskinblueprint);
			if (!eId.isNull())
				si->setID(eId);

			/////// ============= ///////

			scenery->append(si);
		}
		else
			qxtLog->warning() << "importRWSceneryTile(" << abbreviateFilename(filename) << "): expecting node : cDynamicEntity, got " << node.name() ;
	}

	delete xmlDoc;
	QString fname = QFileInfo(filename).fileName();
	QPoint pos = filenameToLocation(fname);
	emitLoadTile(TILETYPE_SCENERY, pos.x(), pos.y());
	return scenery;
}

/*!
	Given a "geometry" filename as found in Asset files, such as :
		Kuju\RailSimulator\Scenery\Structures\[00]Concrete_block01
	return
*/
QString RWImporter::assetGeometryToFilename(QString bp, bool appendBin)
{
	QString result;
	QFileInfo bpfi(bp);
	// split the file and path
	QString fname = bpfi.fileName();
	QString foldr = bpfi.path();
	
	// check for and strip the modifiers
	if (fname[0] == '[' && fname[3] == ']')
		fname = fname.mid(4);
	
	// check for and strip .xml at the end
	if (fname.right(4) == ".xml")
		fname = fname.left(fname.size()-4);

	// render using Qt tradition of forward slashes and
	// make lower case (simplifies lookup)
	foldr = foldr.replace("\\", "/").toLower();

	result = "Assets/" + foldr + "/" + fname;

	if (appendBin && fname.right(4) != ".bin")
		result += ".bin";
	
	return result;
}

/*!
	
*/
Asset *RWImporter::importAsset(const QString &filename)
{
	//Import a single terrain tile from the filesystem into data
	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importAsset(" << abbreviateFilename(filename) << "): unable to load: " << serzFileAccess->lastResultStr();
		return NULL;
	}

	pugi::xpath_node bpnode = xmlDoc->select_single_node("/cBlueprintLoader/Blueprint");
	if (!bpnode)
	{
		qxtLog->error() << "importAsset(" << abbreviateFilename(filename) << "): asset blueprint not found.";
		return NULL;
	}

	pugi::xpath_node sbnode = bpnode.node().select_single_node("cSceneryBlueprint");
	if (!sbnode)
	{
		qxtLog->info() << "importAsset(" << abbreviateFilename(filename) << "): not a scenery blueprint.";
		return NULL;
	}

	QString name = getNamedString(filename, sbnode.node(), "Name");

	pugi::xpath_node browsenode = sbnode.node().select_single_node("BrowseInformation/iBrowseableBlueprint-cBrowseInformation");
	QString display = getNamedString(filename, browsenode.node(), "DisplayName/Localisation-cUserLocalisedString/English");
	QString descript = getNamedString(filename, browsenode.node(), "Description/Localisation-cUserLocalisedString/English");
	QString category = getNamedString(filename, browsenode.node(), "Category");
	bool validscen = getNamedBool(filename, browsenode.node(), "ValidInScenarios");

	pugi::xpath_node srbnode = sbnode.node().select_single_node("RenderComponent/cSceneryRenderBlueprint");
	QString geometry  = getNamedString(filename, srbnode.node(), "GeometryID");
	QString cogeometry= getNamedString(filename, srbnode.node(), "CollisionGeometryID");
	bool pickable  = getNamedBool(filename, srbnode.node(), "Pickable");
	QString shadowtype= getNamedString(filename, srbnode.node(), "ShadowType");
	QString viewtype  = getNamedString(filename, srbnode.node(), "ViewType");
	bool instanc  = (bool)getNamedInt(filename, srbnode.node(), "Instancable"); // 0 or 1, not eTrue or eFalse

	Asset *a = new Asset(Asset::fromString(category), display, geometry, name);
	a->setCollisionGeometry(cogeometry);
	a->setPickable(pickable);
	a->setValidScenarios(validscen);
	a->setInstancable(instanc);
	if (shadowtype == "eShadowType_None")
		a->setShadowType(Asset::eShadowType_None);
	else
		a->setShadowType(Asset::eShadowType_Blobby);

	if (viewtype == "ExternalView")
		a->setViewType(Asset::eView_External);
	else
		a->setViewType(Asset::eView_Cab);

	delete xmlDoc;
	return a;
}


/*!
	Import the XML for a terrain tile, which has some initial values followed by a blob
	of hex-coded floats. Currently there are always 65536 floats in the blob, corresponding
	to a 127x127 array covering the 1024m square tile.
*/
TerrainArray *RWImporter::importRWTerrainTile(const QString &filename)
{
	//Import a single terrain tile from the filesystem into data
	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): unable to load: " << serzFileAccess->lastResultStr();
		return NULL;
	}

	// it's valid for the tile file to contain no height at all; just <cRecordSet><Record/></cRecordSet>
	pugi::xpath_node xpnode = xmlDoc->select_single_node("/cRecordSet/Record");
	if (!xpnode || !xpnode.node().first_child())
	{
		qxtLog->info() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): terrain tile is empty.";
		return NULL;
	}

	xpnode = xmlDoc->select_single_node("/cRecordSet/Record/cHeightFieldTile");
	if (!xpnode)
	{
		qxtLog->warning() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): cHeightFieldTile element not found.";
		return NULL;
	}
	pugi::xpath_node xnode = xpnode.node().select_single_node("X");
	pugi::xpath_node ynode = xpnode.node().select_single_node("Y");
	int tx = QString(xnode.node().child_value()).toInt();
	int ty = QString(ynode.node().child_value()).toInt();

	QString fname = QFileInfo(filename).fileName();
	QPoint pos = filenameToLocation(fname);
	if (pos.x() != tx || pos.y() != ty)
	{
		QString err("tile should have coords %2,%3 but is actually %4,%5");
		err = err.arg(pos.x()).arg(pos.y()).arg(tx).arg(ty);
		qxtLog->error() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): " << err << ".";
	}
	pugi::xpath_node blob = xpnode.node().select_single_node("d:blob");
	if (!blob)
	{
		qxtLog->warning() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): d:blob element not found.";
		return NULL;
	}
	
	TerrainArray *tile;
	tile = new TerrainArray(QPoint(tx,ty), TILE_SIZE_TERRAIN, TILE_SIZE_TERRAIN);
	int num = QString(blob.node().attribute("d:size").value()).toInt();
	if (num == 65536)
	{
		// next element from stream
		int i = 0;
		float f1, f2;
		// data for tile as it comes in...
		const pugi::char_t *str = blob.node().child_value();
		QString blobStr(str);
		QStringList values = blobStr.split(QRegExp("[ \\t\\n]"), QString::SkipEmptyParts);

		for(int y = 0; y < TILE_SIZE_TERRAIN && !isAborting(); y++) // 0..127
		{
			for(int x = 0; x < TILE_SIZE_TERRAIN; x+=2) // 0..63
			{
				if (i >= values.size())
				{
					qxtLog->error() << "importRWTerrainTile(" << abbreviateFilename(filename) << "): d:blob ran out of data after " << i << " values.";
					goto endvaluestream;
				}

				QString &v = values[i++];

				// Value 1 - chars 0..7
				QString el1(v.left(8));
				f1 = convertHexFloat(el1);

				// Value 2 - chars 8..15
				QString el2(v.mid(8,8));
				f2 = convertHexFloat(el2);

				updateMinMaxHeight(f1, tx, ty);
				updateMinMaxHeight(f2, tx, ty);

				tile->at(x,y) = hvalFromFloat(f1);
				tile->at(x+1,y) = hvalFromFloat(f2);
			}
		}
	}
endvaluestream:

	delete xmlDoc;
	
	emitLoadTile(TILETYPE_TERRAIN, tx, ty);
	return tile;
}



/*!
	Import the XML for tracks.bin.
*/
RWTrackInfoList *RWImporter::importRWTracks(const QString &filename)
{
	RWTrackInfo *networkTrackInfo;
	QString networkID;

	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importRWTracks(" << abbreviateFilename(filename) << "): unable to load: " << serzFileAccess->lastResultStr();
		return NULL;
	}

	// TODO: I suspect that it's possible to have multiple /cRecordSet/Record/Network-cTrackNetwork elements
	// in a file, one per network represented.

	// Read in the network ID
	pugi::xpath_node nid = xmlDoc->select_single_node("/cRecordSet/Record/Network-cTrackNetwork/NetworkID/cGUID/DevString");
	if (!nid)
	{
		qxtLog->warning() << "importRWTracks(" << abbreviateFilename(filename) << "): NetworkID/cGUID/DevString element not found.";
	}
	else
		networkID = nid.node().child_value();

    pugi::xpath_node ribboncontainer = xmlDoc->select_single_node("/cRecordSet/Record/Network-cTrackNetwork/RibbonContainer/Network-cRibbonContainerUnstreamed");
	if (!ribboncontainer)
	{
		qxtLog->warning() << "importRWTracks(" << abbreviateFilename(filename) << "): unable to find Ribbons";
        delete xmlDoc;
		return NULL;
	}

	// Make a TrackInfo element, which forms the base for the ribbons and nodes lists.
	networkTrackInfo = new RWTrackInfo(networkID);
	pugi::xpath_node_set ribbons = ribboncontainer.node().select_nodes("Ribbon/*");
	for (pugi::xpath_node_set::const_iterator rit = ribbons.begin(); rit != ribbons.end() && !isAborting(); ++rit)
	{
		pugi::xpath_node xpnode = *rit;
		pugi::xml_node node = xpnode.node();
		TrackRibbon *trackRibbon = NULL;

		if (strcmp(node.name(), "NetworkRibbon-cTrackRibbon") == 0)
		{
			QString ribbonID;
			trackRibbon = new TrackRibbon();

			// Get the RibbonID for this ribbon
			pugi::xpath_node rid = node.select_single_node("RibbonID/cGUID/DevString");
			if (!rid)
			{
				qxtLog->warning() << "importRWTracks(" << abbreviateFilename(filename) << "): RibbonID/cGUID/DevString element not found.";
			}
			else
				ribbonID = rid.node().child_value();

			// RibbonID
			trackRibbon->setRibbonID(ribbonID);

			// _length
			trackRibbon->setRibbonLength(getNamedDouble(filename, node, "_length"));

            // height map
			pugi::xpath_node_set nodes = ribboncontainer.node().select_nodes("Height/*");
			for (pugi::xpath_node_set::const_iterator hit = nodes.begin(); hit != nodes.end() && !isAborting(); ++hit)
			{
				pugi::xpath_node hpnode = *hit;
				pugi::xml_node hnode = hpnode.node();
				if (strcmp(node.name(), "Network-iRibbon-cHeight") == 0)
				{
					HeightPoint *point = new HeightPoint;
					point->height = getNamedDouble(filename, node, "_height");
					point->position = getNamedDouble(filename, node, "_position");
					point->manual = getNamedBool(filename, node, "_manual");
					trackRibbon->addHeight(point);
				}
			}

            // routevector (unused)

            // Rbottomleft/RExtents

            // fixedapatternref

            // lockcounterwhenmodified
			trackRibbon->setLockCounterWhenMod(getNamedBool(filename, node, "LockCounterWhenModified"));

			// properties

            // explicitdirection

            // superelevated
			trackRibbon->setSuperElevated(getNamedBool(filename, node, "Superelevated"));
		}
    }

	pugi::xpath_node_set tracknodes = ribboncontainer.node().select_nodes("Node/*");
	for (pugi::xpath_node_set::const_iterator nit = tracknodes.begin(); nit != tracknodes.end() && !isAborting(); ++nit)
	{
		pugi::xpath_node xpnode = *nit;
		pugi::xml_node node = xpnode.node();
        QString ribbonID;

		if (strcmp(node.name(), "Network-cTrackNode") == 0)
		{
            // sRconnection

	        pugi::xpath_node_set nodes = node.select_nodes("Network-cNetworkNode-sRConnection/*");
	        for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end() && !isAborting(); ++it)
	        {
		        pugi::xpath_node xpnode = *it;
		        pugi::xml_node node = xpnode.node();
                QString ribbonID;

			    pugi::xpath_node rid = node.select_single_node("_id/cGUID/DevString");
			    if (!rid)
			    {
				    qxtLog->warning() << "importRWTracks(" << abbreviateFilename(filename) << "): NetworkID/cGUID/DevString element not found.";
			    }
			    else
				    ribbonID = rid.node().child_value();

            }

            // fixedapatternref
	        pugi::xpath_node fixpat = node.select_single_node("FixedPatternRef/Network-cNetworkNode-sFixedPatternRef/FixedPatternNodeIndex");

            // routevector (unused)

            // patternref
	        pugi::xpath_node_set patterns = node.select_nodes("PatternRef/Network-cNetworkNode-sPatternRef/*");
	        for (pugi::xpath_node_set::const_iterator pit = patterns.begin(); pit != patterns.end() && !isAborting(); ++pit)
	        {
		        pugi::xpath_node xpnode = *pit;
		        pugi::xml_node node = xpnode.node();
                int nodeIndex;
                QString nodeName = node.name();
                TrackPattern *pattern;

        		int id = QString(node.attribute("d:id").value()).toInt();

		        if (nodeName == "PatternNodeIndex")
		        {
					nodeIndex = getNamedInt(filename, node, "PatternNodeIndex");
                }
		        else if (nodeName == "d:nil")
		        {
					pattern = new TrackPattern();
                    pattern->setRefID((IDType)id);
                }
                else if (nodeName.left(9) == "Network-c" && nodeName.right(7) == "Pattern")
                {
        			int id = QString(node.attribute("d:id").value()).toInt();
                    int manual = getNamedInt(filename, node, "Manual");
                    QString state = getNamedString(filename, node, "State");
                    double transTime = getNamedDouble(filename, node, "TransitionTime");
                    QString pState = getNamedString(filename, node, "PreviousState");

                    if (nodeName == "Network-cTurnoutPattern")
                    {
                        pattern = new TurnoutPattern();
                    }
                    else if (nodeName == "Network-c3WayPattern")
                    {
                        pattern = new ThreeWayPattern();
                    }
                    else if (nodeName == "Network-cSingleSlipPattern")
                    {
                        pattern = new SingleSlipPattern();
                    }
                    else if (nodeName == "Network-cDoubleSlipPattern")
                    {
                        pattern = new DoubleSlipPattern();
                    }
                    else if (nodeName == "Network-cCrossingPattern")
                    {
                        pattern = new CrossingPattern();
                    }
                    else 
                    {
            			qxtLog->warning() << "importRWTracks(" << abbreviateFilename(filename) << "): expecting track Pattern, got " << nodeName ;
                    }
                }
            }
		}
    }

    {
	}
}








// this bit uses the MS-sponsored DirectXTex shared source library from http://directxtex.codeplex.com/
#include "DirectXTex.h"

namespace DirectX {
void _CopyScanline( _When_(pDestination == pSource, _Inout_updates_bytes_(outSize))
                    _When_(pDestination != pSource, _Out_writes_bytes_(outSize))
                    LPVOID pDestination, _In_ size_t outSize, 
                    _In_reads_bytes_(inSize) LPCVOID pSource, _In_ size_t inSize,
                    _In_ DXGI_FORMAT format, _In_ DWORD flags );
};


static DXGI_FORMAT imgFormat_RSCtoDXGI(const QString format)
{
	DXGI_FORMAT dxgi_format;
	if (format == "HC_IMAGE_FORMAT_COMPRESSED")
		dxgi_format = DXGI_FORMAT_BC1_UNORM;
	else if (format == "HC_IMAGE_FORMAT_COMPRESSED_INTERP_ALPHA")
		dxgi_format = DXGI_FORMAT_BC2_UNORM;
	else if (format == "HC_IMAGE_FORMAT_COMPRESSED_EXPL_ALPHA")
		dxgi_format = DXGI_FORMAT_BC2_UNORM;
	else if (format == "HC_IMAGE_FORMAT_COL888")
		dxgi_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (format == "HC_IMAGE_FORMAT_COLA8888")
		dxgi_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	else
		dxgi_format = DXGI_FORMAT_UNKNOWN;
	return dxgi_format;
}

/*!
	
*/
RWTexture *RWImporter::importRWTexture(const QString filename)
{
	//Import a single terrain tile from the filesystem into data
	pugi::xml_document *xmlDoc = serzFileAccess->loadSerzFile(filename);
	if (xmlDoc == NULL)
	{
		qxtLog->warning() << "importRWTexture(" << abbreviateFilename(filename) << "): unable to load xml file " << serzFileAccess->lastResultStr();
		return NULL;
	}

	pugi::xpath_node xpnode = xmlDoc->select_single_node("/cHcTextureGroup/Texture/cHcTexture");
	if (!xpnode)
	{
		qxtLog->warning() << "importRWTexture(" << abbreviateFilename(filename) << "): cHcTexture element not found.";
		delete xmlDoc;
		return NULL;
	}
	// base/largest size
	int o_tw = getNamedInt(filename, xpnode.node(), "Width");
	int o_th = getNamedInt(filename, xpnode.node(), "Height");
	QString name = getNamedString(filename, xpnode.node(), "Name");

	pugi::xpath_node texData = xpnode.node().select_single_node("Data/cHcTextureData");
	if (!texData)
	{
		qxtLog->warning() << "importRWTexture(" << abbreviateFilename(filename) << "): cHcTextureData element not found.";
		delete xmlDoc;
		return NULL;
	}
	// Mip, TextureBlob, Palette available here
	// loop over the MIPs (= multiple images in diff resolns)
	pugi::xpath_node_set imageDx_set = texData.node().select_nodes("Mip/*");
	
	RWTexture *rwTex = new RWTexture(name, QSize(o_tw, o_th));
	int numMips = imageDx_set.size();
	DirectX::Image *imgSet = new (std::nothrow) DirectX::Image[numMips];
	int mipNum = 0;
	for (pugi::xpath_node_set::const_iterator it = imageDx_set.begin(); it != imageDx_set.end(); ++it, ++mipNum)
	{
		pugi::xpath_node imageDx = *it;

		if (strcmp(imageDx.node().name(), "cHcImageDx") == 0)
		{
			DXGI_FORMAT dxgi_format;
			QString format = getNamedString(filename, imageDx.node(), "Format");
			dxgi_format = imgFormat_RSCtoDXGI(format);

			if (dxgi_format == DXGI_FORMAT_UNKNOWN)
			{
				qxtLog->warning() << "importRWTexture(" << abbreviateFilename(filename) << "): Image format " << format << " unknown: cannot import";
				delete imgSet;
				delete xmlDoc;
				return NULL;
			}

			int tw = getNamedInt(filename, imageDx.node(), "Width");
			int th = getNamedInt(filename, imageDx.node(), "Height");
			int swizz = getNamedInt(filename, imageDx.node(), "IsSwizzled");
			if (swizz != 0)
				qxtLog->warning() << "importRWTexture(" << abbreviateFilename(filename) << "): IsSwizzled being nonzero is not handled";

			pugi::xpath_node blob = imageDx.node().select_single_node("d:blob");
			int num = QString(blob.node().attribute("d:size").value()).toInt();  // bytes
			// data for tile as it comes in...
			const pugi::char_t *str = blob.node().child_value();
			QString blobStr(str);
			QStringList values = blobStr.split(QRegExp("[ \\t\\n]"), QString::SkipEmptyParts);
			
			// get some memory for the data
			long *pixelData = new (std::nothrow) long[values.size()*2];
			long *pdPtr = pixelData;
			long pixelsLen;

			for(int w = 0; w < values.size(); w++)
			{
				QString &v = values[w];
				if (v.size() > 16)
					throw std::logic_error("Value too large for 2-hex long");

				// now deal with the data
				QString el1(v.left(8));		// Value 1 - chars 0..7
				*pdPtr++ = convertHexLong(el1);

				QString el2(v.mid(8,8));	// Value 2 - chars 8..15
				*pdPtr++ = convertHexLong(el2);
			}
			pixelsLen = pdPtr - pixelData;

			imgSet[mipNum].width = tw;
			imgSet[mipNum].height = th;
			imgSet[mipNum].format = dxgi_format;
			imgSet[mipNum].pixels = (uint8_t *)pixelData;
			DirectX::ComputePitch(dxgi_format, tw, th, imgSet[mipNum].rowPitch, imgSet[mipNum].slicePitch);
		}
	}
	//DirectX::ScratchImage sc;
	//HRESULT hr = sc.InitializeArrayFromImages(imgSet, numMips, false);

	for(int mip=0; mip < numMips; mip++)
	{
		DirectX::ScratchImage decomp;
		//DirectX::Decompress(sc.GetImages()[mip], DXGI_FORMAT_R8G8B8A8_UNORM, decomp);
		DirectX::Decompress(imgSet[mip], DXGI_FORMAT_R8G8B8A8_UNORM, decomp);
		
		delete imgSet[mip].pixels; // delete what was called pixelData above; it's now in decomp.
		
		const DirectX::Image dpImg = decomp.GetImages()[0];
		const DirectX::TexMetadata &tmd = decomp.GetMetadata();
		QSize imgSz(dpImg.width, dpImg.height);
		QImage::Format qformat = tmd.IsPMAlpha() ? QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32;

		QImage *texImg = new QImage(imgSz, qformat);

		for(uint row= 0; row < dpImg.height; row++)
		{
			uint8_t* pDestination = texImg->scanLine(row);
			uint8_t* pSource = dpImg.pixels + (row * dpImg.rowPitch);
			size_t outSize = texImg->bytesPerLine();
			size_t inSize = dpImg.rowPitch;

			//DirectX::_CopyScanline( pDestination, outSize, pSource, inSize, tmd.format, 0 );
			{
                const uint32_t alpha = 0xff000000;

                const uint32_t * __restrict sPtr = reinterpret_cast<const uint32_t*>(pSource);
                uint32_t * __restrict dPtr = reinterpret_cast<uint32_t*>(pDestination);
                size_t size = std::min<size_t>( outSize, inSize );
                for( size_t count = 0; count < size; count += 4 )
                {
                    uint32_t t = *(sPtr++) & 0xFFFFFF;
					// swap R and B over: ABGR => ARGB
					t = ((t & 0xFF0000) >> 16) | ((t & 0xFF) << 16) | (t & 0xFF00) | alpha;
                    *(dPtr++) = t;
                }
			}
		}
		QPixmap *pxmImg = new QPixmap(QPixmap::fromImage(*texImg, Qt::NoOpaqueDetection));
		delete texImg;
		rwTex->add(imgSz, pxmImg);
	}
	delete imgSet;
	delete xmlDoc;

	return rwTex;
}

static BPMetadataList *filterByGUID(BPMetadataList *orig, QString routeGuid)
{
	BPMetadataList *newList = new BPMetadataList;
	for (int i = 0; i < orig->size(); i++)
	{
		BPMetadata *item = orig->at(i);
		if (item->getClass() == BPClass_Route)
		{
			RouteMetadata *rd = (RouteMetadata *)item;
			if (rd->getGUID() == routeGuid)
			{
				newList->append(new RouteMetadata(*rd));
			}
		}
	}
	return newList;
}

/*!
	Return the route icon for the selected route. If there is a choice of
	sizes, select the next largest one from minSize;
*/
QPixmap *RWImporter::getRouteImage(QString route, QString routeGuid, QSize minSize)
{
	BPMetadataList *possibles = filterByGUID(preloadList, routeGuid);
	if (possibles->size() == 0)
		return NULL;

	// should only be one in the list...
	RouteMetadata *rd = (RouteMetadata *)possibles->at(0);
	QString texFile = rd->getImageFilename(dirAssets);
	if (texFile.isEmpty())
		return NULL;

	RWTexture *tex = importRWTexture(texFile);
	QPixmap *pix = tex->findLarger(minSize);
	return pix;
}


/*!
	Return the route icon for the selected route. If there is a choice of
	sizes, select the next largest one from minSize;
*/
RouteMetadata *RWImporter::getRouteData(QString route, QString routeGuid)
{
	BPMetadataList *possibles = filterByGUID(preloadList, routeGuid);
	if (possibles->size() == 0)
		return NULL;

	// should only be one in the list...
	RouteMetadata *rd = (RouteMetadata *)possibles->at(0);
	return rd;
}


float RWImporter::getTerrainHeightAt(const RWPosition &pos) const
{
	QPoint pt = pos.getRoute();
	if (terrain->contains(pt))
	{
		TerrainArray *ta = terrain->value(pt);
		QPointF loc = pos.getTile();  // in tile units, not metres
		QPoint ofs = ta->fromMetres(loc); 
		HVal v = ta->at(ofs);
		return floatFromHval(v);
	}
	return 0;
}
