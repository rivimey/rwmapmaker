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
#include "GraphicsItems.h"
#include "MapWidget.h"


MapWidget::MapWidget(QWidget *parent) : QGraphicsView(parent)
{
    scene = new QGraphicsScene();
	setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setContentsMargins(4, 4, 4, 4);
    setViewportMargins(4, 4, 4, 4);
    setDragMode(QGraphicsView::ScrollHandDrag);
    initPens();
    resetScene();
}


MapWidget::~MapWidget()
{
    delete gridPen; 		gridPen = NULL;
    delete originPen; 		originPen = NULL;
    delete trackPen; 		trackPen = NULL;
    delete roadPen; 		roadPen = NULL;
    delete constructPen; 	constructPen = NULL;
    delete originBrush; 	originBrush = NULL;
    delete dotBrush; 		dotBrush = NULL;
    delete textPen; 		textPen = NULL;
    delete textFont; 		textFont = NULL;
    delete tileAddrBrush; 	tileAddrBrush = NULL;
    delete tileAddrFont; 	tileAddrFont = NULL;

	// DONT delete these; they are shared

	// terrainExtentBitmap
	// trackExtentBitmap
	// roadExtentBitmap
	// tracknet
    // roadnet
    // terrain

	delete scene;			scene = NULL;
}


/*!
    Return a pointer to the map's Graphics Scene.
    'new' because QGraphicsView also implements Scene.
 */
QGraphicsScene *MapWidget::getScene()
{
    return scene;
}

void MapWidget::clearScene()
{
    scene->clear();

	// groups are always added to the scene, and scene->clear() will delete
	// them, so no need to do it again!
	gridGroup = NULL;
	trackGroup = NULL;
	roadGroup = NULL;
	terrainGroup = NULL;
	originGroup = NULL;

    resetTransform();
    scale(INITIAL_SCALE, -INITIAL_SCALE);
}


/*!
   Reset the scene to default settings, but leave the scene data
   (route, track, terrain) etc as it was.
 */
void MapWidget::clear()
{
	clearScene();

    inUpdate = 0;  // positive if in update, 0 for not.
	showorigin = true;
    showgrid = true;
    showterrain = true;
    showtrack = true;
    showroads = true;
    showaddrs = true;
    gridType = GridType_Terrain;
}

/*!
    Create the initial pen, brush and font structures for the scene.
*/
void MapWidget::initPens()
{
    gridPen = new QPen(Qt::magenta, 0, Qt::SolidLine, Qt::SquareCap);
    originPen = new QPen(Qt::darkMagenta, 4, Qt::SolidLine, Qt::RoundCap);
    trackPen = new QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap);
    roadPen = new QPen(Qt::darkYellow, 2, Qt::SolidLine, Qt::SquareCap);
    constructPen = new QPen(QColor(0, 0, 0, 33), 1, Qt::SolidLine, Qt::SquareCap);  // transparent black

    textPen = new QPen(Qt::black);
    textFont = new QFont("Times", 9, 300);

    originBrush = new QBrush(Qt::red);
    dotBrush = new QBrush(Qt::black);
    tileAddrBrush = new QBrush(Qt::lightGray);
    tileAddrFont = new QFont("Times", 20, 300);
    tileAddrScale = 10;

    setBackgroundBrush(QBrush(QColor(255, 240, 230)));
}

/*!
    Reset the widget to default settings and clear the referenced
	data for the scene (route, track, terrain).
*/
void MapWidget::resetScene()
{
	clear();

	// The data to display
    terrain = NULL;
    tracknet = NULL;
    roadnet = NULL;

    // The extent indicates the overall size of the route, in tiles.
    terrainExtentBitmap = NULL;
    trackExtentBitmap = NULL;
    roadExtentBitmap = NULL;
}

/*!
    Scale the w, h by the factor needed to display a terrain tile 
 */
double MapWidget::scaleTerrain()
{
    double f = (TILE_SIZE_METRES / TILE_SIZE_TERRAIN);
    return f;
}

/*!
    Scale the coordinates by the size of a tile. DOES NOT necessarily return a value on a tile boundary.
 */
QPointF MapWidget::scaleTilesToMetres(QPointF xy)
{
    QPointF sxy(TILE_SIZE_METRES * xy.x(), TILE_SIZE_METRES * xy.y());
    return sxy;
}

/*!
    Return the coordinates mapped such that 0,0 is the centre of the (QImage) bitmap. 
 */
QPointF MapWidget::adjustBitmapCentre(QImage *bitmap, QPointF xy)
{
    QPointF sxy(xy.x() + bitmap->width() / 2, xy.y() + bitmap->height() / 2);
    return sxy;
}

// Required or Optional Data

/*!
    Define the bitmap (as a QImage) used to indicate which grid tiles are in use and
    the enclosing rectangle for all tiles that are in use. Values are in tiles, not metres.
    Setting this will cause a repaint of the scene.
 */
void MapWidget::setTerrainTileBitmap(QImage *bitmap, QRectF rect)
{
    terrainExtentBitmap = bitmap;
    terrainRect = rect;
    createScene();
}

/*!
    Define the bitmap (as a QImage) used to indicate which grid tiles are in use and
    the enclosing rectangle for all tiles that are in use. Values are in tiles, not metres.
    Setting this will cause a repaint of the scene. 
 */
void MapWidget::setTrackTileBitmap(QImage *bitmap, QRectF rect)
{
    trackExtentBitmap = bitmap;
    trackRect = rect;
    createScene();
}

/*!
    Define the bitmap (as a QImage) used to indicate which grid tiles are in use and
    the enclosing rectangle for all tiles that are in use. Values are in tiles, not metres.
    Setting this will cause a repaint of the scene. 
 */
void MapWidget::setRoadTileBitmap(QImage *bitmap, QRectF rect)
{
    roadExtentBitmap = bitmap;
    roadRect = rect;
    createScene();
}

/*!
    Define the tracknet of track that is to be plotted.
    Setting this will cause a repaint of the scene. 
 */
void MapWidget::setTrackNetwork(RibbonList *net)
{
    if (tracknet != net)
    {
        tracknet = net;
        createScene();
    }
}

/*!
    Define the tracknet of track that is to be plotted.
    Setting this will cause a repaint of the scene. 
 */
void MapWidget::setRoadNetwork(RibbonList *net)
{
    if (roadnet != net)
    {
        roadnet = net;
        createScene();
    }
}

/*!
    Define the terrain tiles that are to be plotted. Updates the scene
    unless this terrain has already been set. 
 */
void MapWidget::setTerrain(TerrainTileList *tileList)
{
    if (terrain != tileList)
    {
        terrain = tileList;
        createScene();
    }
}

// Configuring the map style

/*!
    Return the pen used to render grid lines. 
 */
const QBrush &MapWidget::getGridTextBrush()
{
	return *tileAddrBrush;
}

/*!
    Set the pen used to render grid lines. The pen is copied; the caller 
	may reuse or destroy the pen supplied.
 */
void MapWidget::setGridTextBrush(const QBrush &b)
{
	delete tileAddrBrush;
	tileAddrBrush = new QBrush(b);
}

const QFont &MapWidget::getGridTextFont()
{
	return *tileAddrFont;
}

void MapWidget::setGridTextFont(const QFont &f)
{
	delete tileAddrFont;
	tileAddrFont = new QFont(f);
}

const QFont &MapWidget::getTextFont()
{
	return *textFont;
}

void MapWidget::setTextFont(const QFont &f)
{
	delete textFont;
	textFont = new QFont(f);
}

/*!
    Return the pen used to render grid lines. 
 */
const QPen &MapWidget::getGridPen()
{
	return *gridPen;
}

/*!
    Set the pen used to render grid lines. The pen is copied; the caller 
	may reuse or destroy the pen supplied.
 */
void MapWidget::setGridPen(const QPen &p)
{
	delete gridPen;
	gridPen = new QPen(p);
}
	
/*!
    Return the pen used to render track.
 */
const QPen &MapWidget::getTrackPen()
{
	return *trackPen;
}

/*!
    Set the pen used to render track. The pen is copied; the caller 
	may reuse or destroy the pen supplied.
 */
void MapWidget::setTrackPen(const QPen &p)
{
	delete trackPen;
	trackPen = new QPen(p);
}
	
/*!
    Return the pen used to render roads.
 */
const QPen &MapWidget::getRoadPen()
{
	return *roadPen;
}

/*!
    Set the pen used to render roads. The pen is copied; the caller 
	may reuse or destroy the pen supplied.
 */
void MapWidget::setRoadPen(const QPen &p)
{
	delete roadPen;
	roadPen = new QPen(p);
}

/*!
    Return the pen used to render the route Origin.
 */
const QPen &MapWidget::getOriginPen()
{
	return *originPen;
}

/*!
    Set the pen used to render the route Origin. The pen is copied; the caller 
	may reuse or destroy the pen supplied.
 */
void MapWidget::setOriginPen(const QPen &p)
{
	delete originPen;
	originPen = new QPen(p);
}



// Showing elements on the map


/*!
    Enable or Disable the inclusion of the tile grid on the map
 */
void MapWidget::showGridlines(bool visible)
{
    if (visible != showgrid)
    {
        showgrid = visible;
        if (gridGroup != NULL && !isInUpdate())
            gridGroup->setVisible(visible);
        else
            createScene();
    }
}

/*!
    Enable or Disable the inclusion of the tile grid on the map
 */
void MapWidget::showOrigin(bool visible)
{
    if (visible != showorigin)
    {
        showorigin = visible;
        if (originGroup != NULL && !isInUpdate())
            originGroup->setVisible(visible);
        else
			createScene();
    }
}

/*!
    Enable or Disable the inclusion of terrain pictures on the map
 */
void MapWidget::showTerrain(bool visible)
{
    if (visible != showterrain)
    {
        showterrain = visible;
        if (terrainGroup != NULL && !isInUpdate())
            terrainGroup->setVisible(visible);
        else
            createScene();
    }
}

/*!
    Enable or Disable the inclusion of rail track on the map 
 */
void MapWidget::showTrack(bool visible)
{
    if (visible != showtrack)
    {
        showtrack = visible;
        if (trackGroup != NULL && !isInUpdate())
            trackGroup->setVisible(visible);
        else
            createScene();
    }
}

/*!
    Enable or Disable the inclusion of rail track on the map 
 */
void MapWidget::showRoads(bool visible)
{
    if (visible != showroads)
    {
        showroads = visible;
        if (roadGroup != NULL && !isInUpdate())
            roadGroup->setVisible(visible);
        else
            createScene();
    }
}

/*!
    Select whether the tile grid displayed is of terrain (GridType_Terrain) or
    track (GridType_Track) tiles. showGridlines(true) is required to display a grid.
 */
void MapWidget::showGridType(GridType gty)
{
    if (gty != gridType && (gty == GridType_Terrain || gty == GridType_Track))
    {
        gridType = gty;
        createScene();
    }
}

/*!
    Enable or Disable the display of the grid tile address as a backdrop to the map
 */
void MapWidget::showTileAddrs(bool visible)
{
    if (visible != showaddrs)
        showaddrs = visible;
    createScene();
}


// Zoom related

// very simple override of wheel event. Doesn't attempt to count delta() over multiple
// events, just 
void MapWidget::wheelEvent(QWheelEvent * ev)
{
     int numDegrees = ev->delta() / 8;
     int numSteps = numDegrees / 15;

	 if (numDegrees > 0)
		scale(WHEEL_SCALE_FACTOR, WHEEL_SCALE_FACTOR);

	 else if (numDegrees < 0)
		scale(1 / WHEEL_SCALE_FACTOR, 1 / WHEEL_SCALE_FACTOR);

     ev->accept();
}

/*!
    Zoom the map to show approximately the given number of metres width
 */
void MapWidget::zoomTo(double amt)
{
    scale(amt, amt);
}
void MapWidget::zoomTo(QScale amt)
{
	scale(amt.SX(), amt.SY());
}

/*!
    Return the current zoom amount. Don't rely on absolute values!
 */
QScale MapWidget::currentZoom()
{
    return QScale(transform().m11(), transform().m12());
}

/*!
    Zoom the map in to show more detail, by a factor of SCALE_FACTOR from the current amount
 */
void MapWidget::zoomIn()
{
    scale(SCALE_FACTOR, SCALE_FACTOR);
}

/*!
    Zoom the map in to show less detail, by a factor of SCALE_FACTOR from the current amount
 */
void MapWidget::zoomOut()
{
    scale(1 / SCALE_FACTOR, 1 / SCALE_FACTOR);
}

/*!
    Zoom (and reposition if needed) to show the whole of the map in the current viewport 
 */
void MapWidget::zoomAll()
{
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

/*!
    Zoom (and reposition if needed) to show the whole width of the map in the current viewport 
 */
void MapWidget::zoomWidth()
{
    QRectF rect = scene->sceneRect();
    rect.setHeight(1);
    fitInView(rect, Qt::KeepAspectRatio);
}

/*!
    Zoom (and reposition if needed) to show the whole height of the map in the current viewport 
 */
void MapWidget::zoomHeight()
{
    QRectF rect = scene->sceneRect();
    rect.setWidth(1);
    fitInView(rect, Qt::KeepAspectRatio);
}



// Painting   
/*!
    Signal that recreation of the scene should be deferred until a matching endUpdate().
    begin/end calls can be nested.
 */
void MapWidget::beginUpdate()
{
    inUpdate += 1;
}

/*!
    Signal that the current group of updates to the map data, signalled by a beginUpdate, are complete
    and recreation of the scene can potentially be done. The scene will be recreated if this is the
    outermost nesting level.
 */
void MapWidget::endUpdate()
{
    inUpdate -= 1;
    if (inUpdate <= 0)
    {
        inUpdate = 0;
        createScene();
    }
}

/*!
    Return true if map scene recreation is deferred because of an update.
 */
bool MapWidget::isInUpdate()
{
    return (inUpdate > 0);
}

/*!
    If not in an update, recreate the map scene.
 */
void MapWidget::createScene()
{
    if (!isInUpdate())
    {
        scene->clear();

        if (terrain != NULL && showterrain)
        {
            terrainGroup = drawTerrain(new QGraphicsItemGroup());
            scene->addItem(terrainGroup);
        }
        if (showgrid)
        {
            gridGroup = tileGrid(new QGraphicsItemGroup());
            scene->addItem(gridGroup);
        }

        if (showorigin)
        {
			originGroup = routeOrigin(new QGraphicsItemGroup());
            scene->addItem(originGroup);
        }

        if (tracknet != NULL && showtrack)
        {
            trackGroup = drawTracknet(new QGraphicsItemGroup());
            scene->addItem(trackGroup);
        }

        if (roadnet != NULL && showroads)
        {
            roadGroup = drawRoadnet(new QGraphicsItemGroup());
            scene->addItem(roadGroup);
        }
    }
}

/*!
    Add the drawing items for the origin marker, a cross placed on the 0,0 point of the 
	route.
 */
QGraphicsItemGroup *MapWidget::routeOrigin(QGraphicsItemGroup *originGroup)
{
	QGraphicsLineItem *line;
	QGraphicsEllipseItem *elli;

    // Mark the route origin with a big cross        
    int crossSz = TILE_SIZE_METRES / 3;
    int crossSzBy2 = crossSz / 3;
    line = new QGraphicsLineItem(          0,   -crossSz,          0,    crossSz);  line->setPen(*originPen);  originGroup->addToGroup(line);
    line = new QGraphicsLineItem(   -crossSz,          0,    crossSz,          0);  line->setPen(*originPen);  originGroup->addToGroup(line);
    line = new QGraphicsLineItem( crossSzBy2,-crossSzBy2,-crossSzBy2, crossSzBy2);  line->setPen(*originPen);  originGroup->addToGroup(line);
    line = new QGraphicsLineItem(-crossSzBy2,-crossSzBy2, crossSzBy2, crossSzBy2);  line->setPen(*originPen);  originGroup->addToGroup(line);

    elli = new QGraphicsEllipseItem(-3, -3, 6, 6);  elli->setBrush(*originBrush);  originGroup->addToGroup(elli);

	return originGroup;
}

/*!
    Add the drawing items for the tile grid to the scene group, returning
    the updated group. Only one type of tile grid is displayed at a time, the
    selection being made by the current gridType.
    
    If a grid is selected and if showaddrs is also true, the tile address, 
    relative to the origin, is added to the scene using the font in tileAddrFont.
 */
QGraphicsItemGroup *MapWidget::tileGrid(QGraphicsItemGroup *gridGroup)
{
    QRectF rect;
    QImage *bitmap = NULL;

    switch (gridType)
    {
        default:
        case GridType_Terrain:
            rect = terrainRect;
            bitmap = terrainExtentBitmap;
            break;

        case GridType_Track:
            rect = trackRect;
            bitmap = trackExtentBitmap;
            break;
    }

    if (bitmap != NULL)
    {
        QRectF tileRect(0, 0, TILE_SIZE_METRES, TILE_SIZE_METRES);

        for (double x = rect.left(); x <= rect.right(); x += 1)
        {
            for (double y = rect.top(); y <= rect.bottom(); y += 1)
            {
                QPointF loc(x, y);

                // extent can be pos or neg, but bitmap isn't: adjust to the centre.
                QPointF bm_loc = adjustBitmapCentre(bitmap, loc);

                int v = bitmap->pixelIndex((int)bm_loc.x(), (int)bm_loc.y());
                if (v == INDEX_TILE)
                {
                    QPointF sxy = scaleTilesToMetres(loc);
                    tileRect.moveTo(sxy);
                    QGraphicsRectItem *r_item = new QGraphicsRectItem(tileRect);
                    r_item->setPen(*gridPen);
                    gridGroup->addToGroup(r_item);

                    if (showaddrs)
                    {
                        QString text = QString("%1,%2").arg((int)loc.x()).arg((int)loc.y());
                        GraphicsCentredText *t_item = new GraphicsCentredText(text);
                        t_item->setFont(*tileAddrFont);
                        t_item->setBrush(*tileAddrBrush);
                        t_item->scale(tileAddrScale, -tileAddrScale);   // undo global -Y scale for this item and make large enough to see!
                        t_item->setPos(tileRect.center());
                        gridGroup->addToGroup(t_item);
                    }
                }
            }
        }
    }
    return gridGroup;
}

/*!
    Add the drawing items for the terrain. The terrain is passed in as QPixmap images in
    the terrain list, and these images are displayed as-is in the scene at the appropriate
    location.
 */
QGraphicsItemGroup *MapWidget::drawTerrain(QGraphicsItemGroup *terrainGroup)
{
    double sc = scaleTerrain();
	for (TerrainTileList::const_iterator it = terrain->begin(); it != terrain->end(); ++it)
    {
		TerrainTile *tile = *it;
        QPointF sxy = scaleTilesToMetres(tile->getPosition());
        QGraphicsPixmapItem *terr = new QGraphicsPixmapItem(*tile->getImage());
        terr->setPos(sxy);
        terr->scale(sc, sc);
        terrainGroup->addToGroup(terr);
    }
            
    return terrainGroup;
}

/*!
    Add the drawing items for the track network. The network is a list of IRibbon-derived
	objects defining the track.
 */
QGraphicsItemGroup *MapWidget::drawTracknet(QGraphicsItemGroup *trackGroup)
{
    // for reasons unknown, the angle specified seems to be normal to the direction of the track. pi/2 is 90 degrees.
    for(int u = 0; u < tracknet->size(); u++)
	{
		IRibbon *ribbon = tracknet->at(u);
		NetworkList *segments = ribbon->segments();

		for(int v = 0; v < segments->size(); v++)
        {
			INetwork *segment = segments->at(v);

            double x = segment->getX();
			double y = segment->getZ();   // UNITS => METRES
            double slen, ang, ax, ay;
			QString tip;

            // mark the "initial point" of the line segment->
            QGraphicsEllipseItem *initial = new QGraphicsEllipseItem(QRectF(x-0.5, y-0.5, 1, 1));
            initial->setPen(QPen());
            initial->setBrush(*dotBrush);
            trackGroup->addToGroup(initial);
                
            switch(segment->curveType())
            {
                case CurveTypeStraight:
					{
                    NetworkCurveStraight *seg_st = (NetworkCurveStraight*)segment;
                    slen = seg_st->getLen();
                    ang = seg_st->getATan2();					// angle in radians.. 
                    ax = floor(x + cos(ang) * slen);
                    ay = floor(y + sin(ang) * slen);
                    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(x, y, ax, ay));
                    line->setPen(*trackPen);
					tip = QString("Track Ribbon: %1\nID: %2\nLength: %3\nAngle: %4")
											.arg(ribbon->ribbonid())
											.arg(seg_st->getId())
											.arg(slen)
											.arg(ang);
                    line->setToolTip(tip);
                    trackGroup->addToGroup(line);
					}
					break;

                case CurveTypeArc:
					{
                    // inputs
                    NetworkCurveArc *seg_cv = (NetworkCurveArc*)segment;
                    slen = seg_cv->getLen();
                    ang = seg_cv->getATan2();					// angle in radians.. 
                    double arc = seg_cv->getArc();              // curvature
                    int sign = seg_cv->getSign();               // unknown purpose

                    GraphicsCurveItem *curve = new GraphicsCurveItem(NULL);
                    curve->setPos(QPointF(x,y));
                    curve->setPen(*trackPen);
                    curve->setSign(sign);
                    curve->setCurvature(arc);
                    curve->setArcLength(slen);
                    curve->setTangent(ang);
					tip = QString("Track Ribbon: %1\nID: %2\nLength: %3\nAngle: %4\nArc: %5")
											.arg(ribbon->ribbonid())
											.arg(seg_cv->getId())
											.arg(slen)
											.arg(ang)
											.arg(arc);
                    curve->setToolTip(tip);
                    trackGroup->addToGroup(curve);
					}
                    break;
            }
        }
    }
    return trackGroup;
}


/*!
    Add the drawing items for the road network. The network is a list of IRibbon-derived
	objects defining the road mesh.
 */
QGraphicsItemGroup *MapWidget::drawRoadnet(QGraphicsItemGroup *roadGroup)
{
    // for reasons unknown, the angle specified seems to be normal to the direction of the road. pi/2 is 90 degrees.
	for (RibbonList::const_iterator it = roadnet->begin(); it != roadnet->end(); ++it)
    {
		IRibbon *ribbon = *it;
		NetworkList *segments = ribbon->segments();
		for (NetworkList::const_iterator it2 = segments->begin(); it2 != segments->end(); ++it2)
        {
			INetwork *segment = *it2;

            double x = segment->getX();
			double y = segment->getZ();   // UNITS => METRES
            double slen, ang, ax, ay;
			QString tip;

            // mark the "initial point" of the line segment->
            QGraphicsEllipseItem *initial = new QGraphicsEllipseItem(QRectF(x-0.5, y-0.5, 1, 1));
            initial->setPen(QPen());
            initial->setBrush(*dotBrush);
            roadGroup->addToGroup(initial);

            switch (segment->curveType())
            {
                case CurveTypeStraight:
                    {
					NetworkCurveStraight *seg_st = (NetworkCurveStraight*)segment;
                    slen = seg_st->getLen();
                    ang = seg_st->getATan2();                // angle in radians.. unknown reference
                    ax = floor(x + cos(ang) * slen);
                    ay = floor(y + sin(ang) * slen);
                    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(x, y, ax, ay));
                    line->setPen(*roadPen);
					tip = QString("Road Ribbon: %1\nID: %2\nLength: %3\nAngle: %4")
											.arg(ribbon->ribbonid())
											.arg(seg_st->getId())
											.arg(slen)
											.arg(ang);
                    line->setToolTip(tip);
                    roadGroup->addToGroup(line);
					}
                    break;

                case CurveTypeArc:
                    {
					// inputs
                    NetworkCurveArc *seg_cv = (NetworkCurveArc*)segment;
                    slen = seg_cv->getLen();
                    ang = seg_cv->getATan2();					// angle in radians.. 
                    double arc = seg_cv->getArc();              // curvature
                    int sign = seg_cv->getSign();               // unknown purpose

                    GraphicsCurveItem *curve = new GraphicsCurveItem(NULL);
                    curve->setPos(QPointF(x,y));
                    curve->setPen(*roadPen);
                    curve->setSign(sign);
                    curve->setCurvature(arc);
                    curve->setArcLength(slen);
                    curve->setTangent(ang);
					tip = QString("Road Ribbon: %1\nID: %2\nLength: %3\nAngle: %4\nArc: %5")
											.arg(ribbon->ribbonid())
											.arg(seg_cv->getId())
											.arg(slen)
											.arg(ang)
											.arg(arc);
                    curve->setToolTip(tip);
                    roadGroup->addToGroup(curve);
					}
                    break;
            }
        }
    }
    return roadGroup;
}

