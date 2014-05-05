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

#ifndef ROUTEBROWSER_H
#define ROUTEBROWSER_H

#include <QtWidgets/QWidget>
#include "Importer.h"
#include "ui_routebrowser.h"
#include "rwlocations.h"

class RouteBrowser : public QDialog
{
	Q_OBJECT;

	RWImporter *importer;
	RWLocations *rwLocate;

	QStringList routeDirs;
	QString selectedRouteFilename;

	bool abortRequested;
	bool folderSearch;

	qint64 prevFolderTime;
	long folderCount;

public:
	RouteBrowser(RWImporter *importer, RWLocations *rwLocate, QWidget *parent = 0);
	~RouteBrowser();
	void init();
	void scanRoutes(int selected = -1);

	QString getSelectedRoute();

signals:
	void scanFoundRWFolder(QString f);	// found an RW folder
	void scanFoundRoutes(int r);		// number of routes in a Routes folder
	void scanFoundRoute(QString f);		// name of a specific route folder (GUID)
	void scanFoundRouteName(QString f);	// name of a specific route folder (Name)
	//void scanFoundRouteImage(bool b);	// preload image found for route

private slots:
	void on_folderBrowse_clicked();
	void on_searchFolders_clicked();
	void on_loadRouteButton_clicked();
	void on_cancelButton_clicked();
	void on_removeSelectedFolder_clicked();
	void on_resetDefaultPaths_clicked();

	void rwlocate_seenFolder();
	void rwlocate_seenRWFolder(QString f);
	void routebrowse_foundRWFolder(QString folder);
	void routebrowse_foundNumRoutes(int n);
	void routebrowse_foundRoute(QString name);
	void routebrowse_foundPreload(QString name);
	void routebrowse_loadPreload(QString folder);
	void routebrowse_scanPreload(int n);

	//void on_foldersList_itemClicked(QListWidgetItem * item);
	void on_foldersList_currentItemChanged(QListWidgetItem * item, QListWidgetItem * previous);

	void on_routesList_itemClicked(QListWidgetItem * item);
	void on_routesList_itemDoubleClicked(QListWidgetItem * item);

protected:
	void showEvent(QShowEvent *e);

private:
	void updateFolderList();
	void updateRouteList();
	void scanRouteFolder(QString &route);
	void scanRouteFolder_main(QString &route);
	Ui::RouteBrowser ui;
};

#endif // ROUTEBROWSER_H
