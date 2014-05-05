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

#include <QListWidget>

#include "routebrowser.h"
#include "preferences.h"


#include <QPainter>
#include <QAbstractItemDelegate>

// Folder List Data:
#define FL_Role_Folder			(Qt::UserRole + 1)

// Route List Data:
#define RL_Role_RouteTitle		(Qt::DisplayRole)
#define RL_Role_Description		(Qt::UserRole + 1)
#define RL_Role_Icon			(Qt::UserRole + 2)
#define RL_Role_RouteFolder		(Qt::UserRole + 3)
#define RL_Role_RouteFilename	(Qt::UserRole + 4)
#define RL_Role_Metadata		(Qt::UserRole + 5)

// min interval between search folder count updates
#define MIN_FOLDERCOUNT_TIME	(250)

// Folder List Data:
// <none>

QRect & operator += ( QRect &rect, const QPoint & by )
{
	QSize s = rect.size();
	rect.setTopLeft(rect.topLeft() + by);
	rect.setSize(s);
	return rect;
}

class RouteListDelegate : public QAbstractItemDelegate
{
	QIcon routeNoIcon;
	QIcon icoAfrica;
	QIcon icoAsia;
	QIcon icoAustralia;
	QIcon icoEurope;
	QIcon icoNorthamerica;
	QIcon icoSouthamerica;
	QIcon icoBasemap;

public:
	RouteListDelegate(QObject *parent = 0);
	virtual ~RouteListDelegate() {};

	void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};


RouteListDelegate::RouteListDelegate(QObject *parent)
{
	routeNoIcon = QIcon(":/RWMapMaker/Resources/RouteNoIcon-ico.png");
	icoAfrica = QIcon(":/RWMapMaker/Resources/africa.png");
	icoAsia = QIcon(":/RWMapMaker/Resources/asia.png");
	icoAustralia = QIcon(":/RWMapMaker/Resources/australia.png");
	icoEurope = QIcon(":/RWMapMaker/Resources/europe.png");
	icoNorthamerica = QIcon(":/RWMapMaker/Resources/northamerica.png");
	icoSouthamerica = QIcon(":/RWMapMaker/Resources/southamerica.png");
	icoBasemap = QIcon(":/RWMapMaker/Resources/basemap.png");
};


void RouteListDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QRect overallRect = option.rect;
	QPen linePen(QColor("#C4C4C4"), 1, Qt::SolidLine);
	QPen lineMarkedPen(QColor("#005A83"), 1, Qt::SolidLine);
	QPen fontPen(QColor("#444"), 1, Qt::SolidLine);
	QPen fontMarkedPen(QColor("#111"), 1, Qt::SolidLine);

	if(option.state & QStyle::State_Selected){
		QLinearGradient gradientSelected(overallRect.left(),overallRect.top(),overallRect.left(),overallRect.height()+overallRect.top());
		gradientSelected.setColorAt(0.0, QColor("#E0FDFF"));
		gradientSelected.setColorAt(1.0, QColor("#B5FBFF"));
		painter->setBrush(gradientSelected);
		painter->drawRect(overallRect);

		//BORDER
		painter->setPen(lineMarkedPen);
		painter->drawRect(overallRect);

		painter->setPen(fontMarkedPen);

	} else {
		//Background
        //Alternating colors
		QLinearGradient gradientStd(overallRect.left(),overallRect.top(),overallRect.left(),overallRect.height()+overallRect.top());
		gradientStd.setColorAt(0.0, QColor("#E2FFD8"));
		gradientStd.setColorAt(1.0, QColor("#D2FFBE"));
		painter->setBrush(gradientStd);
		painter->drawRect(overallRect);

		//Border
		painter->setPen(linePen);
		painter->drawRect(overallRect);

		painter->setPen(fontPen);
	}

	// Get title, description and icon
	QVariant imgVar = index.data(RL_Role_Icon);
	QIcon ic;
	if (imgVar.isValid())
	{
		QPixmap img = qvariant_cast<QPixmap>(imgVar);
		ic = QIcon(img);
	}
	else
		ic = routeNoIcon;

	QString title = index.data(RL_Role_RouteTitle).toString();
	QString description = index.data(RL_Role_Description).toString();
	long fl = index.data(RL_Role_Metadata).toInt();
	RouteMetadata::BrowseFilter browseFilt(fl);

	QRect r;
	r.setTop(overallRect.top() + 5);
	r.setLeft(overallRect.left() + 10);
	r.setBottom(overallRect.bottom() - 5);
	r.setRight(r.left() + 140);
	if (!ic.isNull()) {
		// Icon
		ic.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);
		painter->setPen(Qt::black);
		painter->drawLine(r.bottomLeft(), r.bottomRight());
		painter->drawLine(r.bottomRight(), r.topRight());
	}

	// Title
	painter->setFont( QFont( "Tahoma", 12, QFont::Bold ) );
	QRectF br1 = painter->boundingRect(QRectF(), Qt::AlignTop|Qt::AlignLeft, title);

	r.setTop(overallRect.top()+10);
	r.setLeft(r.right()+10);					// old rh edge is new left
	r.setWidth(br1.width());
	r.setHeight(br1.height());
	painter->drawText(r, Qt::AlignTop|Qt::AlignLeft, title, &r);

	int rightedge = r.right();

	// Description
	painter->setFont( QFont( "Tahoma", 8, QFont::Normal ) );
	QRectF br2 = painter->boundingRect(QRectF(), Qt::AlignTop|Qt::AlignLeft, description);

	r.setTop(r.top() + br1.height() + 10);
	r.setWidth(br2.width());
	r.setHeight(br2.height()*2);
	painter->drawText(r, Qt::AlignLeft, description, &r);

	// Filter-described location of route as map
	rightedge = std::max<int>(r.right(), rightedge);
	// "world" images are 256x128; work out what they scale to here..
	int iw = (80.0/128.0)*256;
	int lft = overallRect.right() - (iw+2);
	if (lft < rightedge)
		lft = rightedge;
	r.setTop(overallRect.top()+2);
	r.setLeft(lft);
	r.setWidth(iw);
	r.setHeight(78);
	Qt::Alignment mapAlign = Qt::AlignCenter;
	icoBasemap.paint(painter, r, mapAlign);
	if (browseFilt.testFlag(RouteMetadata::Region_Africa))
		icoAfrica.paint(painter,r, mapAlign);
	if (browseFilt.testFlag(RouteMetadata::Region_Asia))
		icoAsia.paint(painter, r, mapAlign);
	if (browseFilt.testFlag(RouteMetadata::Region_Australia))
		icoAustralia.paint(painter, r, mapAlign);
	if (browseFilt.testFlag(RouteMetadata::Region_Europe))
		icoEurope.paint(painter, r, mapAlign);
	if (browseFilt.testFlag(RouteMetadata::Region_NorthAmerica))
		icoNorthamerica.paint(painter, r, mapAlign);
	if (browseFilt.testFlag(RouteMetadata::Region_SouthAmerica))
		icoSouthamerica.paint(painter, r, mapAlign);
}

QSize RouteListDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	return QSize(0, 82); 
}

/*!

*/
RouteBrowser::RouteBrowser(RWImporter *import, RWLocations *rwLoc, QWidget *parent)
	: QDialog(parent), importer(import), rwLocate(rwLoc), abortRequested(false), folderCount(0),folderSearch(false)
{
	init();
	updateRouteList();
}

RouteBrowser::~RouteBrowser()
{
}

void RouteBrowser::updateRouteList()
{
	routeDirs = rwLocate->getRouteFolders();
	updateFolderList();
}


void RouteBrowser::updateFolderList()
{
	ui.loadRouteButton->setEnabled(false);
	selectedRouteFilename.clear();
	ui.foldersList->clear();
	for(int i=0; i < routeDirs.size(); i++)
	{
		QListWidgetItem *item = new QListWidgetItem();
		item->setData(FL_Role_Folder, i); // link to routeDirs
		item->setText(QDir::toNativeSeparators(routeDirs[i]));
		ui.foldersList->addItem(item);
	}
}


/*!
	Fetch the list of route folders from rwLocate and call scanRoutes to read in
	the routes stored there.
*/
void RouteBrowser::init()
{
	ui.setupUi(this);

	ui.scanProgress->setMinimum(0);
	ui.scanProgress->setMaximum(0);
	ui.scanProgress->setVisible(false);
	ui.scanText->setText("");
	ui.scanText->setVisible(false);

	ui.routesList->setItemDelegate(new RouteListDelegate(ui.routesList));

	QList<int> si = ui.splitter->sizes();
	si[0] = 80;
	si[1] = 520;
	ui.splitter->setSizes(si);

	connect(this, SIGNAL(scanFoundRWFolder(QString)), this, SLOT(routebrowse_foundRWFolder(QString)));
	connect(this, SIGNAL(scanFoundRouteName(QString)), this, SLOT(routebrowse_foundRoute(QString)));
	connect(this, SIGNAL(scanFoundRoutes(int)), this, SLOT(routebrowse_foundNumRoutes(int)));
	connect(importer, SIGNAL(scanPreloads(int)), this, SLOT(routebrowse_scanPreload(int)));
	connect(importer, SIGNAL(loadPreload(QString)), this, SLOT(routebrowse_loadPreload(QString)));
	connect(rwLocate, SIGNAL(scanSeenPreload(QString)), this, SLOT(routebrowse_foundPreload(QString)));
	connect(rwLocate, SIGNAL(scanSeenFolder()), this, SLOT(rwlocate_seenFolder()));
	connect(rwLocate, SIGNAL(scanSeenRWFolder(QString)), this, SLOT(rwlocate_seenRWFolder(QString)));
}


void RouteBrowser::showEvent(QShowEvent *e)
{
	updateRouteList();
}


void RouteBrowser::on_folderBrowse_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, "Select Route Folder", "C:/");
	if (!dir.isEmpty())
	{
		routeDirs.prepend(dir);
		updateFolderList();
		ui.foldersList->setItemSelected(ui.foldersList->item(0), true);
		scanRoutes(0); // list folders of it
	}
}

void RouteBrowser::on_removeSelectedFolder_clicked()
{
	QList<QListWidgetItem *> items = ui.foldersList->selectedItems();
	for(int i=0; i < routeDirs.size(); i++)
	{
		for(int j = 0; j < items.size(); j++)
		{
			int idx = items[j]->data(FL_Role_Folder).toInt();
			if (i == idx)
				routeDirs.removeAt(i);
		}
	}
	updateFolderList();
}

void RouteBrowser::on_resetDefaultPaths_clicked()
{
	routeDirs.clear();
	updateFolderList();
}

void RouteBrowser::on_searchFolders_clicked()
{
	if (folderSearch)
	{
		rwLocate->requestAbort();
		return;
	}

	routeDirs.clear();
	ui.foldersList->clear();
	updateFolderList();

	folderSearch = true;
	ui.scanText->setVisible(true);
	ui.scanProgress->setVisible(true);
	ui.scanProgress->setValue(0);
	ui.searchFolders->setText("Abort Search");
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();
	
	prevFolderTime = QDateTime::currentMSecsSinceEpoch();
	folderCount = 0;
	routeDirs = rwLocate->search_Routefolders();
	updateFolderList();

	ui.searchFolders->setText("Search for Folders");
	QApplication::restoreOverrideCursor();
	ui.scanProgress->setVisible(false);
	ui.scanText->setText("");
	ui.scanText->setVisible(false);
	QApplication::processEvents();
	folderSearch = false;
}

void RouteBrowser::on_loadRouteButton_clicked()
{
	rwLocate->requestAbort();
	if (!selectedRouteFilename.isEmpty())
		accept();
}

void RouteBrowser::on_cancelButton_clicked()
{
	rwLocate->requestAbort();
	abortRequested = true;
	reject();
}

void RouteBrowser::on_foldersList_currentItemChanged(QListWidgetItem * item, QListWidgetItem * previous)
{
	if (item != NULL)
	{
		rwLocate->requestAbort();
		abortRequested = true;
		if (previous != NULL)
			previous->setSelected(false);
		item->setSelected(true);

		ui.routesList->clear();
		ui.loadRouteButton->setEnabled(false);
		selectedRouteFilename.clear();
		int row = item->listWidget()->row(item);
		scanRouteFolder(routeDirs[row]);
	}
}

void RouteBrowser::on_routesList_itemClicked(QListWidgetItem * item)
{
	// record the selection in case the user then clicks "load".
	selectedRouteFilename = item->data(RL_Role_RouteFilename).toString();
	ui.loadRouteButton->setEnabled(true);
}

void RouteBrowser::on_routesList_itemDoubleClicked(QListWidgetItem * item)
{
	selectedRouteFilename = item->data(RL_Role_RouteFilename).toString();
	ui.loadRouteButton->setEnabled(true);
	accept();
}

/*!
	Return the route selected - i.e. the path to the RouteProperties.xml file.
*/
QString RouteBrowser::getSelectedRoute()
{
	return selectedRouteFilename;
}


/*!
	Update the UI progress to note the discovery of another RW-containing folder.
*/
void RouteBrowser::rwlocate_seenRWFolder(QString f)
{
	folderCount++;

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	prevFolderTime = now + 250;  // lie a little to make the time the folder is displayed longer
	ui.scanText->setText(f);
	QApplication::processEvents();
}

/*!
	Update the UI progress to note the discovery of another RW-containing folder.
*/
void RouteBrowser::rwlocate_seenFolder()
{
	folderCount++;

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	if ((now - prevFolderTime) > 400)
	{
		prevFolderTime = now;
		ui.scanText->setText(QString("%1 folders").arg(folderCount));
		QApplication::processEvents();
	}
}

/*!
	Update the UI progress to note the discovery of another folder.
*/
void RouteBrowser::routebrowse_foundRWFolder(QString folder)
{
	ui.scanProgress->setValue(0);
	ui.scanText->setText("Found Railworks in: " + folder);
	QApplication::processEvents();
}

/*!
	Update the UI progress to note how many routes exist within a Routes folder
*/
void RouteBrowser::routebrowse_foundNumRoutes(int n)
{
	ui.scanProgress->setMaximum(n);
	QApplication::processEvents();
}

/*!
	Update the UI progress to note the discovery of another route.
*/
void RouteBrowser::routebrowse_foundRoute(QString name)
{
	int v = ui.scanProgress->value()+1;
	ui.scanProgress->setValue(v);
	ui.scanText->setText("Found Route: " + name);
	QApplication::processEvents();
}

/*!
	Update the UI progress to note the discovery of another asset preload folder.
*/
void RouteBrowser::routebrowse_foundPreload(QString folder)
{
	ui.scanText->setText("Found Preloads folder: " + folder);
	QApplication::processEvents();
}

/*!
	Update the UI progress to note the number of items in the preload folder
*/
void RouteBrowser::routebrowse_scanPreload(int n)
{
	ui.scanProgress->setValue(0);
	ui.scanProgress->setMaximum(n);
	QApplication::processEvents();
}

/*!
	Update the UI progress to note loading of items in a preload folder
*/
void RouteBrowser::routebrowse_loadPreload(QString folder)
{
	int v = ui.scanProgress->value()+1;
	ui.scanProgress->setValue(v);
	ui.scanText->setText("Loading Preloads in: " + folder);
	QApplication::processEvents();
}

/*!
	For each route in routeDirs, load the
*/
void RouteBrowser::scanRoutes(int selected)
{
	ui.routesList->clear();
	ui.scanText->setVisible(true);
	ui.scanProgress->setVisible(true);
	ui.scanProgress->setValue(0);
	abortRequested = false;

	if (selected >= 0)
		scanRouteFolder_main(routeDirs[selected]);
	else
	{
		for(int rp=0; rp < routeDirs.size() && !abortRequested; rp++)
			scanRouteFolder_main(routeDirs[rp]);
	}

	ui.scanProgress->setVisible(false);
	ui.scanText->setText("");
	ui.scanText->setVisible(false);
}

void RouteBrowser::scanRouteFolder(QString &route)
{
	ui.scanText->setVisible(true);
	ui.scanProgress->setVisible(true);
	ui.scanProgress->setValue(0);
	abortRequested = false;

	scanRouteFolder_main(route);

	ui.scanProgress->setVisible(false);
	ui.scanText->setText("");
	ui.scanText->setVisible(false);
}

void RouteBrowser::scanRouteFolder_main(QString &route)
{
	QDir dir(route);
	dir.setFilter(QDir::Dirs);
	QStringList sl(dir.entryList());
	// dirs must match a GUID regex to be considered route folders
	QStringList fsl = sl.filter(QRegExp("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"));

	emit scanFoundRoutes(fsl.size());

	// preload files are not cleared down automatically as they apply to many routes.
	importer->clearPreloads();

	for(int i=0; i < fsl.size() && !abortRequested; i++)
	{
		QString routeGuid = fsl[i];
		emit scanFoundRoute(routeGuid);

		QString fileName = dir.path() + "/" + routeGuid + "/RouteProperties.xml";
		importer->loadRoute(fileName);
		importer->importRWRouteProps();

		emit scanFoundRouteName(importer->getRouteName());

		if (i == 0)  // don't scan assets multiple times
		{
			ui.scanText->setText("Loading preloads... ");
			ui.scanProgress->setValue(0);
			QApplication::processEvents();
			importer->importRWPreloads(*rwLocate);
		}

		QApplication::processEvents();

		// fetch, if possible, the route preload image. It may not exist!
		QPixmap *img = importer->getRouteImage(route, routeGuid, QSize(100,80));
		RouteMetadata *rd = importer->getRouteData(route, routeGuid);

		//emit scanFoundRouteImage(img != NULL);

		QString descrText;
		descrText = QString("Origin: %2 (N-S), %3 (E-W)\n").arg(importer->getOrigin().x()).arg(importer->getOrigin().y());
		descrText += QString("Route ID: %1").arg(routeGuid);
		QListWidgetItem *item = new QListWidgetItem();

		item->setData(RL_Role_RouteFolder, route);
		item->setData(RL_Role_RouteFilename, fileName);
		item->setData(RL_Role_RouteTitle, importer->getRouteName());
		item->setData(RL_Role_Description, descrText);
		item->setData(RL_Role_Metadata, rd == NULL ? 0 : (int)rd->getBrowseFilter());
		if (img != NULL && !img->isNull())
			item->setData(RL_Role_Icon, *img);
		ui.routesList->addItem(item);
	}
}

