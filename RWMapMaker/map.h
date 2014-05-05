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

#include <QtGui/QPainterPath>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <QtGui/QBrush>

#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QRubberBand>
#include <QtWidgets/QGraphicsSimpleTextItem>

#include "Network.h"
#include "scale.h"
#include "TileMaps.h"
#include "Importer.h"
#include "GraphicsItems.h"
#include "terrainarray.h"


class MapWidget : public GraphicsOverlayView
{
	Q_OBJECT;

public:
	enum GridType
    {
        GridType_Track = 1,
        GridType_Terrain = 2
    };

	enum CursorMode
	{
		Cursor_Hand = 1,
		Cursor_Select = 2,
	};

	enum WheelZoomDir
	{
		Zoom_InOut,
		Zoom_OutIn
	};

	enum ViewScaling
	{
		Scale_None,			// fixed size
		Scale_WithView,		// size depends on zoom
		Scale_Exaggerated	// size depends on zoom but larger when far away
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
    QFont *textFont;
    QBrush *tileAddrBrush;
    QFont *tileAddrFont;

    int inUpdate;  // positive if in update, 0 for not.

    RibbonList *tracknet;
    RibbonList *roadnet;

    GraphicsLayerItem *gridLayer;
    GraphicsLayerItem *trackLayer;
    GraphicsLayerItem *roadLayer;
    GraphicsLayerItem *terrainLayer;
    GraphicsLayerItem *sceneryLayer;
	GraphicsLayerItem *originLayer;

    // Terrain is a dictionary containing pixmaps of terrain tiles.
    TerrainArrayList *terrainArray;
	SceneryItemList* scenery;

    TerrainTileList terrainImages;

    // The extent indicates the overall size of the route, in tiles.
    QImage *terrainExtentBitmap;
    QRectF terrainRect;   // UNITS => TILES
    QImage *trackExtentBitmap;
    QRectF trackRect;
    QImage *roadExtentBitmap;
    QRectF roadRect;

	QPixmap *scaleOverlay;

    bool showorigin;
    bool showgrid;
    bool showterrain;
    bool showtrack;
    bool showroads;
    bool showaddrs;
	bool showmapscale;
	bool showscenery;
	bool showloftends;
    GridType gridType;

	bool fastrender;
	bool mouseUpdates;
	qint64 prevMouseTime;

	QRubberBand *rubberBand;
	QRect selectedRegion;
	QPoint selectOrigin;
	bool rubberBanding;

	ViewScaling trackScaleMode;
	float trackScale;
	ViewScaling roadScaleMode;
	float roadScale;

	CursorMode cmode;
	WheelZoomDir zoomdir;

    ColourMapper *mapper;
#ifdef RWMM_HOVER
	QObject *descMsgObj;
	const char *descMsgSlot;
#endif

public:
	MapWidget(QWidget *parent = NULL);
	~MapWidget();

    QGraphicsScene* getScene();

    void clear();

	bool selectionValid();
	QRect selection();
	QPolygonF selectionAsScene();

    // Required or Optional Data

    void setTerrainTileBitmap(QImage *bitmap, QRectF rect);
    void setTrackTileBitmap(QImage *bitmap, QRectF rect);
    void setRoadTileBitmap(QImage *bitmap, QRectF rect);

    void setTrackNetwork(RibbonList *net);
    void setRoadNetwork(RibbonList *net);

    void setTerrain(TerrainArrayList *tileList);
	void setScenery(SceneryItemList* scenery);

    void setMapGradient(ColourMapper *g);

#ifdef RWMM_HOVER
	// fn is a slot to connect the desctription() signal to
	void setDescriptionMessageFn(QObject *obj, const char *slotname);
#endif
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

	ViewScaling getTrackScaling() { return trackScaleMode; }
	void setTrackScaling(ViewScaling s);

	ViewScaling getRoadScaling() { return roadScaleMode; }
	void setRoadScaling(ViewScaling s);

	// Show or hide elements
	void showGridlines(bool visible);
	bool gridlinesShown() { return showgrid; }

    void showOrigin(bool visible);
	bool originShown() { return showorigin; }

    void showTerrain(bool visible);
	bool terrainShown() { return showterrain; }

    void showTrack(bool visible);
	bool trackShown() { return showtrack; }

    void showRoads(bool visible);
	bool roadsShown() { return showgrid; }

    void showTileAddrs(bool visible);
	bool tileAddrsShown() { return showaddrs; }

	void showMapScale(bool visible);
	bool mapScaleShown() { return showmapscale; }

	void showScenery(bool visible);
	bool sceneryShown() { return showscenery; }

	void showLoftEnds(bool visible);
	bool loftEndsShown() { return showloftends; }

    void setGridType(GridType gty);
	GridType getGridType() { return gridType; };

	void setFastRender(bool fast);
	bool isFastRender() { return fastrender; }

	void setWheelZoom(WheelZoomDir dir) { zoomdir = dir; }
	WheelZoomDir getWheelZoom() { return zoomdir; }

	void setMouseUpdates(bool enable);
	bool getMouseUpdates() { return mouseUpdates; }

	void setCursorMode(CursorMode cm);
	CursorMode getCursorMode() { return cmode; }

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

public slots:
	//void reportInfo(QString line1, QString line2, QString line3);

signals:
	void zoomChanged();
	void zoomChangedRedraw();

	void mousePos(QPointF p);

private:
    void initPens();
    void clearScene();
    void resetScene();

    double scaleTerrain();

#ifdef RWMM_HOVER
	void applyDescriptionMessageFn(GraphicsLayerItem *layer);
#endif

    GraphicsLayerItem *tileGrid(GraphicsLayerItem *);
	
    GraphicsLayerItem *drawScenery(GraphicsLayerItem *);
    GraphicsLayerItem *drawTerrain(GraphicsLayerItem *);
    GraphicsLayerItem *drawTracknet(GraphicsLayerItem *);
    GraphicsLayerItem *drawRoadnet(GraphicsLayerItem *);
	GraphicsLayerItem *drawRouteOrigin(GraphicsLayerItem *);

	void drawMapScale();
	void updateMapScale();
	void updateLoftScale(bool redraw = false);
	double calcTicks(double minScale, double maxScale, int maxMarks);

protected:
	void remapTerrain();
	void wheelEvent(QWheelEvent * ev);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);

};


Q_DECLARE_METATYPE(MapWidget::GridType);
Q_DECLARE_METATYPE(MapWidget::CursorMode);
Q_DECLARE_METATYPE(MapWidget::WheelZoomDir);

#endif
