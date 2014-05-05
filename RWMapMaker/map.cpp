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

#include <QtConcurrent/QtConcurrent>
#include "RWMapMaker.h"
#include "GraphicsItems.h"
#include "map.h"

/*!
    
 */
inline double copysign(double tickInterval, double distance)
{
	double x = fabs(tickInterval);
	if (distance >= 0)
		return x;
	else
		return -x;
}


MapWidget::MapWidget(QWidget *parent) : GraphicsOverlayView(parent)
{
    scene = new QGraphicsScene();
	setScene(scene);
    setContentsMargins(4, 4, 4, 4);
    setViewportMargins(4, 4, 4, 4);
	
	rubberBand = NULL;
	cmode = Cursor_Hand;
	zoomdir = Zoom_InOut;
	setCursorMode(Cursor_Hand);

    setRenderHint(QPainter::Antialiasing);
	setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

	//setOverlayOpacity(1);
	setOverlayAlignment(Qt::AlignRight | Qt::AlignBottom);
	scaleOverlay = new QPixmap(QSize(SCALE_WIDTH,SCALE_HEIGHT));
	scaleOverlay->fill(QColor(0,0,0,SCALE_OPACITY));  // transparent black
	setOverlayPixmap(scaleOverlay);

	prevMouseTime = QDateTime::currentMSecsSinceEpoch();

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
    // terrainArray
	// scenery

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

	// reset the scene
	setScene(NULL);
	setScene(scene);
	
	// groups are always added to the scene, and scene->clear() will delete
	// them, so no need to do it again!
	gridLayer = NULL;
	trackLayer = NULL;
	roadLayer = NULL;
	terrainLayer = NULL;
	sceneryLayer = NULL;
	originLayer = NULL;

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
	showmapscale = true;
	showscenery = true;
	showloftends = true;
    gridType = GridType_Terrain;

	trackScaleMode = Scale_WithView;
	trackScale = 1.0;
	roadScaleMode = Scale_WithView;
	roadScale = 1.0;
}

/*!
    Create the initial pen, brush and font structures for the scene.
*/
void MapWidget::initPens()
{
    gridPen = new QPen(Qt::magenta, 0, Qt::SolidLine, Qt::FlatCap);
    originPen = new QPen(Qt::darkMagenta, 4, Qt::SolidLine, Qt::RoundCap);
    trackPen = new QPen(Qt::black, 0, Qt::SolidLine, Qt::FlatCap);
    roadPen = new QPen(Qt::darkYellow, 2, Qt::SolidLine, Qt::FlatCap);
    constructPen = new QPen(QColor(0, 0, 0, 33), 1, Qt::SolidLine, Qt::FlatCap);  // transparent black

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
    terrainArray = NULL;  // loaned to us
    deleteTerrainTileList(&terrainImages);  // derived, but leave list empty not null
    tracknet = NULL;
    roadnet = NULL;
	scenery = NULL;

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
	Provide the configured gradient-mapper object which will be used
	to map terrain height to a colour on the tile. This object is only
	used by RWImporter and will not be deleted by it.
*/
void MapWidget::setMapGradient(ColourMapper *g)
{
	mapper = g;
	remapTerrain();
    if (showterrain && terrainLayer != NULL && !isInUpdate())
	{
		terrainLayer->childItems().clear();
		drawTerrain(terrainLayer);
	}
}

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
void MapWidget::setTerrain(TerrainArrayList *tiles)
{
    if (terrainArray != tiles)
    {
        terrainArray = tiles;
		remapTerrain();
		createScene();
    }
}

/*!
    Define the terrain tiles that are to be plotted. Updates the scene
    unless this terrain has already been set. 
 */
void MapWidget::setScenery(SceneryItemList *sceneryList)
{
    if (scenery != sceneryList)
    {
        scenery = sceneryList;
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

void MapWidget::setTrackScaling(ViewScaling s)
{
	if (trackScaleMode != s)
	{
		trackScaleMode = s;
		trackPen->setCosmetic(trackScaleMode == Scale_None);
	}
}

void MapWidget::setRoadScaling(ViewScaling s)
{
	if (roadScaleMode != s)
	{
		roadScaleMode = s;
		roadPen->setCosmetic(roadScaleMode == Scale_None);
	}
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
        if (gridLayer != NULL && !isInUpdate())
            gridLayer->setVisible(visible);
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
        if (originLayer != NULL && !isInUpdate())
            originLayer->setVisible(visible);
        else
			createScene();
    }
}

void MapWidget::showMapScale(bool visible)
{
    if (visible != showmapscale)
    {
        showmapscale = visible;

		if (showmapscale)
			setOverlayPixmap(scaleOverlay);
		else
			setOverlayPixmap(NULL);
    }
}

void MapWidget::showLoftEnds(bool visible)
{
    if (visible != showloftends)
    {
        showloftends = visible;
		createScene();
    }
}

/*!
    Enable or Disable the inclusion of scenery on the map
 */
void MapWidget::showScenery(bool visible)
{
    if (visible != showscenery)
    {
        showscenery = visible;
        if (sceneryLayer != NULL && !isInUpdate())
            sceneryLayer->setVisible(visible);
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
        if (terrainLayer != NULL && !isInUpdate())
            terrainLayer->setVisible(visible);
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
        if (trackLayer != NULL && !isInUpdate())
            trackLayer->setVisible(visible);
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
        if (roadLayer != NULL && !isInUpdate())
            roadLayer->setVisible(visible);
        else
            createScene();
    }
}

/*!
    Select whether the tile grid displayed is of terrain (GridType_Terrain) or
    track (GridType_Track) tiles. showGridlines(true) is required to display a grid.
 */
void MapWidget::setGridType(GridType gty)
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
	{
        showaddrs = visible;
		createScene();
	}
}

/*!
    Enable or Disable the display of the grid tile address as a backdrop to the map
 */
void MapWidget::setFastRender(bool fast)
{
    if (fast != fastrender)
    {
		fastrender = fast;
	}
}

// Zoom related

// very simple override of wheel event. Doesn't attempt to count delta() over multiple
// events, just 
void MapWidget::wheelEvent(QWheelEvent * ev)
{
	int numDegrees = ev->delta() / 8;
	int numSteps = numDegrees / 15;
	if (numDegrees == 0)
	{
		ev->accept();
		return;
	}

	switch(zoomdir)
	{
	case Zoom_InOut:
		if (numDegrees < 0)
			scale(WHEEL_SCALE_FACTOR, WHEEL_SCALE_FACTOR);
		else
			scale(1 / WHEEL_SCALE_FACTOR, 1 / WHEEL_SCALE_FACTOR);
		break;

	case Zoom_OutIn:
		if (numDegrees > 0)
			scale(WHEEL_SCALE_FACTOR, WHEEL_SCALE_FACTOR);
		else
			scale(1 / WHEEL_SCALE_FACTOR, 1 / WHEEL_SCALE_FACTOR);
		break;
	}

	updateMapScale();
	updateLoftScale(true);

	ev->accept();
}


void MapWidget::setCursorMode(CursorMode cm)
{
	cmode = cm;

	if (rubberBand != NULL)
		rubberBand->hide();

	// hand mode is done by view...
	if (cmode == Cursor_Hand)
		setDragMode(QGraphicsView::ScrollHandDrag);
	else
		setDragMode(QGraphicsView::NoDrag);

	// otherwise done by us.
	if (cmode == Cursor_Select)
		setCursor(Qt::ArrowCursor);
}

void MapWidget::setMouseUpdates(bool enable)
{
	setMouseTracking(enable);
	mouseUpdates = enable;
}

void MapWidget::mousePressEvent(QMouseEvent *e)
{
	if (cmode == Cursor_Select)
	{
		selectOrigin = e->pos();
		selectedRegion = QRect(e->pos(), QSize());
		if (rubberBand == NULL)
			rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
		else
			rubberBand->hide();
		rubberBand->setGeometry(selectedRegion);
		rubberBand->show();
		rubberBanding = true;	// true while selection area changing, false otherwise
	}
	QGraphicsView::mousePressEvent(e);
}

void MapWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (cmode == Cursor_Select && rubberBand != NULL)
	{
		rubberBanding = false;
		// update for the last point...
		selectedRegion = QRect(selectOrigin, e->pos()).normalized();
		// and if the resulting size is a point, disable the selection.
		if (selectedRegion.size().isEmpty())
			rubberBand->hide();
	}
	QGraphicsView::mouseReleaseEvent(e);
}

/*!
    Handle a mouse move event.
	There are two cases:
	- handling rubber-band sizing during area selection
	- and updating the location information.
 */
void MapWidget::mouseMoveEvent(QMouseEvent *e)
{
	if (cmode == Cursor_Select && rubberBand != NULL && rubberBanding)
	{
		selectedRegion = QRect(selectOrigin, e->pos()).normalized();
		rubberBand->setGeometry(selectedRegion);
	}
	if (mouseUpdates && (tracknet != NULL || roadnet != NULL))
	{
		qint64 now = QDateTime::currentMSecsSinceEpoch();
		if ((now - prevMouseTime) > 100)
		{
			QPointF pos;
			pos = mapToScene(e->pos());
			emit mousePos(pos);
			prevMouseTime = now;
		}
	}
	QGraphicsView::mouseMoveEvent(e);
}

/*!
    Return true if the selection region is valid (i.e. there is
	a region defined)
 */
bool MapWidget::selectionValid()
{
	return rubberBand != NULL && rubberBand->isVisible();
}


/*!
    Return the selection area in scene coordinates. Because it is
	in theory possible for the scene to be rotated, a rectanglular
	selection does not necessarily map to a rectangle in the scene.
 */
QPolygonF MapWidget::selectionAsScene()
{
	QPolygonF rect;
	if (selectionValid())
	{
		rect = mapToScene(selectedRegion);
	}
	return rect;
}

/*!
    Return the selection in window coordinates. If there is no selection
	defined, return a null rectangle.
 */
QRect MapWidget::selection()
{
	QRect rect;
	if (selectionValid())
	{
		rect = selectedRegion;
	}
	return rect;
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
    scale(1 / SCALE_FACTOR, 1 / SCALE_FACTOR);
	updateMapScale();
	updateLoftScale(true);
}

/*!
    Zoom the map in to show less detail, by a factor of SCALE_FACTOR from the current amount
 */
void MapWidget::zoomOut()
{
    scale(SCALE_FACTOR, SCALE_FACTOR);
	updateMapScale();
	updateLoftScale(true);
}

/*!
    Zoom (and reposition if needed) to show the whole of the map in the current viewport 
 */
void MapWidget::zoomAll()
{
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
	updateMapScale();
	updateLoftScale(true);
}

/*!
    Zoom (and reposition if needed) to show the whole width of the map in the current viewport 
 */
void MapWidget::zoomWidth()
{
    QRectF rect = scene->sceneRect();
    rect.setHeight(1);
    fitInView(rect, Qt::KeepAspectRatio);
	updateMapScale();
	updateLoftScale(true);
}

/*!
    Zoom (and reposition if needed) to show the whole height of the map in the current viewport 
 */
void MapWidget::zoomHeight()
{
    QRectF rect = scene->sceneRect();
    rect.setWidth(1);
    fitInView(rect, Qt::KeepAspectRatio);
	updateMapScale();
	updateLoftScale(true);
}


#ifdef RWMM_HOVER
void MapWidget::setDescriptionMessageFn(QObject *obj, const char *slotname)
{
	descMsgObj = obj;
	descMsgSlot = slotname;
}

void MapWidget::applyDescriptionMessageFn(GraphicsLayerItem *layer)
{
	if (layer != NULL && descMsgObj != NULL && descMsgSlot != NULL)
		connect(layer, SIGNAL(description(const MapItemGroupMessage&)), descMsgObj, descMsgSlot);
}
#endif

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
    Signal that the current Layer of updates to the map data, signalled by a beginUpdate, are complete
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
		if (fastrender)
		{
			setRenderHints(QPainter::TextAntialiasing);
			setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
		}
		else
		{
			setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
			setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
		}
		
		updateLoftScale();
        
		if (terrainArray != NULL && showterrain)
        {
            terrainLayer = drawTerrain(new GraphicsLayerItem());
            scene->addItem(terrainLayer);
        }

        if (showgrid)
        {
            gridLayer = tileGrid(new GraphicsLayerItem());
            scene->addItem(gridLayer);
        }

        if (showorigin)
        {
			originLayer = drawRouteOrigin(new GraphicsLayerItem());
            scene->addItem(originLayer);
        }

        if (scenery != NULL && showscenery)
        {
            sceneryLayer = drawScenery(new GraphicsLayerItem());
#ifdef RWMM_HOVER
			applyDescriptionMessageFn(sceneryLayer);
#endif
			sceneryLayer->setAcceptHoverEvents(true);
            scene->addItem(sceneryLayer);
        }
		// see also updateLoftScale
        if (tracknet != NULL && showtrack)
        {
            trackLayer = drawTracknet(new GraphicsLayerItem());
#ifdef RWMM_HOVER
			applyDescriptionMessageFn(trackLayer);
#endif
			trackLayer->setAcceptHoverEvents(true);
            scene->addItem(trackLayer);
        }
		// see also updateLoftScale
        if (roadnet != NULL && showroads)
        {
            roadLayer = drawRoadnet(new GraphicsLayerItem());
#ifdef RWMM_HOVER
			applyDescriptionMessageFn(roadLayer);
#endif
			roadLayer->setAcceptHoverEvents(true);
            scene->addItem(roadLayer);
        }

		updateMapScale();
    }
}


/*!
	If needed, update the map's scale widget
 */
void MapWidget::updateMapScale()
{
    if (!isInUpdate() && showmapscale)
    {
		drawMapScale();
		setOverlayPixmap(scaleOverlay);
    }
}


/*!
	If needed, recalculate and update the size of loft elements, depending
	on the "exaggerated" flags et al.

	Redrawing of road and track layers is done explicitly rather than calling
	createScene() for performance.
 */
void MapWidget::updateLoftScale(bool redraw)
{
	emit zoomChanged();

	if (!isInUpdate() && (trackScaleMode == Scale_Exaggerated || roadScaleMode == Scale_Exaggerated))
    {
		// update loft scale
		bool redrawn = false;
		inUpdate += 1;  // beginUpdate() - prevent recursive redraw or other changes

		QScale sc = currentZoom();
		float newScale;
		if (sc.SX() < 0.001)
			newScale = 50;
		else if (sc.SX() < 0.01)
			newScale = 10;
		else if (sc.SX() < 0.1)
			newScale = 5;
		else if (sc.SX() < 0.3)
			newScale = 2;
		else if (sc.SX() < 0.6)
			newScale = 1.2f;
		else
			newScale = 1.0f;

		if (trackScaleMode == Scale_Exaggerated && newScale != trackScale)
		{
			trackScale = newScale;
			if (redraw)
			{
				trackLayer->childItems().clear();
		        drawTracknet(trackLayer);
				redrawn = true;
			}
		}
		if (roadScaleMode == Scale_Exaggerated && newScale != roadScale)
		{
			roadScale = newScale;
			if (redraw)
			{
				roadLayer->childItems().clear();
		        drawRoadnet(roadLayer);
				redrawn = true;
			}
		}
		inUpdate -= 1;  // endUpdate(), except don't recreate scene.

		if (redrawn)
			emit zoomChangedRedraw();
	}
}

/*!
    Add the drawing items for the origin marker, a cross placed on the 0,0 point of the 
	route.
 */
void MapWidget::drawMapScale()
{
	scaleOverlay->fill(QColor(0,0,0,SCALE_OPACITY));  // transparent black
	QPainter painter(scaleOverlay);

	const int ofsX = 20;  // pixels offset from lh & rh edge
	const int ofsY = 10;  // pixels offset from t & b edge

	QRect scaleRect = scaleOverlay->rect();
	scaleRect.adjust(ofsX,ofsY,-ofsX,-ofsY);
	// work out a good tick interval in scene coordinates.
	const QSizeF sz = overlaySize(scaleRect);
	// SCALE_WIDTH wide
	const int txtOfsY = 4;  // pixels down from lineY
	const int lineY = 14;  // pixels from top, overall 60 high
	const int tickSz = 6; // pixels up from lineY

	const double tickIntv = calcTicks(0.0, sz.width(), 10);
	double x, xp;

	// scene origin..
	QPoint p_org = mapFromScene(0, 0);
	QFontMetrics fm(*textFont);
	// 
	QString unit("metres");
	if (tickIntv >= SCALE_KM_BREAK)
		unit = "kilometres";
	QPen linePen(Qt::black);
	linePen.setWidth(2);
	painter.setPen(linePen);
	painter.setFont(*textFont);
	// x is scene coords, not view...
	for(x = 0; x <= sz.width(); x += tickIntv)
	{
		QPoint p = mapFromScene((qreal)x, 0);
		xp = p.x() - p_org.x() + ofsX;

		painter.drawLine(xp, lineY, xp, lineY - tickSz);
		
		double lab_x = x;
		if (tickIntv >= SCALE_KM_BREAK)
			lab_x /= 1000;
		QString label = QString("%1").arg(lab_x);
		QRectF br = fm.boundingRect(label);
		xp -= br.width()/2;
		br.adjust(0,0,2,1);
		br.moveTopLeft(QPoint(xp, lineY + txtOfsY));
		painter.drawText(br, 0, label);
	}
	QPoint p = mapFromScene((qreal)x-tickIntv, 0);
	xp = p.x() - p_org.x() + ofsX;
	painter.drawLine(ofsX, lineY, xp, lineY);

	QRectF br = fm.boundingRect(unit);
	xp = (xp - br.width())/2;
	br.moveTopLeft(QPoint(xp, lineY + txtOfsY + 1 + br.height()));
	painter.drawText(br, 0, unit);

#if 1
	QGraphicsScene *s = this->getScene();
	QGraphicsView *v = this;
	int allobjs = s->children().count();
	int visible = v->children().count();
	QString vectorInfo = QString("Objects %1/%2").arg(visible).arg(allobjs);

	QFont vectorFont = *textFont;
	vectorFont.setPixelSize(vectorFont.pixelSize() * 2 / 3);
	QFontMetrics vfm(vectorFont);
	br = fm.boundingRect(vectorInfo);
	br.moveTopLeft(QPoint(10, 30));
	painter.setFont(vectorFont);
	painter.drawText(br, 0, vectorInfo);
#endif
}


/*!
	Calculate optimum tickmark parameters for the scale defined by minScale,
	maxScale and maxMarks. Return a tick interval in scale units.
*/
double MapWidget::calcTicks(double minScale, double maxScale, int maxMarks)
{
	int powerOfTen;
	double tickInterval;
	double distance;

	//
	// calculate the range of values which must be displayed.
	//
	distance = maxScale - minScale;

	//
	// first do a sanity check
	//
	if (distance == 0)
		distance = 1;
	
	tickInterval = fabs(distance) / maxMarks;
	powerOfTen = 0;
	while ((tickInterval < 1.0) || (tickInterval >= 10.0))
	{
		if (tickInterval < 1.0)
		{
			tickInterval *= 10.0;
			powerOfTen --;
		}
		if (tickInterval >= 10.0)
		{
			tickInterval /= 10.0;
			powerOfTen ++;
		}
	}

	if (tickInterval > 5.0)
		tickInterval = 10.0;
	else if (tickInterval > 2.0)
		tickInterval = 5.0;
	else
		tickInterval = 2.0;
	
	tickInterval *= pow(10.0, powerOfTen);
	tickInterval = copysign(tickInterval, distance);

	return tickInterval;
}


/*!
    Add the drawing items for the origin marker, a cross placed on the 0,0 point of the 
	route.
 */
GraphicsLayerItem *MapWidget::drawRouteOrigin(GraphicsLayerItem *originLayer)
{
	QGraphicsLineItem *line;
	QGraphicsEllipseItem *elli;

    // Mark the route origin with a big cross        
    int crossSz = TILE_SIZE_METRES / 3;
    int crossSzBy2 = crossSz / 3;
    line = new QGraphicsLineItem(          0,   -crossSz,          0,    crossSz);  line->setPen(*originPen);  originLayer->addToLayer(line);
    line = new QGraphicsLineItem(   -crossSz,          0,    crossSz,          0);  line->setPen(*originPen);  originLayer->addToLayer(line);
    line = new QGraphicsLineItem( crossSzBy2,-crossSzBy2,-crossSzBy2, crossSzBy2);  line->setPen(*originPen);  originLayer->addToLayer(line);
    line = new QGraphicsLineItem(-crossSzBy2,-crossSzBy2, crossSzBy2, crossSzBy2);  line->setPen(*originPen);  originLayer->addToLayer(line);

    elli = new QGraphicsEllipseItem(-20, -20, 40, 40);  elli->setBrush(*originBrush);  originLayer->addToLayer(elli);

	return originLayer;
}

/*!
    Add the drawing items for the tile grid to the scene group, returning
    the updated group. Only one type of tile grid is displayed at a time, the
    selection being made by the current gridType.
    
    If a grid is selected and if showaddrs is also true, the tile address, 
    relative to the origin, is added to the scene using the font in tileAddrFont.
 */
GraphicsLayerItem *MapWidget::tileGrid(GraphicsLayerItem *gridLayer)
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
                    gridLayer->addToLayer(r_item);

                    if (showaddrs)
                    {
                        QString text = QString("%1,%2").arg((int)loc.x()).arg((int)loc.y());
                        GraphicsCentredText *t_item = new GraphicsCentredText(text);
                        t_item->setFont(*tileAddrFont);
                        t_item->setBrush(*tileAddrBrush);
						// undo global -Y scale for this item and make large enough to see!
						t_item->setTransform(QTransform::fromScale(tileAddrScale, -tileAddrScale), true);
                        t_item->setPos(tileRect.center());
                        gridLayer->addToLayer(t_item);
                    }
                }
            }
        }
    }
    return gridLayer;
}


/*!
	Convert the TerrainArray in \tilearray - a 2D height field using fixed point maths - into
	a new TerrainTile - a 2D colour-mapped field, using the colour mapper object \a mapper .

	The function is static and reentrant so that it can be called as a thread task. As
	a result of this, the final conversion - of QImage to QPixmap - is deferred because QPixmap
	calls can only be done on the GUI thread.
*/
static TerrainTile *convertTerrainTile(ColourMapper *mapper, const TerrainArray *tilearray)
{
	QImage *img = new QImage(TILE_SIZE_TERRAIN, TILE_SIZE_TERRAIN, QImage::Format_ARGB32_Premultiplied);
	QPainter painter(img);
	img->fill(Qt::black);
	for(int y = 0; y < TILE_SIZE_TERRAIN; y++)
	{
		for(int x = 0; x < TILE_SIZE_TERRAIN; x++)
		{
			// now deal with the data
			float f1 = floatFromHval(tilearray->at(x,y));
			QColor &c1 = mapper->mapToColour(f1);
			painter.setPen(QPen(c1));
			painter.drawPoint(x,y);
		}
	}
	painter.end();

	TerrainTile *tile = new TerrainTile(tilearray->getPosition(), img);
	return tile;
}

// Function object (functor) to allow "mapper" pointer for convertTerrainTile
struct Converted
{
	Converted(ColourMapper *map)
		: mapper(map) { }

	typedef TerrainTile *result_type;

	TerrainTile* operator()(const TerrainArray *tilearray)
	{
		return (convertTerrainTile(mapper, tilearray));
	}

	ColourMapper *mapper;
};

/*!
	Recreate the set of terrain images used by the graphics view from the set of
	terrain height arrays provided to us.
 */
void MapWidget::remapTerrain()
{
	deleteTerrainTileList(&terrainImages);

	if (terrainArray == NULL)
		return;

	QFuture<TerrainTile*> images = QtConcurrent::mapped(terrainArray->begin(), terrainArray->end(), Converted(mapper));
	
	TerrainTileList l = images.results();
	for (TerrainTileList::const_iterator it = l.begin(); it != l.end(); ++it)
	{
		TerrainTile* tileI = *it;
		tileI->convertImage();		// perform deferred processing of QImage to QPixmap
		terrainImages.append(tileI);
	}
}

/*!
    Add the drawing items for the terrain. The terrain is passed in as QPixmap images in
    the terrain list, and these images are displayed as-is in the scene at the appropriate
    location.
 */
GraphicsLayerItem *MapWidget::drawTerrain(GraphicsLayerItem *terrainLayer)
{
    double sc = scaleTerrain();
	for (TerrainTileList::const_iterator it = terrainImages.begin(); it != terrainImages.end(); ++it)
    {
		TerrainTile *tile = *it;
        QPointF sxy = scaleTilesToMetres(tile->getPosition());
        QGraphicsPixmapItem *terr = new QGraphicsPixmapItem(*tile->getImage());
        terr->setPos(sxy);
		terr->setTransform(QTransform::fromScale(sc, sc), true);
        terrainLayer->addToLayer(terr);
    }
            
    return terrainLayer;
}

/*!
    Add the drawing items for the terrain. The terrain is passed in as QPixmap images in
    the terrain list, and these images are displayed as-is in the scene at the appropriate
    location.
 */
GraphicsLayerItem *MapWidget::drawScenery(GraphicsLayerItem *sceneryLayer)
{
	for (SceneryItemList::const_iterator it = scenery->begin(); it != scenery->end(); ++it)
    {
		SceneryItem *item = *it;
		if (item->getCategory() == SceneryItem::Water || item->getCategory() == SceneryItem::Other)
			continue;

		if (! item->getLocation().isValid())
			continue;

        double x = item->getX();
		double y = item->getZ();   // UNITS => METRES
		if ((x > 39000 || x < -39000) || (y > 39000 || y < -39000))
			qxtLog->debug() << "Scenery @" << x << ", " << y << ": " << item->getBlueprint();
		QSize s = SceneryItem::toSize(item->getCategory());
		QColor c = SceneryItem::toColor(item->getCategory());
		QPointF p(x, y);
		QPointF q(x+s.width()/2, y+s.height()/2);
        GraphicsSceneryItem *ei = new GraphicsSceneryItem(QRectF(p,s));
		QRadialGradient rg(q, s.width()/2);
		rg.setColorAt(0, c);
		rg.setColorAt(0.25, c);
		c.setAlpha(0);
		rg.setColorAt(1, c);
		ei->setBrush(rg);
		ei->setPen(QPen(Qt::NoPen));
		ei->setBlueprint(item->getBlueprint());
		ei->setToolTip(item->getBlueprint());
        sceneryLayer->addToLayer(ei);
    }
    
    return sceneryLayer;
}

/*!
    Add the drawing items for the track network. The network is a list of IRibbon-derived
	objects defining the track.
 */
GraphicsLayerItem *MapWidget::drawTracknet(GraphicsLayerItem *trackLayer)
{
	QPen realTrackPen(*trackPen);
	qreal nwidth = realTrackPen.widthF() * trackScale;
	realTrackPen.setWidthF(nwidth);
	realTrackPen.setCapStyle(Qt::FlatCap);

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

			if (showloftends)
			{
				// mark the "initial point" of the line segment->
				QGraphicsEllipseItem *initial = new QGraphicsEllipseItem(QRectF(x-0.75, y-0.75, 1.5, 1.5));
				initial->setPen(QPen(Qt::NoPen));
				initial->setBrush(*dotBrush);
				trackLayer->addToLayer(initial);
			}

            switch(segment->curveType())
            {
                case CurveTypeStraight:
					{
                    NetworkCurveStraight *seg_st = (NetworkCurveStraight*)segment;
                    slen = seg_st->getLen();
                    ang = seg_st->getATan2();					// angle in radians.. 
                    ax = x + cos(ang) * slen;
                    ay = y + sin(ang) * slen;
                    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(x, y, ax, ay));
                    line->setPen(realTrackPen);
					tip = QString("Track Ribbon: %1\nID: %2\nStart@: %3, %4\nEnd@: %5, %6\nLength: %7\nAngle: %8")
											.arg(ribbon->ribbonid())
											.arg(seg_st->getId())
											.arg(x).arg(y)
											.arg(ax).arg(ay)
											.arg(slen).arg(ang);
                    line->setToolTip(tip);
                    trackLayer->addToLayer(line);
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
                    curve->setPen(realTrackPen);
                    curve->setSign(sign);
                    curve->setCurvature(arc);
                    curve->setArcLength(slen);
                    curve->setTangent(ang);
					tip = QString("Track Ribbon: %1\nID: %2\nStart@: %3, %4\nLength: %5\nAngle: %6\nArc: %7")
											.arg(ribbon->ribbonid())
											.arg(seg_cv->getId())
											.arg(x).arg(y)
											.arg(slen)
											.arg(ang)
											.arg(arc);
                    curve->setToolTip(tip);
                    trackLayer->addToLayer(curve);
					}
                    break;
            }
        }
    }
    return trackLayer;
}


/*!
    Add the drawing items for the road network. The network is a list of IRibbon-derived
	objects defining the road mesh.
 */
GraphicsLayerItem *MapWidget::drawRoadnet(GraphicsLayerItem *roadLayer)
{
	QPen realRoadPen(*roadPen);
	realRoadPen.setWidthF(realRoadPen.widthF() * roadScale);
	realRoadPen.setCapStyle(Qt::FlatCap);

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

			if (showloftends)
			{
				// mark the "initial point" of the line segment->
				QGraphicsEllipseItem *initial = new QGraphicsEllipseItem(QRectF(x-0.75, y-0.75, 1.5, 1.5));
				initial->setPen(QPen());
				initial->setBrush(*dotBrush);
				roadLayer->addToLayer(initial);
			}

            switch (segment->curveType())
            {
                case CurveTypeStraight:
                    {
					NetworkCurveStraight *seg_st = (NetworkCurveStraight*)segment;
                    slen = seg_st->getLen();
                    ang = seg_st->getATan2();                // angle in radians.. unknown reference
                    ax = x + cos(ang) * slen;
                    ay = y + sin(ang) * slen;
                    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(x, y, ax, ay));
                    line->setPen(realRoadPen);
					tip = QString("Road Ribbon: %1\nID: %2\nLength: %3\nAngle: %4")
											.arg(ribbon->ribbonid())
											.arg(seg_st->getId())
											.arg(slen)
											.arg(ang);
                    line->setToolTip(tip);
                    roadLayer->addToLayer(line);
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
                    curve->setPen(realRoadPen);
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
                    roadLayer->addToLayer(curve);
					}
                    break;
            }
        }
    }
    return roadLayer;
}

