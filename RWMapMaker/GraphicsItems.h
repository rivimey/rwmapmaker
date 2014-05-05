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

#ifndef GRAPHICSITEMS_H
#define GRAPHICSITEMS_H

#include <QtCore/QPointF>
#include <QtGui/QPainterPath>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGraphicsView>



enum ItemTypes {
	Scene_baseItem =	80000,					// >= GraphicsItem::UserType

	// first things not using description()
	Scene_CentText =	Scene_baseItem,

	// others, needing description()
	Scene_DescItem =	Scene_baseItem + 1,		// should be overriden
	Scene_Layer =		Scene_baseItem + 2,
	Scene_Curve =		Scene_baseItem + 3,
	Scene_Straight =	Scene_baseItem + 4,
	Scene_Scenery =		Scene_baseItem + 5,
	Scene_Easement =	Scene_baseItem + 6,
	Scene_Ellipse = 	Scene_baseItem + 7,

	Scene_FirstDescItem=Scene_DescItem,
};

typedef QList<QPointF> QPointFList;
typedef QMap<QString, QVariant> ItemDetails;
typedef QList<ItemDetails> ItemDetailGroup;

class MapItemGroupMessage
{
	ItemDetailGroup msg;

public:
	MapItemGroupMessage() {}
	virtual ~MapItemGroupMessage() {}

	virtual const ItemDetailGroup &get() const { return msg; }
	virtual void set(const ItemDetails &val) { msg.append(val); };
};

class MapItemMessage
{
	ItemDetails msg;

public:
	MapItemMessage() {}
	virtual ~MapItemMessage() {}

	virtual const ItemDetails &get() const { return msg; }
	virtual void set(const QString &key, const QVariant &val) { msg.insert(key, val); };
};


class GraphicsLayerItem : public QGraphicsObject
{
	Q_OBJECT;

    bool hoverHilite;

public:
	GraphicsLayerItem(QGraphicsItem *parent = NULL);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void addToLayer(QGraphicsItem *item);

	enum { Type = Scene_Layer };
	int type() { return Scene_Layer; }

#ifdef RWMM_HOVER
signals:
	void description(const MapItemGroupMessage &m);

protected:

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
#endif
};



class GraphicsDescItem : public QAbstractGraphicsShapeItem 
{
	bool hoverHilite;

public:
	GraphicsDescItem(QGraphicsItem *parent = NULL) : hoverHilite(false), QAbstractGraphicsShapeItem (parent)
	{
		setAcceptHoverEvents(true);
	}

	virtual MapItemMessage description() = 0;
	enum { Type = Scene_DescItem };
	int type() { return Scene_DescItem; }

#ifdef RWMM_HOVER
protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
#endif
};



class GraphicsCentredText : public QGraphicsSimpleTextItem
{
public:
	GraphicsCentredText(QString &txt, QGraphicsItem *parent = NULL);

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

	enum { Type = Scene_CentText };
	int type() { return Scene_CentText; }
};


class GraphicsStraightItem : public GraphicsDescItem
{
/*
    Position (location)
    Length (1 float)
    StartTangent (2 floats)
*/
    double tangent;				// direction at initial position
	double length;				// length in metres
	QLineF theLine;

public:
	GraphicsStraightItem(QGraphicsItem *parent = NULL)
			: GraphicsDescItem(parent)
	{ }
	GraphicsStraightItem(const QLineF &line, QGraphicsItem *parent = NULL)
			: GraphicsDescItem(parent)
	{ }
	~GraphicsStraightItem() { }

	void setLine(const QLineF &p);
	QLineF line() { return theLine; }

	void setTangent(double a) { tangent = a; }
	double getTangent() const { return tangent; }

	void setLength(double a) { length = a; }
	double getLength() const { return length; }

	MapItemMessage description();
	enum { Type = Scene_Straight };
	int type() { return Scene_Straight; }

    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
};


class GraphicsEllipseItem : public GraphicsDescItem
{
	int theSpanAngle;
	int theStartAngle;
	QRectF theBoundingRect;		// 
	QRectF theRect;				// 
    QPen thePen;				// 
    QBrush theBrush;			// 

public:
	GraphicsEllipseItem(QGraphicsItem *parent = NULL)
			: GraphicsDescItem(parent)
	{}
	GraphicsEllipseItem(const QRectF &rect, QGraphicsItem *parent = NULL)
			: GraphicsDescItem(parent)
	{
		setRect(rect);
	}
	~GraphicsEllipseItem() { }

	void setSpanAngle(int theSpanAngle);
	int spanAngle() const { return theSpanAngle; }

	void setStartAngle(int theStartAngle);
	int startAngle() const { return theStartAngle; }

	void setRect(const QRectF &p);
	QRectF rect() const { return theRect; }

	MapItemMessage description();
	enum { Type = Scene_Ellipse };
	int type() { return Scene_Ellipse; }

    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
};


class GraphicsSceneryItem : public GraphicsEllipseItem
{
/*
    Position (location)
	Blueprint (string)
*/
	QString blueprint;

public:
	GraphicsSceneryItem(QGraphicsItem *parent = NULL)
			: GraphicsEllipseItem(parent)
	{ }
	GraphicsSceneryItem(const QRectF &ellipseRect, QGraphicsItem *parent = NULL)
			: GraphicsEllipseItem(ellipseRect, parent)
	{ }
	~GraphicsSceneryItem() { }

	void setBlueprint(const QString &bp) { blueprint = bp; }
	QString getBlueprint() const { return blueprint; }

	MapItemMessage description();
	enum { Type = Scene_Scenery };
	int type() { return Scene_Scenery; }
};



class GraphicsCurveItem : public GraphicsDescItem
{
/*
    Length (1 float)
    StartPos (location)
    StartTangent (2 floats)
    CurveSign (1 float - either 1 or -1)
	Curvature (1 float)
*/

protected:
    double sweep;				// angle subtended at centre
    int sign;					// left- or right- curve
    double arclen;				// 
    double polylen;				// 
    int nPoints;				// 
    double tangent;				// 
    double pointAngle;			// 
    QPainterPath *painterpath;	// 
    double curvature;			// 
    bool needUpdate;			// 
    bool hoverHilite;			// 

public:
	GraphicsCurveItem(QGraphicsItem *parent = NULL);
	~GraphicsCurveItem() { delete painterpath; }

    void setCurvature(double curv);
    void setSign(int s);
    void setTangent(double t);
    void setArcLength(double alen);
    double getCurvature();
    double getSweptAngle();
    double getTangent();
    int    getSign();
    double getArcLength();

	MapItemMessage description();
	enum { Type = Scene_Curve };
	int type() { return Scene_Curve; }

private:
    double calcRadius();
    double calcSweptAngle();

    QPainterPath *pointsToQtPath(QPointFList points);

    void calcPoly();

public:
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual QPainterPath shape() const;
};


class GraphicsSpiralItem : public GraphicsDescItem
{
/*
    Length (1 floats)
    StartPos (location)
    StartTangent (2 floats)
    CurveSign (1 float - either 1 or -1)
    Sharpness (1 float)
    Offset (1 float)
    OffsetIsZero (bool)
    L0 (1 float)
    TraversalSign (1 float - either 1 or -1)
    L0Offset (4 floats)
    L0Tangent (2 floats)
*/

protected:
    int sign;					// left- or right- curve
    int traversSign;			// ?
    double arclen;				// Y
    double tangent;				// 

    double sharpness;			// ?
    double offset;				// 
    bool offsetIsZero;			// 
    double L0;					// 
    double L0tangent;			// 
    double pointAngle;			// 

    QPainterPath *painterpath;	// 
    QPen pen;					// 
    QBrush brush;				// 
    int nPoints;				// number of points for straight line approx.
    double polylen;				// length of each line in approximation
    bool needUpdate;			// true when parameters <> painterpath
    bool hoverHilite;			// 

public:
	GraphicsSpiralItem(QGraphicsItem *parent = NULL);
	~GraphicsSpiralItem() { delete painterpath; }

    void setSign(int s);
    void setTravSign(int s);
    void setTangent(double t);
    void setArcLength(double alen);
	void setL0(double t);
    void setL0Tangent(double t);
    void setOffset(double t[4]);
    void setL0Offset(double t[4]);
	void setSharpness(double t);

    void setPen(QPen p);
    void setBrush(QBrush p);

    double getSharpness();
    double getL0();
    double getL0Tangent();
    double getTangent();
    int    getSign();
    double getArcLength();

	MapItemMessage description();
	enum { Type = Scene_Easement };
	int type() { return Scene_Easement; }

private:
    double calcRadius();
    double calcSweptAngle();

    QPainterPath *pointsToQtPath(QPointFList points);

    void calcPoly();

public:
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual QPainterPath shape() const;
};



class GraphicsOverlayView : public QGraphicsView
{
	QPixmap *pixImage;
	Qt::Alignment pixAlign;
	qreal pixOpacity;
	QLabel pixWidget;

public:
	GraphicsOverlayView(QWidget *parent = 0);
    GraphicsOverlayView(QGraphicsScene *scene, QWidget *parent = 0);
    ~GraphicsOverlayView();

	qreal overlayOpacity() const;
	void setOverlayOpacity(qreal o);

	QPixmap *overlayPixmap() const;
	void setOverlayPixmap(QPixmap *p);

	Qt::Alignment overlayAlignment() const;
	void setOverlayAlignment(Qt::Alignment a);
	
	QSizeF overlaySize() const;
	QSizeF overlaySize(QRect rect) const;

protected:
	void resizeEvent(QResizeEvent *event);
	QPoint calcAlignment(QRect overall, QRect item) const;
};

#endif
