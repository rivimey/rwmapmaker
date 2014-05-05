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

#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtCore/QTimerEvent>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QAction>
#include <QtWidgets/QActionGroup>
#include <QtConcurrent/QtConcurrent>
#include "Settings.h"
#include "map.h"
#include "Importer.h"
#include "LoadingDialog.h"
#include "LoadRouteEvent.h"
#include "GradGGR.h"
#include "rwlocations.h"
#include "routebrowser.h"
#include "routefiledialog.h"
#include "savepngfiledialog.h"

#include "ui_RWMapMaker.h"
#include "preferences.h"

#include "qxtlogger.h"


class RWMapMaker : public QMainWindow
{
	Q_OBJECT;

	QSettings *settings;
	RWImporter *rwimporter;
	RWLocations *rwLocate;
	GimpGradient *gGradient;
	ColourMapper *gMapper;
	QString loadFilename;
	int loadTimerId;

	SavePNGFileDialog *savePNGDialog;
	QFileDialog *saveSVGDialog;
	QFileDialog *saveExcelDialog;
	RouteFileDialog *openDialog;
	RouteBrowser *routesDialog;
	LoadingDialog *loadingDialog;
	Preferences *prefsDialog;
	bool loadingAborted;
	bool rotateExcel;

	// generic folder locations, setup at prog start.
	QString routesFolder;
	QString imageFolder;
	QString excelFolder;
	QString tempFolder;
	QString confFolder;

public:
	RWMapMaker(QString &argFilename, QWidget *parent = 0);
	~RWMapMaker();

private:
	Ui::RWMapMakerClass ui;

	QSpinBox *toolbarTerrainHeight;

	void setCurrentFile(QString fileName);
	// excel;
	void markTiles(QRect oR, TileExtentMap *extentMap, void* sheet, void* fmt_tile);


protected:
	void timerEvent(QTimerEvent *ev);

	void onReportRouteLoaded();
	void onReportLoadTrack();
	void onReportLoadRoads();
	void onReportLoadScenery();
	void onReportLoadTerrain();
	void onReportRouteInfo(QString n, double la, double lo);
	void onReportRouteExtent(int n1, int n2, int n3, int n4);
    void init();
    void readAppSettings();
    void writeSettings();
    void closeFile();
    void loadFile(QString fileName, bool terrainToo);
    void updateUISettingsToWidgets();
    void exportExcel(QString fileName);
    void exportSVG(QString fileName);
    void exportPNG(QString fileName, bool transparentPNG, double dotsPerKm);
	void setupGradients();
	void setupFileDialogs();

	void resetAbortLoading();
	bool isLoadingAborted();

public slots:
	void closeEvent(QCloseEvent *ev);
	void showMousePos(QPointF p);
	void onGradientLoad(QAction*a);
	void maxTerrainValueChanged(int x);

	void on_actAboutQt_triggered();
    void on_actAboutRWMapMaker_triggered();
	void on_actBrowseRoutes_triggered();
    void on_actExit_triggered();
	void on_actGradientEditor_triggered();
    void on_actGridAddresses_triggered();
	void on_actOpenRoute_triggered();
	void on_actPreferences_triggered();
    void on_actSaveAsPNG_triggered();
    void on_actSaveAsSVG_triggered();
	void on_actSaveAsExcel_triggered();
    void on_actShowGrid_triggered();
    void on_actShowOrigin_triggered();
    void on_actShowRoads_triggered();
    void on_actShowScenery_triggered();
    void on_actShowTerrain_triggered();
    void on_actShowTerrainGrid_triggered();
    void on_actShowTrack_triggered();
    void on_actShowMapScale_triggered();
    void on_actZoomAll_triggered();
    void on_actShowTrackGrid_triggered();
	void on_actRWMapMakerWebsite_triggered();
    void on_actZoomHeight_triggered();
    void on_actZoomIn_triggered();
    void on_actZoomOut_triggered();
    void on_actZoomWidth_triggered();
    void on_actPanMoveMap_triggered();
    void on_actSelectRegion_triggered();

	void on_actShowMessagesDock_triggered();
	void on_actDebugLevel_triggered();
	void on_actInfoLevel_triggered();
	void on_actWarningLevel_triggered();

public slots:
    void loadTile_notify(TileType, int, int);
    void scanTerrain_notify();
    void scanRoads_notify();
    void scanScenery_notify();
    void scanTracks_notify();
	void doAbortLoading();
#ifdef RWMM_HOVER
	void mapItemInfo(const MapItemGroupMessage &m);
#endif

signals:
	void reportRouteInfo(QString , double la, double lo);
	void reportRouteExtent(int n1, int n2, int n3, int n4);
	void reportRouteLoaded();
	void reportLoadTrack();
	void reportLoadRoads();
	void reportLoadScenery();
	void reportLoadTerrain();
	void aborting();

};

