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

#ifndef IMPORTER_H
#define IMPORTER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QRectF>


#include "pugixml.hpp"
#include "Settings.h"
#include "Network.h"
#include "RWPosition.h"
#include "TileMaps.h"
#include "terrainarray.h"
#include "ColourMapper.h"
#include "GradGGR.h"
#include "range.h"
#include "serzfile.h"
#include "TangentPair.h"
#include "Pair.h"
#include "blueprint.h"
#include "rwtexture.h"
#include "rwlocations.h"
#include "blueprint.h"
#include "qxtlogger.h"
#include "sceneryitem.h"
#include "asset.h"
#include "rwtracks.h"

class RWImporter : public QObject
{
	Q_OBJECT;

    QString routeFilename;
    QString RouteName;
    QString RouteBlueprint;
    QString TerrainBlueprint;
    double OriginLatitude;
    double OriginLongitude;
    QString dirTrack;
    QString dirRoads;
    QString dirLofts;
    QString dirScenery;
    QString dirTerrain;
    QString dirMixMap;
	QString dirAssets;
    int tileWidth;
    int tileHeight;
        
	SerzFile *serzFileAccess;

    QRectF overallRect;
	double overallMinHeight, overallMaxHeight;
	int minHeightTx, maxHeightTx;
	int minHeightTy, maxHeightTy;

    TileExtentMap *terrainTileMap;
    TileExtentMap *trackTileMap;
    TileExtentMap *roadTileMap;
    TileExtentMap *sceneryTileMap;

    RibbonList *roadRibbons;
    RibbonList *trackRibbons;
    TerrainArrayList *terrain;
	SceneryItemList *scenery;
	BPMetadataList *preloadList;

	bool abortFlag;

	QString serzPath;
	bool serzAuto;
	QRegExp tileFileRegex;

public:
	RWImporter();
	~RWImporter();

	void resetAbort();
	bool isAborting();

	QString getSerzExe() const;

	void setSerzPath(QString &p);
	QString getSerzPath() const;

    void setTempPath(QString &p);
    QString getTempPath() const;

	void setSerzAuto(bool a);
	bool getSerzAuto();

public Q_SLOTS:
	void abort();

signals:
    // Signal to indicate progress on loading track tiles.
    void loadTile(TileType, int, int);
    void scanTerrain();
    void scanTracks();
    void scanRoads();
    void scanScenery();
	void scanPreloads(int n);
    void loadPreload(QString n);

protected:
    virtual void emitScanTerrain();
    virtual void emitScanTracks();
    virtual void emitScanRoads();
    virtual void emitScanScenery();
    virtual void emitLoadTile(TileType ty, int x, int y);

    float convertHexFloat(QString &hex);
	long convertHexLong(QString &hex);
    RWCoordinate getCoordinate(pugi::xml_node& node);
    RWPosition getPosition(pugi::xml_node& node);
	TangentPair getStartTangent(pugi::xml_node& root);
	Pair getPair(const QString &filename, pugi::xml_node &root, char *name);

	double getNamedDouble(const QString &filename, pugi::xml_node &root, char *name, bool missingOK = false);
	int getNamedInt(const QString &filename, pugi::xml_node &root, char *name, bool missingOK = false);
	bool getNamedBool(const QString &filename, pugi::xml_node &node, char *name, bool missingOK = false);
	QString getNamedString(const QString &filename, pugi::xml_node &node, char *name, bool missingOK = false);
    QString blueprintLocation(const QString &filename, pugi::xml_node& node, bool emptyOK = false);

public:
	void clear();
	void clearPreloads();
    void loadRoute(QString &filename);

    QString getRouteName();
    QString getRouteBlueprint();
    QString getTerrainBlueprint();

    QPointF getOrigin();
	QRange getMinMaxHeight();
    QSize getTileExtent();
	QRectF getTiledRegion();

    int getTerrainTiles();
    int getTrackTiles();
    int getRoadTiles();
	int getSceneryTiles();

	float getTerrainHeightAt(const RWPosition &p) const;

    TileExtentMap *getTerrainTileMap();
    TileExtentMap *getTrackTileMap();
    TileExtentMap *getRoadTileMap();
    TileExtentMap *getSceneryTileMap();

    TerrainArrayList *getTerrain();
    SceneryItemList *getScenery();
    RibbonList *getTrackRibbons();
    RibbonList *getRoadRibbons();
	BPMetadataList *getPreloads();
    
	QPoint filenameToLocation(QString &filename);
    void importRWRouteProps();
    void importRWTileExtents();
    void importRWTrack();
    void importRWRoads();
    void importRWScenery();
    void importRWTerrain();
	void importRWPreloads(RWLocations &rwloc = RWLocations());
	Asset *importAsset(const QString &filename);
	QString assetGeometryToFilename(QString bp, bool appendBin = true);

	QPixmap *getRouteImage(QString route, QString routeGuid, QSize minSize);
	RouteMetadata *getRouteData(QString route, QString routeGuid);

private:
    TileExtentMap *findTileExtent(QString &tileDir);
    void setupLocations(QString &basedir);
	void updateMinMaxHeight(double v, int x, int y);

	RWTrackInfoList *RWImporter::importRWTracks(const QString &filename);
    RibbonList *importRWTrackTile(const QString &filename);
    RibbonList *importRWRoadTile(const QString &filename);
    TerrainArray *importRWTerrainTile(const QString &filename);
	SceneryItemList *importRWSceneryTile(const QString &filename);
	RWTexture *importRWTexture(const QString filename);
	BPMetadataList *importRWPreloadFile(const QString filename);
	RouteMetadata *importRWRouteMetadataBlueprint(const QString filename, pugi::xml_node node);
	void setupSerzPaths();
};

#endif
