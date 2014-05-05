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

#include "StdAfx.h"
#include "rwlocations.h"
#include <set>
#include "Windows.h"

#define ROUTE_SEARCH_DEPTH	(8)

struct cistringset_compare
{
	bool operator () (const QString &lhs, const QString &rhs) 
	{
		return lhs.compare(rhs, Qt::CaseInsensitive);
	}
};

int QStringList_removeDuplicates_CI(QStringList &that)
{
    int n = that.size();
    int j = 0;
    std::set<QString, cistringset_compare> seen;
    for (int i = 0; i < n; ++i) {
        const QString &s = that.at(i);
        if (seen.find(s) != seen.end())
            continue;
        seen.insert(s);
        if (j != i)
            (that)[j] = s;
        ++j;
    }
    if (n != j)
        that.erase(that.begin() + j, that.end());
    return n - j;
}



RWLocations::RWLocations()
	: abortSearch(false), specifiedRoutefolder(false)
{
	defaultRWLocations();
	defaultRoutes();
}

RWLocations::~RWLocations()
{
}

void RWLocations::setSpecifiedRouteFolder(QString fldr)
{
	specifiedRoutefolder = true;

	rwFolders.clear();
	routeFolders.clear();

	rwFolders.append(fldr);
	defaultRoutes();
	QDir::setSearchPaths("trsim", rwFolders);
}

QStringList RWLocations::getRWFolders()
{
	return rwFolders;
}

QStringList RWLocations::getRouteFolders()
{
	return routeFolders;
}

QStringList RWLocations::getPreloadFolders()
{
	return preloadFolders;
}

/*!
	Set the search paths to the defaults for this computer, reading from the
	registry and using "known" likely locations from the Steam folder structure.

	Notes: the Steam app uninstall path will reflect the most recent full install
	of the game on the machine and is not affected by verify cache. The RSC entry
	may not be set on some machines: it seems to have been deleted in recent
	editions.

	Steam App 24010 is Train Simulator / Railworks
	Steam App 212430 is Train Simulator 2013 Alpha
*/
void RWLocations::defaultRWLocations()
{
	specifiedRoutefolder = false;

    // try to work out where the railworks folder is.
	QString rwPath;
	QStringList folders;

	QSettings registryLM_Steam("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 212430", QSettings::NativeFormat);
	rwPath = registryLM_Steam.value("InstallLocation", "").toString();
	if (!rwPath.isEmpty() && QFileInfo(rwPath).exists())
		folders << rwPath;

	QSettings registryLM_SteamA("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 24010", QSettings::NativeFormat);
	rwPath = registryLM_SteamA.value("InstallLocation", "").toString();
	if (!rwPath.isEmpty() && QFileInfo(rwPath).exists())
		folders << rwPath;

	QSettings registryLM_RSC("HKEY_LOCAL_MACHINE\\SOFTWARE\\RailSimulator.com\\RailWorks", QSettings::NativeFormat);
	rwPath = registryLM_RSC.value("Install_Path", "").toString();
	if (!rwPath.isEmpty() && QFileInfo(rwPath).exists())
		folders << rwPath;

	rwPath = QString(QDir::rootPath() + "Program Files/Steam/steamapps/common/RailWorks");
	if (!rwPath.isEmpty() && QFileInfo(rwPath).exists())
		folders << rwPath;

	rwPath = QString(QDir::rootPath() + "Program Files (x86)/Steam/steamapps/common/RailWorks");
	if (!rwPath.isEmpty() && QFileInfo(rwPath).exists())
		folders << rwPath;

	for(int i = 0; i < folders.size(); i++)
	{
		QDir fldr(folders[i]);
		fldr.cd(folders[i]);
		rwFolders << fldr.path();
		//rwFolders << fldr.canonicalPath();
	}
	rwFolders.removeDuplicates();

	QDir::setSearchPaths("trsim", rwFolders);
}


/*!
	For each folder in rwFolders, check that the folder contains a Content/Routes
	folder. If it does, add the canonical path to that folder to the routeFolders
	list (made available through getRouteFolders() )

	At the end, remove any duplicates from the resulting list (note we didn't clear
	the routeFolders list to begin with).
*/
void RWLocations::defaultRoutes()
{
    // try to work out where the railworks folder is.
	QString routePath;

	for(int i = 0; i < rwFolders.size(); i++)
	{
		routePath = rwFolders[i] + "/Content/Routes";
		QDir route(routePath);
		if (route.exists())
		{
			routeFolders << route.canonicalPath();
		}
	}
	routeFolders.removeDuplicates();
}


/*!
	Save, using QSettings, the current rwFolders list in the system.
*/
void RWLocations::saveRWLocations()
{
	QSettings settings;

	settings.setValue("paths/rwFolders", rwFolders);
}


/*!
	Load, using QSettings, the current rwFolders list from the system.
*/
void RWLocations::loadRWLocations()
{
	QSettings settings;

	rwFolders = settings.value("paths/rwFolders").toStringList();

}


/*!
	From a given starting point, \a assetsPath, search for folders called "PreLoad" which
	could contain the images, etc, of the route metadata files. a limit of 5 levels deep
	is applied. Starting point should be the pathname to the folder Assets in a Railworks
	distribution. If the scan is aborted using requestAbort(), emit scanAborted() just before
	returning whatever results have been collected.

	For each folder discovered, emit scanSeenPreload with the folder name.

	The list of matching folders is available in getPreloadFolders() until the next time
	search_Preloadfolders function is called, as well as being returned as the value of
	this function.
*/
QStringList RWLocations::search_Preloadfolders(const QString assetsPath)
{
	resetScanAborted();
	preloadFolders.clear();

	recurseDirectory(assetsPath, &RWLocations::filter_collectPreloadFolders, 5);
	checkScanAborted();
	return preloadFolders;
}


/*!
	Search the main filesystems for Railworks Route folders, that is a folder called
	Routes containing folders named as GUIDs at least one of which contains a
	RouteProperties.xml file; or a folder containing a railworks binary, an assets folder,
	and a content/routes folder containing GUID folders. A limit of ROUTE_SEARCH_DEPTH levels
	deep is applied.

	For each folder discovered (matching or otherwise), emit scanSeenFolder. For each folder
	matching the criteria (and added to routeFolders), emit scanSeenRWFolder with the folder
	added. If the scan is aborted using requestAbort(), emit scanAborted() just before returning
	whatever results have been collected.

	The list of matching folders is available in getRouteFolders() until the next search_X
	function is called.
*/
QStringList RWLocations::search_Routefolders()
{
	resetScanAborted();
	rwFolders.clear();

	QFileInfoList drives(QDir::drives());
	for (int d = 0; d < drives.size() && !abortSearch; d++)
	{
		// don't wander onto remote, removable (e.g. floppy), missing, or optical drives...
		int isfixed = GetDriveType(drives[d].path().utf16());
		if (isfixed != DRIVE_FIXED)
			continue;

		recurseDirectory(drives[d].path(), &RWLocations::filter_collectRouteFolders, ROUTE_SEARCH_DEPTH);
	}
	checkScanAborted();
	return routeFolders;
}


/*!
	Search the main filesystems for an install of Railworks, that is a folder containing the
	railworks binary, an assets and contents folder and the binary of serz.	A limit of
	ROUTE_SEARCH_DEPTH levels deep is applied.

	For each folder discovered (matching or otherwise), emit scanSeenFolder. For each folder
	matching the criteria (and added to routeFolders), emit scanSeenRWFolder with the folder
	added. If the scan is aborted using requestAbort(), emit scanAborted() just before returning
	whatever results have been collected.

	The list of matching folders is available in getRWFolders() until the next search_X
	function is called.
*/
QStringList RWLocations::search_RWfolders()
{
	resetScanAborted();
	rwFolders.clear();

	QFileInfoList drives(QDir::drives());
	for (int d = 0; d < drives.size() && !abortSearch; d++)
	{
		// don't wander onto remote, removable (e.g. floppy), missing, or optical drives...
		int isfixed = GetDriveType(drives[d].path().utf16());
		if (isfixed != DRIVE_FIXED)
			continue;

		recurseDirectory(drives[d].path(), &RWLocations::filter_collectRailworksFolders, ROUTE_SEARCH_DEPTH);
	}
	checkScanAborted();
	return rwFolders;
}

void RWLocations::recurseDirectory(const QString sDir, bool (RWLocations::*applyFunc)(const QString, const QFileInfo), int maxdepth)
{
	if (maxdepth <= 0 || applyFunc == NULL)
		return;

	QStringList excluded;
	excluded << "C:/Windows" << "D:/Windows" << "C:/cygwin" << "C:/MSOCache" ;

	if (excluded.contains(sDir, Qt::CaseInsensitive))
		return;

	QDir dir(sDir);
	dir.setFilter(QDir::Dirs);
	QFileInfoList list = dir.entryInfoList();
	for (int iList = 0; iList < list.count() && !abortSearch; iList++)
	{
		QFileInfo info = list[iList];

		if (info.fileName() != ".." && info.fileName() != ".")
		{
			if ((this->*applyFunc)(sDir, info))
			{
				QString sFilePath = info.filePath();
				recurseDirectory(sFilePath, applyFunc, maxdepth-1);
			}
			else
				break;  // found something here...
		}
	}
}

bool RWLocations::filter_collectRouteFolders(const QString sFilePath, const QFileInfo info)
{
	QDir dir(sFilePath);

	emit scanSeenFolder();

	// is this the root of an RW folder?
	if (dir.exists("RailWorks.exe") && dir.exists("Assets") && dir.exists("Content/Routes"))
	{
		dir.cd("Content/Routes");
		dir.setFilter(QDir::Dirs);
		QStringList sl(dir.entryList());
		QStringList fsl = sl.filter(QRegExp("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"));
		if (fsl.size() > 0)
			routeFolders << dir.path();
		
		emit scanSeenRWFolder(dir.path());
		return false; // stop recursing here.
	}

	// Is it an RW routes folder existing outside of a RW Contents folder? (or we should have caught it before.)
	if (dir.exists("Routes"))
	{
		dir.cd("Routes");
		dir.setFilter(QDir::Dirs);
		QStringList sl(dir.entryList());
		QStringList fsl = sl.filter(QRegExp("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"));
		
		// be a little more careful than above.
		for(int i=0; i < fsl.size(); i++)
		{
			if (dir.exists(fsl[i] + "/RouteProperties.xml"))
			{
				routeFolders << dir.path();
				emit scanSeenRWFolder(dir.path());
				return false; // stop recursing here.
			}
		}
	}
	return true; // carry on
}

bool RWLocations::filter_collectRailworksFolders(const QString sFilePath, const QFileInfo info)
{
	QDir dir(sFilePath);

	emit scanSeenFolder();

	// is this the root of an RW folder?
	if (dir.exists("RailWorks.exe") && dir.exists("Assets") && dir.exists("serz.exe") && dir.exists("Content"))
	{
		rwFolders << sFilePath;
		emit scanSeenRWFolder(sFilePath);
		return false; // stop recursing here.
	}

	return true; // carry on
}

bool RWLocations::filter_collectPreloadFolders(const QString sFilePath, const QFileInfo info)
{
	if (info.fileName().compare("PreLoad", Qt::CaseInsensitive) == 0)
	{
		QString path = info.filePath();
		preloadFolders << path;
		emit scanSeenPreload(path);
		return true; // carry on
	}
	return true; // carry on
}


/*!
	Resolve the file path name supplied using the QDir search paths. If the
	prefix of the filename corresponds to a pre-defined search path prefix,
	attempt to replace the path with its expansion. If the resulting path or
	file exists, return that path.

	If there is no registered prefix, or the expansion does not exist, return
	the filename unchanged.

	This code limits possible prefixes to 7 characters. QDir limits prefixes
	to be more than 1 character (so as to prevent conflict with Windows drive
	names).
*/
QString RWLocations::resolveSearchPath(QString filename)
{
	int i = filename.indexOf(':');
	if (i > 1 && i <= 8)  // not c: et al - prefix of 2 or less is forbidden
	{
		QString pfx = filename.left(i);
		QString rest = filename.mid(i+1);
		QStringList paths = QDir::searchPaths(pfx);
		for(int p = 0; p < paths.size(); p++)
		{
			QFileInfo fi(paths[p] + "/" + rest);
			if (fi.exists())
				return fi.absoluteFilePath();
		}
	}
	return filename;
}
