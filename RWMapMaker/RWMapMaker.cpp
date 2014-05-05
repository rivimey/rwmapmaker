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
#include "Network.h"
#include "qxtwidgetloggerengine.h"
#include "splashscreen.h"

/* #include "vld.h" */

Q_DECLARE_METATYPE(std::string);
Q_DECLARE_METATYPE(RouteMetadata);


RWMapMaker::RWMapMaker(QString &argFilename, QWidget *parent)
	: QMainWindow(parent), loadingAborted(false), rotateExcel(false),
	gMapper(0)
{
	QString fileName;

	qRegisterMetaType<std::string>();
	qRegisterMetaType<RouteMetadata>();
	gGradient = NULL;
	gMapper = NULL;
	rwimporter = NULL;
	rwLocate = NULL;

	ui.setupUi(this);

	init();

	QApplication::processEvents();

	if (argFilename.isEmpty()) {
		//fileName = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\railworks\\Content\\Routes\\76b3a3f0-c9b0-48f2-9eff-1af9b00166d6\\RouteProperties.xml";
		if (!fileName.isEmpty()) {
			qxtLog->info("Loading predefined route: " + fileName);
		}
	}
	else
	{
		fileName = argFilename;
		qxtLog->info("Loading route from arguments: " + fileName);
	}

	// if there is a route to load straight away, queue the request to do it.
	if (fileName != NULL)
	{
		loadFilename = fileName;
		loadTimerId = startTimer(300); // 1/3 second
	}
}

RWMapMaker::~RWMapMaker()
{
	delete gGradient; 		gGradient = NULL;
	delete gMapper; 		gMapper = NULL;
	delete rwimporter; 		rwimporter = NULL;
	delete settings; 		settings = NULL;
	delete rwLocate;
	/* VLDReportLeaks(); */
}

void RWMapMaker::init()
{
	setAttribute(Qt::WA_DeleteOnClose);

	settings = new QSettings();

	QApplication::processEvents();
	
	bool logToWidget = settings->value("logToWidget", true).toBool();
	if (logToWidget)
	{
		qxtLog->addLoggerEngine("logWidget", new QxtWidgetLoggerEngine(ui.loggerText));
	}

	setupFileDialogs(); // sets up rwLocate too.

	setupGradients();  // initialise gGradient and gMapper

	rwimporter = new RWImporter();
    rwimporter->setTempPath(tempFolder);

	routesDialog = new RouteBrowser(rwimporter, rwLocate, this);

	loadingDialog = new LoadingDialog(this);

	prefsDialog = new Preferences();
	prefsDialog->setMapMaker(this);
	prefsDialog->setMapWidget(ui.map);
	prefsDialog->setImporter(rwimporter);
	prefsDialog->setLocater(rwLocate);

	toolbarTerrainHeight = new QSpinBox(this);
	toolbarTerrainHeight->setRange(100,10000);
	toolbarTerrainHeight->setSingleStep(200);
	toolbarTerrainHeight->setSuffix("m");
	toolbarTerrainHeight->setAlignment(Qt::AlignRight);
	toolbarTerrainHeight->setToolTip("Maximum height of terrain colouring.");
	toolbarTerrainHeight->setMinimumWidth(80);
	toolbarTerrainHeight->setValue(gMapper->maxHeight());
	ui.terrainToolBar->addWidget(toolbarTerrainHeight);
	connect(toolbarTerrainHeight, SIGNAL(valueChanged(int)), this, SLOT(maxTerrainValueChanged(int)));

	ui.map->setMouseUpdates(true);
	ui.map->setMapGradient(gMapper);
#ifdef RWMM_HOVER
	ui.map->setDescriptionMessageFn(this, SLOT(mapItemInfo(const MapItemGroupMessage&)));
#endif
	readAppSettings();

	connect(rwimporter, SIGNAL(scanTerrain()),	this, SLOT(scanTerrain_notify()));
	connect(rwimporter, SIGNAL(scanTracks()),	this, SLOT(scanTracks_notify()));
	connect(rwimporter, SIGNAL(scanRoads()),	this, SLOT(scanRoads_notify()));
	connect(rwimporter, SIGNAL(scanScenery()),	this, SLOT(scanScenery_notify()));
	connect(rwimporter, SIGNAL(loadTile(TileType, int, int)), this, SLOT(loadTile_notify(TileType, int, int)));
	connect(this, SIGNAL(reportRouteInfo(QString, double, double)), loadingDialog, SLOT(reportRouteInfo(QString, double, double)));
	connect(this, SIGNAL(reportRouteExtent(int, int, int, int)), loadingDialog, SLOT(reportRouteExtent(int, int, int, int)));
	connect(this, SIGNAL(reportLoadTerrain()),	loadingDialog, SLOT(reportLoadTerrain()));
	connect(this, SIGNAL(reportLoadTrack()),	loadingDialog, SLOT(reportLoadTrack()));
	connect(this, SIGNAL(reportLoadScenery()),	loadingDialog, SLOT(reportLoadScenery()));
	connect(this, SIGNAL(reportLoadRoads()),	loadingDialog, SLOT(reportLoadRoads()));
	connect(loadingDialog, SIGNAL(loadAborted()), this, SLOT(doAbortLoading()));
	connect(this, SIGNAL(aborting()), rwimporter, SLOT(abort()));
	connect(ui.map, SIGNAL(mousePos(QPointF)), this, SLOT(showMousePos(QPointF)));

	QApplication::processEvents();

	setCurrentFile(NULL);
	statusBar()->showMessage("Ready");
}

void RWMapMaker::timerEvent(QTimerEvent *ev)
{
	this->killTimer(loadTimerId);
	if (loadFilename != NULL)
	{
		loadFile(loadFilename, true);
		loadFilename = QString();
	}
}


#ifdef RWMM_HOVER
void RWMapMaker::mapItemInfo(const MapItemGroupMessage &m)
{
	QString htmlPfx = QLatin1String("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
									"<html><head><meta name=\"qrichtext\" content=\"1\" />"
									"<style type=\"text/css\">\n"
									"p, li { white-space: pre-wrap; }\n"
									"thead { font-weight: bold; border-bottom: 1px solid black; }\n"
									".firstcol { border-right: 1px solid black; minimum-width: 33%; }\n"
									".object { font-weight: bold; background-color: #eee; }\n"
									"body { font-family:'MS Shell Dlg 2'; font-size:8.5pt; font-weight:400; font-style:normal; }"
									"</style></head><body>\n"
									"<table><thead><tr><td>Name</td><td>Value</td></tr></thead>"
									"<tbody>");
	QString htmlSfx = QLatin1String("</tbody></table></body></html>");
	QString html;
	ItemDetailGroup groups = m.get();
	foreach(ItemDetails idets, groups)
	{
		html += QLatin1String("<tr class=\"object\"><td class=\"firstcol\">Object</td><td></tr></td>\n");
		for(QMap<QString,QVariant>::const_iterator iter = idets.constBegin(); iter != idets.constEnd(); ++iter)
		{
			html += QLatin1String("<tr><td class=\"firstcol\">") + iter.key() + QLatin1String("</td><td>") + iter.value().toString() + QLatin1String("</tr></td>\n");
		}
	}
	ui.mapInfo->setHtml(htmlPfx + html + htmlSfx);
}
#endif

void RWMapMaker::showMousePos(QPointF p)
{
	RWCoordinate xc(p.x());
	RWCoordinate yc(p.y());
	RWPosition rwpos(xc, yc);

	int routex = xc.getRoute(), routey = yc.getRoute();
	float tilex = xc.getTile(), tiley = yc.getTile();
	float height = rwimporter->getTerrainHeightAt(rwpos);

	QString msg("Origin: %1m, %2m [ Tile: X:%3t(%4m)  Y:%5(%6m) ]  Height %7");
	msg = msg.arg(p.x()).arg(p.y())
		     .arg(routex).arg(tilex)
		     .arg(routey).arg(tiley)
			 .arg(height);

	statusBar()->showMessage(msg);
}

void RWMapMaker::closeEvent(QCloseEvent *ev)
{
	writeSettings();
	ev->accept();
}

void RWMapMaker::resetAbortLoading()
{
	loadingAborted = false;
}

void RWMapMaker::doAbortLoading()
{
	loadingAborted = true;
	emit aborting();
}

bool RWMapMaker::isLoadingAborted()
{
	return loadingAborted;
}

void RWMapMaker::on_actShowMessagesDock_triggered()
{
	if (ui.actShowMessagesDock->isChecked())
		ui.dockLoggerWidget->hide();
	else
		ui.dockLoggerWidget->show();
}

void RWMapMaker::on_actDebugLevel_triggered()
{
	if (ui.actDebugLevel->isChecked())
		qxtLog->enableLogLevels(QxtLogger::DebugLevel|QxtLogger::TraceLevel);
	else
		qxtLog->disableLogLevels(QxtLogger::DebugLevel|QxtLogger::TraceLevel);
}

void RWMapMaker::on_actInfoLevel_triggered()
{
	if (ui.actInfoLevel->isChecked())
		qxtLog->enableLogLevels(QxtLogger::InfoLevel);
	else
		qxtLog->disableLogLevels(QxtLogger::InfoLevel);
}

void RWMapMaker::on_actWarningLevel_triggered()
{
	if (ui.actWarningLevel->isChecked())
		qxtLog->enableLogLevels(QxtLogger::WarningLevel);
	else
		qxtLog->disableLogLevels(QxtLogger::WarningLevel);
}

void RWMapMaker::on_actPreferences_triggered()
{
	prefsDialog->loadPrefs();
	if (prefsDialog->exec() == (int)QDialog::Accepted)
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		prefsDialog->savePrefs();
		writeSettings();

		// update the scene should prefs now be out of date...
		ui.map->createScene();
		QApplication::restoreOverrideCursor();
	}
}

void RWMapMaker::on_actGradientEditor_triggered()
{
	QMessageBox::information(this, "RWMapMaker Gradient Editor",
		"This function is not implemented.");
}

void RWMapMaker::on_actBrowseRoutes_triggered()
{
	writeSettings();
	if (routesDialog->exec() == (int)QDialog::Accepted)
	{
		QString route = routesDialog->getSelectedRoute();
		if (!route.isNull())
		{
			QString fileName = QDir::toNativeSeparators(route);
			loadFile(fileName, true);
		}
	}
}

void RWMapMaker::on_actOpenRoute_triggered()
{
	writeSettings();
	if (openDialog->exec() == (int)QDialog::Accepted)
	{
		QStringList &fileNames = openDialog->selectedFiles();
		QString fileName = QDir::toNativeSeparators(fileNames[0]);
		loadFile(fileName, openDialog->getLoadTerrainTiles());
	}
}

void RWMapMaker::on_actSaveAsExcel_triggered()
{
	saveExcelDialog->selectFile(rwimporter->getRouteName() + " Tiles.xls");
	if (saveExcelDialog->exec() == (int)QDialog::Accepted)
	{
		QStringList &fileNames = saveExcelDialog->selectedFiles();
		QString fileName = fileNames[0];
		exportExcel(fileName);
	}
}

void RWMapMaker::on_actSaveAsPNG_triggered()
{
	bool transparentPNG = settings->value("transparentPNG", false).toBool();
	double dotsPerKm = settings->value("dotsPerKm", 1024.0).toDouble();

	savePNGDialog->selectFile(rwimporter->getRouteName() + ".png");
	savePNGDialog->setTransparentPNG(transparentPNG);
	savePNGDialog->setDotsPerKm(dotsPerKm);

	if (savePNGDialog->exec() == (int)QDialog::Accepted)
	{
		settings->setValue("transparentPNG", savePNGDialog->isTransparentPNG());
		settings->setValue("dotsPerKm", savePNGDialog->getDotsPerKm());

		QStringList &fileNames = savePNGDialog->selectedFiles();
		QString fileName = fileNames[0];
		exportPNG(fileName, savePNGDialog->isTransparentPNG(), savePNGDialog->getDotsPerKm());
	}
}


void RWMapMaker::on_actSaveAsSVG_triggered()
{
	saveSVGDialog->selectFile(rwimporter->getRouteName() + ".svg");
	if (saveSVGDialog->exec() == (int)QDialog::Accepted)
	{
		QStringList &fileNames = saveSVGDialog->selectedFiles();
		QString fileName = fileNames[0];
		exportSVG(fileName);
	}
}

void RWMapMaker::on_actAboutQt_triggered()
{
	QMessageBox::aboutQt(this);
}

void RWMapMaker::on_actRWMapMakerWebsite_triggered()
{
}

void RWMapMaker::on_actAboutRWMapMaker_triggered()
{
	QMessageBox::about(this, "About RWMapMaker",
PROGRAM_NAME_VER "\n\
Route Map Creation for Train Simulator 2014 by Dovetail Games.\n\
\n\
Copyright (c) Ruth Ivimey-Cook 2011-2014 http://www.ivimey.com\n\
\n\
RWMapMaker is free software: you can redistribute it and/or modify it under\n\
the terms of the GNU General Public License as published by the Free Software\n\
Foundation, either version 3 of the License, or (at your option) any later\n\
version.\n\
\n\
RWMapMaker is distributed in the hope that it will be useful, but WITHOUT\n\
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n\
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more\n\
details. You should have received a copy of the GNU General Public License\n\
along with RWMapMaker.  If not, see http://www.gnu.org/licenses/gpl.html.\n\
\n\
This program uses code from:\n\
 - Qt, http://www.qt-project.org\n\
 - Qxt, http://libqxt.bitbucket.org\n\
 - Pugixml, http://www.pugixml.org\n\
 - DirectXTex, http://directxtex.codeplex.com\n\
 - Ned Batchelder, http://nedbatchelder.com\n\
without which it would not have been possible.");
}

void RWMapMaker::on_actPanMoveMap_triggered()
{
	ui.actPanMoveMap->setChecked(true);
	ui.actSelectRegion->setChecked(false);
	ui.map->setCursorMode(MapWidget::Cursor_Hand);
}

void RWMapMaker::on_actSelectRegion_triggered()
{
	ui.actPanMoveMap->setChecked(false);
	ui.actSelectRegion->setChecked(true);
	ui.map->setCursorMode(MapWidget::Cursor_Select);
}

void RWMapMaker::on_actShowMapScale_triggered()
{
	ui.map->showMapScale(ui.actShowMapScale->isChecked());
}

void RWMapMaker::on_actZoomIn_triggered()
{
	ui.map->zoomIn();
}

void RWMapMaker::on_actZoomOut_triggered()
{
	ui.map->zoomOut();
}

void RWMapMaker::on_actZoomAll_triggered()
{
	ui.map->zoomAll();
}

void RWMapMaker::on_actZoomHeight_triggered()
{
	ui.map->zoomHeight();
}

void RWMapMaker::on_actZoomWidth_triggered()
{
	ui.map->zoomWidth();
}

void RWMapMaker::on_actShowTerrain_triggered()
{
	ui.map->showTerrain(ui.actShowTerrain->isChecked());
}

void RWMapMaker::on_actShowRoads_triggered()
{
	ui.map->showRoads(ui.actShowRoads->isChecked());
}

void RWMapMaker::on_actShowScenery_triggered()
{
	ui.map->showScenery(ui.actShowScenery->isChecked());
}

void RWMapMaker::on_actShowTrackGrid_triggered()
{
	ui.map->setGridType(MapWidget::GridType_Track);
}

void RWMapMaker::on_actShowTerrainGrid_triggered()
{
	ui.map->setGridType(MapWidget::GridType_Terrain);
}

void RWMapMaker::on_actShowTrack_triggered()
{
    ui.map->showTrack(ui.actShowTrack->isChecked());
}

void RWMapMaker::on_actShowOrigin_triggered()
{
	ui.map->showOrigin(ui.actShowOrigin->isChecked());
}

void RWMapMaker::on_actShowGrid_triggered()
{
    ui.map->showGridlines(ui.actShowGrid->isChecked());
}

void RWMapMaker::on_actGridAddresses_triggered()
{
    ui.map->showTileAddrs(ui.actGridAddresses->isChecked());
}

void RWMapMaker::on_actExit_triggered()
{
	writeSettings();
	QApplication::exit(0);
}

void RWMapMaker::readAppSettings()
{
	QByteArray bytes = settings->value("geometry", saveGeometry()).toByteArray();
	restoreGeometry(bytes);

	ui.actShowTrack->setChecked(settings->value("showTrack", true).toBool());
	ui.actShowRoads->setChecked(settings->value("showRoads", true).toBool());
	ui.actShowTerrain->setChecked(settings->value("showTerrain", true).toBool());
	ui.actShowScenery->setChecked(settings->value("showScenery", true).toBool());
	ui.actShowGrid->setChecked(settings->value("showGrid", true).toBool());
	ui.actGridAddresses->setChecked(settings->value("showGridAddresses", true).toBool());
	ui.actShowOrigin->setChecked(settings->value("showOrigin", true).toBool());
	int gty = settings->value("gridType").toInt();
	switch(gty)
	{
	case MapWidget::GridType_Track:
		ui.actShowTrackGrid->setChecked(true);
		ui.actShowTerrainGrid->setChecked(false);
		break;

	default:
	case MapWidget::GridType_Terrain:
		ui.actShowTrackGrid->setChecked(false);
		ui.actShowTerrainGrid->setChecked(true);
		break;
	}

	updateUISettingsToWidgets();

	// UI setting here is in preferences dialog, which is updated each time
	// it is invoked, so only update the actual setting from here on.
	ui.map->setGridPen(qvariant_cast<QPen>(settings->value("gridPen", ui.map->getGridPen())));
	ui.map->setTrackPen(qvariant_cast<QPen>(settings->value("trackPen", ui.map->getTrackPen())));
	ui.map->setRoadPen(qvariant_cast<QPen>(settings->value("roadPen", ui.map->getRoadPen())));
	ui.map->setOriginPen(qvariant_cast<QPen>(settings->value("originPen", ui.map->getOriginPen())));

	ui.map->setGridTextBrush(qvariant_cast<QBrush>(settings->value("gridTextBrush", ui.map->getGridTextBrush())));
	ui.map->setGridTextFont(qvariant_cast<QFont>(settings->value("gridTextFont", ui.map->getGridTextFont())));
	ui.map->setTextFont(qvariant_cast<QFont>(settings->value("textFont", ui.map->getTextFont())));

	ui.map->setFastRender(settings->value("fastRender", false).toBool());
	ui.map->showLoftEnds(settings->value("showLoftEnds", false).toBool());
	ui.map->setWheelZoom(qvariant_cast<MapWidget::WheelZoomDir>(settings->value("wheelZoom")));
}

void RWMapMaker::writeSettings()
{
	settings->setValue("geometry", saveGeometry());

	settings->setValue("showTrack", ui.actShowTrack->isChecked());
	settings->setValue("showRoads", ui.actShowRoads->isChecked());
	settings->setValue("showScenery", ui.actShowScenery->isChecked());
	settings->setValue("showTerrain", ui.actShowTerrain->isChecked());
	settings->setValue("showGridAddresses", ui.actGridAddresses->isChecked());
	settings->setValue("showGrid", ui.actShowGrid->isChecked());
	settings->setValue("showOrigin", ui.actShowOrigin->isChecked());
	settings->setValue("gridType", (int)ui.map->getGridType());

	settings->setValue("gridPen", ui.map->getGridPen());
	settings->setValue("trackPen", ui.map->getTrackPen());
	settings->setValue("roadPen", ui.map->getRoadPen());
	settings->setValue("originPen", ui.map->getOriginPen());

	settings->setValue("gridTextBrush", ui.map->getGridTextBrush());
	settings->setValue("gridTextFont", ui.map->getGridTextFont());
	settings->setValue("textFont", ui.map->getTextFont());

	settings->setValue("fastRender", ui.map->isFastRender());
	settings->setValue("showLoftEnds", ui.map->loftEndsShown());
	settings->setValue("wheelZoom", ui.map->getWheelZoom());
	
	// these are written on app exit and read only on init();

	QString routesFolder = openDialog->directory().absolutePath();
	settings->setValue("routesFolder", routesFolder);
	QString imageFolder = savePNGDialog->directory().absolutePath();
	settings->setValue("imageFolder", imageFolder);
	QString excelFolder = saveExcelDialog->directory().absolutePath();
	settings->setValue("excelFolder", excelFolder);
    QString tempFolder = rwimporter->getTempPath();
	settings->setValue("tempFolder", tempFolder);
}


void RWMapMaker::updateUISettingsToWidgets()
{
	ui.map->beginUpdate();

	// Calling these will ensure the UI and map are in sync
	on_actShowTerrain_triggered();
	on_actShowTrack_triggered();
	on_actShowRoads_triggered();
	on_actShowScenery_triggered();
	on_actShowOrigin_triggered();
	on_actGridAddresses_triggered();
	on_actShowGrid_triggered();

	if (ui.actShowTerrainGrid->isChecked()) ui.map->setGridType(MapWidget::GridType_Terrain);
	if (ui.actShowTrackGrid->isChecked())   ui.map->setGridType(MapWidget::GridType_Track);

	on_actDebugLevel_triggered();
	on_actInfoLevel_triggered();
	on_actWarningLevel_triggered();

	ui.map->endUpdate();
}

void RWMapMaker::setCurrentFile(QString name)
{
	setWindowModified(false);
	if (name.isEmpty())
		setWindowTitle(QString("RWMapMaker"));
	else
		setWindowTitle(QString("%1 - RWMapMaker").arg(name));
}

void RWMapMaker::loadTile_notify(TileType ty, int x, int y)
{
	loadingDialog->bumpProgress();
	QCoreApplication::processEvents();
}

void RWMapMaker::scanTerrain_notify()
{
	onReportLoadTerrain();
}

void RWMapMaker::scanTracks_notify()
{
	onReportLoadTrack();
}

void RWMapMaker::scanRoads_notify()
{
	onReportLoadRoads();
}

void RWMapMaker::scanScenery_notify()
{
	onReportLoadScenery();
}

void RWMapMaker::onReportRouteInfo(QString n, double la, double lo)
{
	emit reportRouteInfo(n,la,lo);
}

void RWMapMaker::onReportRouteExtent(int n1, int n2, int n3, int n4)
{
	emit reportRouteExtent(n1, n2, n3, n4);
}

void RWMapMaker::onReportRouteLoaded()
{
	emit reportRouteLoaded();
}

void RWMapMaker::onReportLoadTrack()
{
	emit reportLoadTrack();
}

void RWMapMaker::onReportLoadScenery()
{
	emit reportLoadScenery();
}

void RWMapMaker::onReportLoadRoads()
{
	emit reportLoadRoads();
}

void RWMapMaker::onReportLoadTerrain()
{
	emit reportLoadTerrain();
}

void RWMapMaker::closeFile()
{
	loadingDialog->hide();
	rwimporter->clear();
	ui.map->clear();
	setCurrentFile(NULL);
}

void RWMapMaker::loadFile(QString fileName, bool terrainToo)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	qxtLog->info("Loading route from file: " + fileName);
	resetAbortLoading();
	loadingDialog->setupFile(fileName);
	loadingDialog->show();

	ui.map->clear();
	updateUISettingsToWidgets();
	try
	{
		rwimporter->loadRoute(fileName);
		rwimporter->importRWRouteProps();
		setCurrentFile(rwimporter->getRouteName());
		QCoreApplication::processEvents();

		onReportRouteInfo(rwimporter->getRouteName(),
			rwimporter->getOrigin().x(),
			rwimporter->getOrigin().y());

		// read in basic info about the route's tiles
		rwimporter->importRWTileExtents();
		int terrainTiles = rwimporter->getTerrainTiles();
		if (!terrainToo)
			terrainTiles = 0;
		onReportRouteExtent(terrainTiles, rwimporter->getTrackTiles(), rwimporter->getRoadTiles(), rwimporter->getSceneryTiles());

		statusBar()->showMessage("Loading route files");
		
		if (isLoadingAborted()) { goto aborted; }
		
		// read the track network
		rwimporter->importRWTrack();
		
		if (isLoadingAborted()) { goto aborted; }

		// read in the road network
		rwimporter->importRWRoads();
		
		if (isLoadingAborted()) { goto aborted; }
		
		// read in the scenery items
		rwimporter->importRWScenery();
		
		if (isLoadingAborted()) { goto aborted; }

		if (terrainToo)
		{
			// read in the terrain tiles
			rwimporter->importRWTerrain();
		
			if (isLoadingAborted()) {  goto cleanup; }
		}

		qxtLog->info("Drawing map");
		statusBar()->showMessage("Drawing map");

		ui.map->beginUpdate();
		ui.map->setTerrainTileBitmap(rwimporter->getTerrainTileMap()->getImage(), rwimporter->getTerrainTileMap()->getRegion());
		ui.map->setTrackTileBitmap(rwimporter->getTrackTileMap()->getImage(), rwimporter->getTrackTileMap()->getRegion());
		ui.map->setScenery(rwimporter->getScenery());
		ui.map->setTerrain(rwimporter->getTerrain());
		ui.map->setTrackNetwork(rwimporter->getTrackRibbons());
		ui.map->setRoadNetwork(rwimporter->getRoadRibbons());
		ui.map->endUpdate();

		qxtLog->info("Route loaded");
		statusBar()->showMessage("Route loaded");
		onReportRouteLoaded();
	}
	catch(std::exception ex)
	{
	    QMessageBox mbox;
		qxtLog->error("Exception caught during route loading...", ex.what()); 
		mbox.setText(QString("Unable to read route files:\n") + ex.what());
	    mbox.setStandardButtons(QMessageBox::Cancel);
	    mbox.exec();
	    closeFile();
	}

	ui.map->zoomAll();
	goto cleanup;

aborted:
	closeFile();
	qxtLog->info("Route loading aborted by user");

cleanup:
	resetAbortLoading();
	loadingDialog->hide();
	QCoreApplication::processEvents();
	QApplication::restoreOverrideCursor();
}

void RWMapMaker::setupFileDialogs()
{
	QStringList savefilters;

	// rwlocate provides access to RW installation folder locations
	rwLocate = new RWLocations;
	QStringList routeDirs = rwLocate->getRouteFolders();
	QList<QUrl> urls;
	for(int rp=0; rp < routeDirs.size(); rp++)
	{
		QString file = routeDirs[rp];
		urls << QUrl::fromLocalFile(file);
	}

	{
		QString loc = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
		routesFolder = settings->value("routesFolder", loc).toString();

		QString picloc = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
		imageFolder = settings->value("imageFolder", picloc).toString();

		QString docloc = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		excelFolder = settings->value("excelFolder", picloc).toString();

		QString temploc = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		tempFolder = settings->value("tempFolder", temploc).toString();

		QString confloc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
		confFolder = settings->value("configFolder", confloc).toString();
	}
	urls << QUrl::fromLocalFile(routesFolder);
	urls << QUrl::fromLocalFile(QDir::rootPath());

	// Set up the save and load dialogs. These are long-lived so the
	// info in them is not zapped...
	savePNGDialog = new SavePNGFileDialog(this);
	savePNGDialog->setWindowTitle("Save As PNG Image");
	savePNGDialog->setFileMode(QFileDialog::AnyFile);
	savePNGDialog->setAcceptMode(QFileDialog::AcceptSave);
	savefilters.clear();
	savefilters << "Pixel Image Files (*.png)";
	savePNGDialog->setNameFilters(savefilters);
	savePNGDialog->setDirectory(QDir(imageFolder));

	saveSVGDialog = new QFileDialog(this);
	saveSVGDialog->setWindowTitle("Save As SVG Image");
	saveSVGDialog->setFileMode(QFileDialog::AnyFile);
	saveSVGDialog->setAcceptMode(QFileDialog::AcceptSave);
	savefilters.clear();
	savefilters << "Vector Image Files (*.svg)";
	saveSVGDialog->setNameFilters(savefilters);
	saveSVGDialog->setDirectory(QDir(imageFolder));

	saveExcelDialog = new QFileDialog(this, "Save Excel XLS As");
	saveExcelDialog->setFileMode(QFileDialog::AnyFile);
	saveExcelDialog->setAcceptMode(QFileDialog::AcceptSave);
	savefilters.clear();
	savefilters << "Excel Files (*.xls)";
	saveExcelDialog->setNameFilters(savefilters);
	saveExcelDialog->setDirectory(QDir(excelFolder));

	openDialog = new RouteFileDialog(this);
	openDialog->setWindowTitle("Open Route (select RouteProperties.xml in Route folder)");
	openDialog->setSidebarUrls(urls << openDialog->sidebarUrls());
	openDialog->setFileMode(QFileDialog::ExistingFile);
	openDialog->setAcceptMode(QFileDialog::AcceptOpen);
	openDialog->setNameFilters(QStringList(QString("Route Files (RouteProperties.xml)")));
	openDialog->setDirectory(QDir(routesFolder));
}

void RWMapMaker::setupGradients()
{
	QString appFolder = QCoreApplication::applicationDirPath();
	QString gradientDir = appFolder + "/gradients";
	QString defaultGrad("default.ggr");

	QString gradientFilename = settings->value("gradientFile", gradientDir + "/" + defaultGrad).toString();

	QFile gradFile(gradientFilename);
	if (gradFile.open(QIODevice::ReadOnly))
	{
		qxtLog->info("Loading gradient file: " + gradientFilename);
		QTextStream ts(&gradFile);
		gGradient = new GimpGradient(ts);
		gMapper = new ColourMapper(gGradient);
		gradFile.close();
	}
	
	if (gGradient == NULL)
	{
		qxtLog->warning("Using backup greyscale; gradient : " + gradientFilename + " is not readable\n");

		// a simple gradient of black to white.
		QString backupGradient =
"GIMP Gradient\
Name: Grey\
1\
0.000000 0.500000 1.000000 0.000000 0.000000 0.000000 1.000000 1.000000 1.000000 1.000000 1.000000 0 0 0 0";
		QTextStream ts(&backupGradient);
		gGradient = new GimpGradient(ts);
		gMapper = new ColourMapper(gGradient);
	}


	// does user app dir exist; if not make and populate it.
	QDir confDir(confFolder + "/gradients");
	if (!confDir.exists())
	{
		confDir.mkpath(confDir.path());
		QDir appGrads(gradientDir);
		QStringList grads = appGrads.entryList(QStringList() << "*.ggr", QDir::Files);
		for(int f = 0; f < grads.size(); f++)
		{
			QFile file(gradientDir + "/" + grads[f]);
			QString newFile = confDir.path() + "/" + grads[f];
			file.copy(newFile);
		}
	}

	// populate the gradients menu from the app dir files.
	QStringList gradientEntries = confDir.entryList(QStringList() << "*.ggr", QDir::Files);

	for(int f = 0; f < gradientEntries.size(); f++)
	{
		QString name = QFileInfo(gradientEntries[f]).baseName();
		QString path = confFolder + "/" + gradientEntries[f];
		QAction *act = new QAction(name, this);					// no "ggr" on the end.
		act->setData(path);										// complete file path, not just name
		act->setToolTip(path);

		ui.menuLoad_Colourmap->addAction(act);
	}
	connect(ui.menuLoad_Colourmap, SIGNAL(triggered(QAction*)), this, SLOT(onGradientLoad(QAction*)));
}


void RWMapMaker::onGradientLoad(QAction* act)
{
	QString gradientFilename = act->data().toString();

	QFile gradFile(gradientFilename);
	if (gradFile.open(QIODevice::ReadOnly))
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		qxtLog->info("Loading gradient file: " + gradientFilename);
		QTextStream ts(&gradFile);
		gGradient = new GimpGradient(ts);
		gMapper = new ColourMapper(gGradient);
		if (toolbarTerrainHeight && gMapper)
			toolbarTerrainHeight->setValue(gMapper->maxHeight());
		gradFile.close();
		ui.map->setMapGradient(gMapper);
		QApplication::restoreOverrideCursor();
	}
}

void RWMapMaker::maxTerrainValueChanged(int m)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	statusBar()->showMessage("Remapping terrain colouration...", 1000);
	gMapper->setMaxHeight(m);
	// this is needed because otherwise auto-repeat engages and the spinbox can
	// apparently "double-click".
	QApplication::processEvents(QEventLoop::AllEvents, 20);
	ui.map->setMapGradient(gMapper);
	QApplication::restoreOverrideCursor();
}
