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

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QtCore/QRectF>
#include <QtCore/QRect>

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QPainterPath>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <QtGui/QWidget>
#include <QtGui/QBrush>

#include "Network.h"
#include "scale.h"
#include "TileMaps.h"
#include "Importer.h"


class MapWidget : public QGraphicsView
{
	Q_OBJECT;

public:
	enum GridType
    {
        GridType_Track = 1,
        GridType_Terrain = 2
    };

private:
    QGraphicsScene *scene;
    double tileAddrScale;
    QPen *gridPen;
    QPen *originPen;
    QPen *trackPen;
    QPen *roadPen;
    QPen *constructPen;//unused
    QBrush *originBrush;
    QBrush *dotBrush;
    QPen *textPen;
    QFont *textFont;//unused
    QBrush *tileAddrBrush;
    QFont *tileAddrFont;

    int inUpdate;  // positive if in update, 0 for not.

    RibbonList *tracknet;
    RibbonList *roadnet;

    QGraphicsItemGroup *gridGroup;
    QGraphicsItemGroup *trackGroup;
    QGraphicsItemGroup *roadGroup;
    QGraphicsItemGroup *terrainGroup;
	QGraphicsItemGroup *originGroup;

    // Terrain is a dictionary containing pixmaps of terrain tiles.
    TerrainTileList *terrain;

    // The extent indicates the overall size of the route, in tiles.
    QImage *terrainExtentBitmap;
    QRectF terrainRect;   // UNITS => TILES
    QImage *trackExtentBitmap;
    QRectF trackRect;
    QImage *roadExtentBitmap;
    QRectF roadRect;

    bool showorigin;
    bool showgrid;
    bool showterrain;
    bool showtrack;
    bool showroads;
    bool showaddrs;
    GridType gridType;


public:
	MapWidget(QWidget *parent = NULL);
	~MapWidget();

    QGraphicsScene* getScene();

    void clear();

    // Required or Optional Data

    void setTerrainTileBitmap(QImage *bitmap, QRectF rect);
    void setTrackTileBitmap(QImage *bitmap, QRectF rect);
    void setRoadTileBitmap(QImage *bitmap, QRectF rect);

    void setTrackNetwork(RibbonList *net);
    void setRoadNetwork(RibbonList *net);

    void setTerrain(TerrainTileList *tileList);

	// Configuring map style
	const QPen &getGridPen();
	void setGridPen(const QPen &);
	
	const QPen &getTrackPen();
	void setTrackPen(const QPen &);
	
	const QPen &getRoadPen();
	void setRoadPen(const QPen &);

	const QPen &getOriginPen();
	void setOriginPen(const QPen &);

	const QBrush &getGridTextBrush();
	void setGridTextBrush(const QBrush &);

	const QFont &getGridTextFont();
	void setGridTextFont(const QFont &);

	const QFont &getTextFont();
	void setTextFont(const QFont &);

	// Show or hide elements
	void showGridlines(bool visible);
    void showOrigin(bool visible);
    void showTerrain(bool visible);
    void showTrack(bool visible);
    void showRoads(bool visible);
    void showGridType(GridType gty);
    void showTileAddrs(bool visible);

	GridType getGridType() { return gridType; };

    // Zoom related
    void zoomTo(double amt);
    void zoomTo(QScale amt);
    QScale currentZoom();
    void zoomIn();
    void zoomOut();
    void zoomAll();
    void zoomWidth();
    void zoomHeight();

    // Painting

    void beginUpdate();
    void endUpdate();
    bool isInUpdate();
    void createScene();

    static QPointF scaleTilesToMetres(QPointF xy);
    static QPointF adjustBitmapCentre(QImage *bitmap, QPointF xy);

private:
    void initPens();
    void clearScene();
    void resetScene();

    double scaleTerrain();
    QGraphicsItemGroup *tileGrid(QGraphicsItemGroup *gridGroup);

    QGraphicsItemGroup *drawTerrain(QGraphicsItemGroup *terrainGroup);
    QGraphicsItemGroup *drawTracknet(QGraphicsItemGroup *trackGroup);
    QGraphicsItemGroup *drawRoadnet(QGraphicsItemGroup *roadGroup);
	QGraphicsItemGroup *routeOrigin(QGraphicsItemGroup *originGroup);

protected:
	void wheelEvent(QWheelEvent * ev);
};

#endif
