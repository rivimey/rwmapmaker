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
#include <QGraphicsSceneHoverEvent>
#include "GraphicsItems.h"

#include "Settings.h"

QPoint operator +=(QPoint p, QSize s)
{
	return QPoint(p.x() + s.width(), p.y() + s.height());
}

GraphicsLayerItem::GraphicsLayerItem(QGraphicsItem *parent)
		: QGraphicsObject(parent), hoverHilite(false)
{
	setFlag(QGraphicsItem::ItemHasNoContents);
    setHandlesChildEvents(true);
}

QRectF GraphicsLayerItem::boundingRect() const
{
    return childrenBoundingRect();
}

void GraphicsLayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	//if (hoverHilite)
	//{
	//	QColor red(Qt::red);
	//	painter->setPen(QPen(Qt::NoPen));
	//	painter->setBrush(QBrush(red));
	//	painter->setOpacity(0.25);
	//	painter->fillRect(boundingRect(), Qt::VerPattern);

	//	painter->setPen(QPen(red));
	//	painter->setBrush(Qt::NoBrush);
	//	painter->setOpacity(0.85);
	//	painter->drawRect(boundingRect());
	//}
}

void GraphicsLayerItem::addToLayer(QGraphicsItem *item)
{
	item->setParentItem(this);
}

#ifdef RWMM_HOVER
char *GITypeToString(int t)
{
	switch(t)
	{
	case 1:
		return "Item";
	case 2:
		return "PathItem";
	case 3:
		return "RectItem";
	case 4:
		return "EllipseItem";
	case 5:
		return "PolygonItem";
	case 6:
		return "LineItem";
	case 7:
		return "PixmapItem";
	case 8:
		return "TextItem";
	case 9:
		return "SimpleTextItem";
	case 10:
		return "ItemGroup";
	case 65536:
		return "UserType";

	case Scene_CentText:
		return "CenteredText";
	case Scene_DescItem:
		return "DescItem";
	case Scene_Layer:
		return "Layer";
	case Scene_Curve:
		return "Curve";
	case Scene_Straight:
		return "Straight";
	case Scene_Scenery:
		return "Scenery";
	case Scene_Easement:
		return "Easement";
	case Scene_Ellipse:
		return "Ellipse";
	}
	return "?";
}

void GraphicsLayerItem::hoverEnterEvent(QGraphicsSceneHoverEvent *ev)
{
	hoverHilite = true;
	QGraphicsScene *s = scene();
	if (s)
	{
		MapItemGroupMessage mg;
		QPointF pos = ev->scenePos();
		QList<QGraphicsItem *> possibles = s->items(pos, Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
		for(int i = 0; i < 2 && i < possibles.size(); i++)
		{
			QGraphicsItem *item = possibles[i];
			int t = item->type();

			// based on GraphicsItemDesc - can describe itself.
			GraphicsDescItem *it = dynamic_cast<GraphicsDescItem *>(item);
			if (it)
			{
				const MapItemMessage &msg = it->description();
				mg.set(msg.get());
			}

			// based on QGraphicsItem only
			//if (t <= Scene_CentText)
			if (!it)
			{
				MapItemMessage m;
				m.set(QString("Object"), QString("%1").arg((long)item,8,16));
				m.set(QString("Type"), QString("%1").arg(GITypeToString(t)));
				m.set(QString("Parent"), QString("%1").arg((long)item->parentItem(),8,16));
				m.set(QString("Position"), QString("%1, %2").arg(item->scenePos().x()).arg(item->scenePos().y()));
				m.set(QString("Z Order"), QString("%1").arg(item->zValue()));

				GraphicsCentredText *ct = dynamic_cast<GraphicsCentredText *>(item);
				if (ct)
				{
					QString txt = ct->text();
					m.set(QString("Text"), txt);
				}

				mg.set(m.get());
			}
		}
		emit description(mg);
	}
	update();
}

void GraphicsLayerItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *ev)
{
	hoverHilite = false;
	update();
}
#endif

/*
             **************** Descriptions **************** 
*/

MapItemMessage GraphicsSceneryItem::description()
{
	MapItemMessage m;
	m.set(QString("Type"), QString("Scenery"));
	m.set(QString("Position"), pos());
	m.set(QString("Blueprint"), blueprint);
	return m;
}

MapItemMessage GraphicsCurveItem::description()
{
	MapItemMessage m;
	m.set(QString("Type"), QString("Curve"));
	m.set(QString("Curve"), curvature);
	m.set(QString("Angle"), tangent);
	m.set(QString("Position"), pos());
	m.set(QString("Length"), arclen);
	return m;
}

MapItemMessage GraphicsStraightItem::description()
{
	MapItemMessage m;
	m.set(QString("Type"), QString("Straight"));
	m.set(QString("Angle"), tangent);
	m.set(QString("Position"), pos());
	m.set(QString("Length"), length);
	return m;
}

MapItemMessage GraphicsSpiralItem::description()
{
	MapItemMessage m;
	m.set(QString("Type"), QString("Easement"));
	m.set(QString("StartPos"), pos());
	m.set(QString("Length"), arclen);
	return m;
}

MapItemMessage GraphicsEllipseItem::description()
{
	MapItemMessage m;
	m.set(QString("Type"), QString("Ellipse"));
	return m;
}


#ifdef RWMM_HOVER
/*
             **************** Hover **************** 
*/
void GraphicsDescItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	hoverHilite = true;
	update();
}

void GraphicsDescItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	hoverHilite = false;
	update();
}
#endif


/*
             **************** Straight Lines **************** 
*/

void GraphicsStraightItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(pen());
    painter->drawLine(theLine);
}

void GraphicsStraightItem::setLine(const QLineF &line)
{
    if (theLine == line)
        return;
    prepareGeometryChange();
    theLine = line;
    update();
}

QPainterPath GraphicsStraightItem::shape() const
{
	QPainterPath path;
    if (theLine == QLineF())
        return path;

    path.moveTo(theLine.p1());
    path.lineTo(theLine.p2());

	QPainterPathStroker stroker;
	stroker.setDashPattern(pen().style());
	stroker.setCapStyle(pen().capStyle());
	stroker.setJoinStyle(pen().joinStyle());
	stroker.setWidth(pen().width());
	QPainterPath newpath = stroker.createStroke(path);
    return newpath;
}

QRectF GraphicsStraightItem::boundingRect() const
{
    if (pen().widthF() == 0.0) {
        const qreal x1 = theLine.p1().x();
        const qreal x2 = theLine.p2().x();
        const qreal y1 = theLine.p1().y();
        const qreal y2 = theLine.p2().y();
        qreal lx = qMin(x1, x2);
        qreal rx = qMax(x1, x2);
        qreal ty = qMin(y1, y2);
        qreal by = qMax(y1, y2);
        return QRectF(lx, ty, rx - lx, by - ty);
    }
    return shape().controlPointRect();
}

/*
             **************** Simple Curves **************** 
*/

void GraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    if ((theSpanAngle != 0) && (qAbs(theSpanAngle) % (360 * 16) == 0))
        painter->drawEllipse(theRect);
    else
        painter->drawPie(theRect, theStartAngle, theSpanAngle);
}

void GraphicsEllipseItem::setRect(const QRectF &rect)
{
    if (theRect == rect)
        return;
    prepareGeometryChange();
    theRect = rect;
    update();
}

void GraphicsEllipseItem::setSpanAngle(int a)
{
    if (theSpanAngle == a)
        return;
    prepareGeometryChange();
    theSpanAngle = a;
    update();
}

void GraphicsEllipseItem::setStartAngle(int a)
{
    if (theStartAngle == a)
        return;
    prepareGeometryChange();
    theStartAngle = a;
    update();
}

QPainterPath GraphicsEllipseItem::shape() const
{
    QPainterPath path;
    if (theRect.isNull())
        return path;
    if (theSpanAngle != 360 * 16) {
        path.moveTo(theRect.center());
        path.arcTo(theRect, theStartAngle / 16.0, theSpanAngle / 16.0);
    } else {
        path.addEllipse(theRect);
    }

	QPainterPathStroker stroker;
	stroker.setDashPattern(thePen.style());
	stroker.setCapStyle(thePen.capStyle());
	stroker.setJoinStyle(thePen.joinStyle());
	stroker.setWidth(thePen.width());
	QPainterPath newpath = stroker.createStroke(path);
    return newpath;
}

QRectF GraphicsEllipseItem::boundingRect() const
{
    if (theBoundingRect.isNull()) {
        qreal pw = pen().style() == Qt::NoPen ? qreal(0) : pen().widthF();
        if (pw == 0.0 && theSpanAngle == 360 * 16)
            (const_cast<GraphicsEllipseItem*>(this))->theBoundingRect = theRect;
        else
            (const_cast<GraphicsEllipseItem*>(this))->theBoundingRect = shape().controlPointRect();
    }
    return theBoundingRect;
}

/*
             **************** Centred Text **************** 
*/

GraphicsCentredText::GraphicsCentredText(QString &txt, QGraphicsItem *parent)
	: QGraphicsSimpleTextItem(txt, parent)
{}

QRectF GraphicsCentredText::boundingRect() const
{
	QRectF br (QGraphicsSimpleTextItem::boundingRect());
    return br.translated(-br.width() / 2, -br.height() / 2);
}

void GraphicsCentredText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QRectF br (QGraphicsSimpleTextItem::boundingRect());
    painter->translate(-br.width() / 2, -br.height() / 2);
    QGraphicsSimpleTextItem::paint(painter, option, widget);
}

/*
             **************** Curve Item **************** 
*/

GraphicsCurveItem::GraphicsCurveItem(QGraphicsItem *parent)
    : GraphicsDescItem(parent)
{
    sweep = 0;      // overall angle of arc
    sign = 1;
    arclen = 0;     // length of arc (prop of circumference);
    polylen = 0;
    nPoints = 1;    // number of points in arc
    pointAngle = 0; // diff' angle for each point
    painterpath = NULL;  // set of points to stroke or return BB of.
    curvature = 0;
    tangent = 0;
    needUpdate = true;
	hoverHilite = false;

	setAcceptHoverEvents(true);
}

void GraphicsCurveItem::setCurvature(double curv)
{
    curvature = curv;
    needUpdate = true;
}

void GraphicsCurveItem::setSign(int s)
{
    sign = s;
    needUpdate = true;
}

void GraphicsCurveItem::setTangent(double t)
{
    tangent = t;
    needUpdate = true;
}

void GraphicsCurveItem::setArcLength(double alen)
{
    arclen = alen;
    needUpdate = true;
}

double GraphicsCurveItem::getCurvature()
{
    return curvature;
}

double GraphicsCurveItem::getSweptAngle()
{
    return sweep;
}

double GraphicsCurveItem::getTangent()
{
    return tangent;
}

int GraphicsCurveItem::getSign()
{
    return sign;
}

double GraphicsCurveItem::getArcLength()
{
    return arclen;
}

double GraphicsCurveItem::calcRadius()
{
    return 1.0 / curvature;
}

double GraphicsCurveItem::calcSweptAngle()
{
    // circumf = 2*pi*r, we know a length that is a fraction of this, and we know (1/r);
    //
    // circumf = 2*math.pi*calcRadius();
    // ratio = getArcLength() / circumf
    // if (ratio > 0.999);
    //    raise ValueError('ratio should not be >= 1');
    // sweep = ratio * 2*math.pi 
    //
    // which simplifies to
    // sweep = (getArcLength() / circumf) * 2*math.pi
    //and
    // sweep = (getArcLength() * 2* math.pi / 2*math.pi*calcRadius()) 
    //and
    // sweep = (getArcLength() / calcRadius()) 

    sweep = getArcLength() * curvature;
    return sweep;
}

QPainterPath *GraphicsCurveItem::pointsToQtPath(QPointFList points)
{
    QPainterPath *pointpath = new QPainterPath();
    if (points.size() > 1)
    {
        QPointF xy = points[0];
        pointpath->moveTo(xy);
        foreach (QPointF p, points)
        {
            pointpath->lineTo(p);
        }
    }
    return pointpath;
}

void GraphicsCurveItem::calcPoly()
{
    if (!needUpdate)
        return;

    // work out how many points we split this curve up into.
    nPoints = (int)(CURVE_FACTOR * arclen);
    if (nPoints < 2)
        nPoints = 2;

    // the length of original arc contributed by each line segment
    polylen = arclen / nPoints;

    // the angle rotated-through for each new line segment
    pointAngle = calcSweptAngle() / nPoints;

    //Debug.WriteLine("Curve len {0:.2f}, tan {4:.3f}, sign {5}: {1} pts, sweep {2:.2f}, ptang {3:.4f}\n ".format(arclen, nPoints, sweep, pointAngle, tangent, sign));

    if (getSign() > 0) 
        pointAngle = -pointAngle;

    QPointF axy;
    QPointF xy(0, 0);
    QPointFList points;
    points.append(xy);
    double angle = tangent + pointAngle;
    for (int p = 0; p < nPoints; p++)
    {
        axy = xy + QPointF(cos(angle) * polylen, sin(angle) * polylen);

        angle = angle + pointAngle;
        points.append(xy);
        xy = axy;
    }
    delete painterpath;
	painterpath = pointsToQtPath(points);
    needUpdate = false;
}

QRectF GraphicsCurveItem::boundingRect() const
{
    ((GraphicsCurveItem*)this)->calcPoly();   // need to ensure bounds up to date...
    return painterpath->boundingRect();
}

void GraphicsCurveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    calcPoly();
	if (hoverHilite)
	    painter->setPen(QPen(Qt::red));
	else
	    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPath(*painterpath);
}

QPainterPath GraphicsCurveItem::shape() const
{
    ((GraphicsCurveItem*)this)->calcPoly();   // need to ensure bounds up to date...
    return *painterpath;
}



/*
             **************** Spiral Item **************** 
*/

GraphicsSpiralItem::GraphicsSpiralItem(QGraphicsItem *parent)
    : GraphicsDescItem(parent)
{
    sharpness = 0;
    sign = 1;
    arclen = 0;     // length of arc (prop of circumference);
    polylen = 0;
    nPoints = 1;    // number of points in arc
    pointAngle = 0; // diff' angle for each point
    painterpath = NULL;  // set of points to stroke or return BB of.
    L0 = 0;
    L0tangent = 0;
    tangent = 0;
    needUpdate = true;
	hoverHilite = false;

	setAcceptHoverEvents(true);
}

void GraphicsSpiralItem::setSharpness(double s)
{
    sharpness = s;
    needUpdate = true;
}

void GraphicsSpiralItem::setL0Tangent(double t)
{
    L0tangent = t;
    needUpdate = true;
}

void GraphicsSpiralItem::setL0(double l)
{
    L0 = l;
    needUpdate = true;
}

void GraphicsSpiralItem::setSign(int s)
{
    sign = s;
    needUpdate = true;
}

void GraphicsSpiralItem::setTangent(double t)
{
    tangent = t;
    needUpdate = true;
}

void GraphicsSpiralItem::setArcLength(double alen)
{
    arclen = alen;
    needUpdate = true;
}

void GraphicsSpiralItem::setPen(QPen p)
{
    pen = p;
}

void GraphicsSpiralItem::setBrush(QBrush b)
{
    brush = b;
}

double GraphicsSpiralItem::getSharpness()
{
    return sharpness;
}

double GraphicsSpiralItem::getL0()
{
    return L0;
}

double GraphicsSpiralItem::getL0Tangent()
{
    return L0tangent;
}

double GraphicsSpiralItem::getTangent()
{
    return tangent;
}

int GraphicsSpiralItem::getSign()
{
    return sign;
}

double GraphicsSpiralItem::getArcLength()
{
    return arclen;
}

QPainterPath *GraphicsSpiralItem::pointsToQtPath(QPointFList points)
{
    QPainterPath *pointpath = new QPainterPath();
    if (points.size() > 1)
    {
        QPointF xy = points[0];
        pointpath->moveTo(xy);
        foreach (QPointF p, points)
        {
            pointpath->lineTo(p);
        }
    }
    return pointpath;
}

void GraphicsSpiralItem::calcPoly()
{
    if (!needUpdate)
        return;
}

QRectF GraphicsSpiralItem::boundingRect() const
{
    ((GraphicsSpiralItem*)this)->calcPoly();   // need to ensure bounds up to date...
    return painterpath->boundingRect();
}

void GraphicsSpiralItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    calcPoly();
	if (hoverHilite)
	    painter->setPen(QPen(Qt::red));
	else
	    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawPath(*painterpath);
}

QPainterPath GraphicsSpiralItem::shape() const
{
    ((GraphicsSpiralItem*)this)->calcPoly();   // need to ensure bounds up to date...
    return *painterpath;
}


/*
             **************** GraphicsOverlayView **************** 
*/

GraphicsOverlayView::GraphicsOverlayView(QWidget *parent)
	: pixImage(NULL), pixOpacity(1), pixAlign(Qt::AlignLeft | Qt::AlignTop), QGraphicsView(parent), pixWidget(this)
{
	pixWidget.raise();
	// quality too low for:
	//setViewport(new QGLWidget());
}

GraphicsOverlayView::GraphicsOverlayView(QGraphicsScene *scene, QWidget *parent)
	: pixImage(NULL), pixOpacity(1), pixAlign(Qt::AlignLeft | Qt::AlignTop), QGraphicsView(parent), pixWidget(this)
{
	pixWidget.raise();
	// quality too low for:
	//setViewport(new QGLWidget());
}

GraphicsOverlayView::~GraphicsOverlayView()
{
}

QPixmap *GraphicsOverlayView::overlayPixmap() const
{
	return pixImage;
}

void GraphicsOverlayView::setOverlayPixmap(QPixmap *p)
{
	if (p)
	{
		pixImage = p;
		QPoint topleft = calcAlignment(viewport()->rect(), pixImage->rect());
		pixWidget.setFixedSize(p->width(), p->height());
		pixWidget.move(topleft);
		pixWidget.setPixmap(*p);
		pixWidget.show();
	}
	else
	{
		pixImage = NULL;
		pixWidget.hide();
	}
}

qreal GraphicsOverlayView::overlayOpacity() const
{
	return pixOpacity;
}

/*
	
*/
void GraphicsOverlayView::setOverlayOpacity(qreal o)
{
	pixOpacity = o;
	pixWidget.setWindowOpacity(o);
}

/*
	
*/
Qt::Alignment GraphicsOverlayView::overlayAlignment() const
{
	return pixAlign;
}

/*
	
*/
void GraphicsOverlayView::setOverlayAlignment(Qt::Alignment a)
{
	pixAlign = a;
}

/*
	
*/
void GraphicsOverlayView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	if (pixImage)
	{
		QPoint topleft = calcAlignment(viewport()->rect(), pixImage->rect());
		pixWidget.move(topleft);
	}
}

	
/*
	Return the coordinate of the top left of the overlay pixmap given the
	desired alignment (specified in setOverlayAlignment())
*/
QPoint GraphicsOverlayView::calcAlignment(QRect overall, QRect item) const
{
	double x = overall.x();
	if ( pixAlign & Qt::AlignRight ) {
		x += ( overall.width() - item.width() );
	}
	else if ( pixAlign & Qt::AlignCenter ) {
		x += ( overall.width() - item.width() )/2;
	}

	double y = overall.y();
	if( pixAlign & Qt::AlignVCenter ) {
		y += ( overall.height() / 2 - item.height() / 2 );
	}
	else if ( pixAlign & Qt::AlignBottom ) {
		y += ( overall.height() - item.height() );
	}
	return 	QPoint(x,y);
}

/*
	Return the size in scene coordinates of the provided Rect,
	that uses viewport-relative device coordinates.
	For example, to return the size of the overlay Pixmap use
	    return overlaySize(overlayPixmap()->rect());
*/
QSizeF GraphicsOverlayView::overlaySize(QRect theRect) const
{
	double w,h;

	QPoint a1 = calcAlignment(this->rect(), theRect);
	QPointF p1 = mapToScene(a1);
	a1.setX(a1.x() + theRect.width());
	QPointF p2 = mapToScene(a1);
	w = p1.x() - p2.x();
	h = p1.y() - p2.y();
	double w2 = w*w + h*h;

	a1.setY(a1.x() + theRect.height());
	QPointF p3 = mapToScene(a1);
	w = p3.x() - p2.x();
	h = p3.y() - p2.y();
	double h2 = w*w + h*h;

	return QSizeF(sqrt(w2), sqrt(h2));
}


/*
	Return the size in scene coordinates of the overlay pixmap.
	If there is no pixmap set, return 0,0.
*/
QSizeF GraphicsOverlayView::overlaySize() const
{
	if (pixImage == NULL)
		return QSizeF(0,0);

	return overlaySize(pixImage->rect());
}


