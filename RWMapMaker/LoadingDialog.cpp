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

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtWidgets/QWidget>

#include "LoadingDialog.h"


LoadingDialog::LoadingDialog(QWidget *parent) : QDialog(parent)
{
    //Modal = true;
    setWindowTitle("Loading Route...");
        
    iconArea = new QLabel();
    iconArea->setObjectName("iconArea");
    iconArea->setFrameStyle(QFrame::StyledPanel);
    iconArea->setMinimumSize(QSize(210, 210));
    iconArea->setPixmap(QPixmap(":/RWMapMaker/Resources/MapMakerPersp-ico.png"));
    iconArea->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        
    logText = new QLabel();
    logText->setObjectName("logText");
    logText->setFrameStyle(QFrame::StyledPanel);
    logText->setMinimumSize(QSize(240, 192));
    logText->setMaximumSize(QSize(480, 240));
    logText->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	logText->setWordWrap(true);
        
    cancelButton = new QPushButton();
    cancelButton->setObjectName("cancelButton");
    cancelButton->setText("Abort");
    cancelButton->setShortcut(QKeySequence("Esc"));
    cancelButton->setMinimumSize(QSize(48, 12));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(doAbort()));

    progressBar = new QProgressBar();
    progressBar->setObjectName("progressBar");
    progressBar->setFormat("%v (%p%)");
    progressBar->setAlignment(Qt::AlignCenter);
        
    row1Layout = new QHBoxLayout();
    row1Layout->setObjectName("row1Layout");
    row1Layout->setSpacing(12);
    row1Layout->addWidget(iconArea);
    row1Layout->addWidget(logText);
        
    row2Layout = new QHBoxLayout();
    row2Layout->setObjectName("row2Layout");
    row2Layout->setSpacing(12);
    row2Layout->addWidget(this->cancelButton);
    row2Layout->addStretch(1);
        
    col1Layout = new QVBoxLayout();
    col1Layout->setObjectName("col1Layout");
    col1Layout->setSpacing(12);
    col1Layout->setContentsMargins(QMargins(10,10,10,5));
    col1Layout->addLayout(this->row1Layout);
    col1Layout->addWidget(this->progressBar);
    col1Layout->addLayout(this->row2Layout);
        
    setLayout(col1Layout);
}

LoadingDialog::~LoadingDialog()
{
	delete iconArea;	iconArea = NULL;
	delete logText;		logText = NULL;
	delete cancelButton; cancelButton = NULL;
	delete progressBar; progressBar = NULL;
	delete row1Layout;	row1Layout = NULL;
	delete row2Layout;	row2Layout = NULL;
	delete col1Layout;	col1Layout = NULL;
}

void LoadingDialog::doAbort()
{
	emit loadAborted();
}

void LoadingDialog::resetProgress()
{
    loadingAmt = 0;
    progressBar->setValue(0);
}

void LoadingDialog::bumpProgress()
{
    loadingAmt = progressBar->value() + 1;
    progressBar->setValue(loadingAmt);
}

void LoadingDialog::setLog(QString newText)
{
    logText->setText(newText);
}

void LoadingDialog::appendToLog(QString newText)
{
    QString newmsg = logText->text() + newText;
    logText->setText(newmsg);
}

void LoadingDialog::setupFile(QString fileName)
{
    progressBar->reset();
    progressBar->setRange(0, 20);
    setLog("Loading: " + fileName + "\n");
}

void LoadingDialog::reportRouteInfo(QString name, double lat, double lon)
{
    progressBar->setValue(2);
    QString newmsg = QString("Route name: \"%1\"\nRoute origin: %2, %3\n").arg(name).arg(lat).arg(lon);
    appendToLog(newmsg);
}

void LoadingDialog::reportRouteExtent(int nTerrainTiles, int nTrackTiles, int nRoadTiles, int nSceneryTiles)
{
    loadingAmt = 1;
    progressBar->setValue(1);
	int tot = nTerrainTiles + nTrackTiles + nRoadTiles + nSceneryTiles;
    progressBar->setRange(0, 1 + tot);

    QString newmsg = QString("Route Extent: %1 tiles in total, consisting of:\n"
							 "              %3 track tiles\n"
							 "              %4 road tiles\n"
							 "              %5 scenery tiles\n"
							 "              %2 terrain tiles\n")
							.arg(tot)
							.arg(nTerrainTiles)
							.arg(nTrackTiles)
							.arg(nRoadTiles)
							.arg(nSceneryTiles);
    appendToLog(newmsg);
}

void LoadingDialog::reportRouteLoaded()
{
    appendToLog("Route loaded");
}
        
void LoadingDialog::reportLoadTrack()
{
    appendToLog("Reading track tiles...\n");
}

void LoadingDialog::reportLoadRoads()
{
    appendToLog("Reading road tiles...\n");
}

void LoadingDialog::reportLoadScenery()
{
    appendToLog("Reading scenery tiles...\n");
}

void LoadingDialog::reportLoadTerrain()
{
    appendToLog("Reading terrain tiles...\n");
}
