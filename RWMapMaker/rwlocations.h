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

#ifndef RWLOCATIONS_H
#define RWLOCATIONS_H

class RWLocations;
typedef void (RWLocations::*FilterFunc)(const QString sDir, const QFileInfo &info);

class RWLocations : public QObject
{
	Q_OBJECT;

	QStringList rwFolders;
	QStringList routeFolders;
	QStringList preloadFolders;

	bool abortSearch;				// true while aborting a search.
	bool specifiedRoutefolder;		// user has specified particular location.

public:
	RWLocations();
	~RWLocations();

	QStringList getRWFolders();
	QStringList getRouteFolders();
	QStringList getPreloadFolders();

	void setSpecifiedRouteFolder(QString f);
	bool isRoutefolderSpecified() { return specifiedRoutefolder && rwFolders.size() == 1; }
	QString routeFolder() { return rwFolders[0]; }

	void defaultRWLocations();
	void defaultRoutes();
	void saveRWLocations();
	void loadRWLocations();
	QStringList search_Routefolders();
	QStringList search_RWfolders();
	QStringList search_Preloadfolders(const QString assetsPath);

	static QString resolveSearchPath(QString filename);

	// provide a way to cancel
	void requestAbort()		{  abortSearch = true; }

signals:
	void scanSeenRWFolder(QString f);
	void scanSeenFolder();
	void scanSeenPreload(QString f);
	void scanAborted();

private:
	bool filter_collectRouteFolders(const QString sFilePath, const QFileInfo info);
	bool filter_collectRailworksFolders(const QString sFilePath, const QFileInfo info);
	bool filter_collectPreloadFolders(const QString sDir, const QFileInfo info);

	void recurseDirectory(const QString sDir, bool (RWLocations::*applyFunc)(const QString sDir, const QFileInfo info), int maxdepth = 8);

	// provide a way to cancel
	void checkScanAborted() { if (abortSearch) { emit scanAborted(); } abortSearch = false; }
	void resetScanAborted() { abortSearch = false; }
};

#endif // RWLOCATIONS_H
