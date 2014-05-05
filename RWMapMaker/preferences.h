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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtWidgets/QDialog>
#include "ui_preferences.h"

class MapWidget;
class RWImporter;
class RWMapMaker;
class RWLocations;

class Preferences : public QDialog
{
	Q_OBJECT;

	MapWidget *map;
	RWImporter *import;
	RWMapMaker *maker;
	RWLocations *locater;

	QFont gridFont;
	QFont textFont;

	QString manualSerzPath;
	QString manualRoutePath;
	QString manualTempPath;

public:
	Preferences(QWidget *parent = 0);
	~Preferences();

	void setMapWidget(MapWidget*w);
	void setImporter (RWImporter*i);
	void setMapMaker (RWMapMaker*m);
	void setLocater  (RWLocations*l);

	void loadPrefs();
	void savePrefs();

private:
	Ui::Preferences ui;
	void setColourPalette(QtColorPicker &p);
	void nameFonts();

private slots:
	void on_pathSerzBrowse_clicked();

	void on_pathSerzAutolocate_stateChanged(int s);
	void on_trackLineWidth_valueChanged(double v);
	void on_roadLineWidth_valueChanged(double v);
	void on_trackScaleWithView_stateChanged(int s);
	void on_roadScaleWithView_stateChanged(int s);
	void on_gridTextFontChange_clicked();
	void on_namesTextFontChange_clicked();
    void on_pathTempFolderAutolocate_stateChanged(int s);
    void on_pathTempFolderBrowse_clicked();
	void on_pathRailworksRoutesAutolocate_stateChanged(int s);
	void on_pathRailworksRoutesBrowse_clicked();
};

#endif // PREFERENCES_H
