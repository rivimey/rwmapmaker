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

#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QtCore/QList>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

class LoadingDialog : public QDialog
{
	Q_OBJECT;

    QLabel *iconArea;
    QLabel *logText;
    QPushButton *cancelButton;
    QProgressBar *progressBar;
    QHBoxLayout *row1Layout;
    QHBoxLayout *row2Layout;
    QVBoxLayout *col1Layout;

    int loadingAmt;

public:
	LoadingDialog(QWidget *parent = NULL);
	~LoadingDialog();

public slots:
    void resetProgress();
    void bumpProgress();
	void doAbort();

public:
    void setLog(QString newText);
    void appendToLog(QString newText);
    void setupFile(QString fileName);

public slots:
    void reportRouteInfo(QString name, double lat, double lon);
    void reportRouteExtent(int nTerrainTiles, int nTrackTiles, int nRoadTiles, int nSceneryTiles);
    void reportRouteLoaded();
    void reportLoadTrack();
    void reportLoadRoads();
    void reportLoadScenery();
    void reportLoadTerrain();

signals:
	void loadAborted();
};

#endif
