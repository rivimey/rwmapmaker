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
#include "preferences.h"

#include "map.h"
#include "Importer.h"
#include "RWMapMaker.h"
#include "rwlocations.h"

#include <QtWidgets/QFontDialog>


/*!
	Initialise a Preferences object, which is purely a GUI thing: it doesn't
	permanently hold info about anything. State should be loaded in using
	Preferences::loadPrefs() and, if the dialog is accepted,
	Preferences::savePrefs() will read the same info back out again.

	In order for loadPrefs etc to work, the necessary system objects must
	be defined using setMapWidget, setImporter and setMapMaker.
*/
Preferences::Preferences(QWidget *parent)
	: QDialog(parent), locater(NULL), import(NULL), map(NULL), maker(NULL)
{
	ui.setupUi(this);

	setColourPalette(*ui.backgroundColour);
	setColourPalette(*ui.gridPenColour);
	setColourPalette(*ui.trackPenColour);
	setColourPalette(*ui.roadPenColour);
	setColourPalette(*ui.originPenColour);
	setColourPalette(*ui.gridTextPenColour);
}

/*!
	Manage the ScaleWithView checkbox for track lines.
	If set and the line width is 0, set it to 1 (the minimum); else set it to
	0 (cosmetic).
*/
void Preferences::on_trackScaleWithView_stateChanged(int s)
{
	if (ui.trackScaleWithView->isChecked())
	{
		if (ui.trackLineWidth->value() == 0.0)
			ui.trackLineWidth->setValue(1);
		ui.trackLineWidth->setEnabled(true);
	}
	else
	{
		if (ui.trackLineWidth->value() != 0.0)
			ui.trackLineWidth->setValue(0);
		ui.trackLineWidth->setEnabled(false);
	}
}

/*!
	Manage the ScaleWithView checkbox for road lines.
	If set and the line width is 0, set it to 1 (the minimum); else set it to
	0 (cosmetic).
*/
void Preferences::on_roadScaleWithView_stateChanged(int s)
{
	if (ui.roadScaleWithView->isChecked())
	{
		if (ui.roadLineWidth->value() == 0.0)
			ui.roadLineWidth->setValue(1);
		ui.roadLineWidth->setEnabled(true);
	}
	else
	{
		if (ui.roadLineWidth->value() != 0.0)
			ui.roadLineWidth->setValue(0);
		ui.roadLineWidth->setEnabled(false);
	}
}

/*!
	If the track line width changes, modify its enabled and the
	scale with view widgets to maintain the condition that
	{scalewithview => width > 0} and {!scalewithview => width == 0}
*/
void Preferences::on_trackLineWidth_valueChanged(double v)
{
	if (v > 0.0)
	{
		ui.trackLineWidth->setEnabled(true);
		ui.trackScaleWithView->setChecked(true);
	}

	if (v == 0.0)
	{
		ui.trackLineWidth->setEnabled(false);
		ui.trackScaleWithView->setChecked(false);
	}
}

/*!
	If the road line width changes, modify its enabled and the
	scale with view widgets to maintain the condition that
	{scalewithview => width > 0} and {!scalewithview => width == 0}
*/
void Preferences::on_roadLineWidth_valueChanged(double v)
{
	if (v > 0.0)
	{
		ui.roadLineWidth->setEnabled(true);
		ui.roadScaleWithView->setChecked(true);
	}

	if (v == 0.0)
	{
		ui.roadLineWidth->setEnabled(false);
		ui.roadScaleWithView->setChecked(false);
	}
}

/*!
	If autolocate is enabled, setup the text in the path to the
	located path; otherwise, set it to the manual path.
*/
void Preferences::on_pathSerzAutolocate_stateChanged(int s)
{
	if (ui.pathSerzAutolocate->isChecked())
	{
		manualSerzPath = ui.pathSerz->text();
		QString p = RWLocations::resolveSearchPath("trsim:serz.exe");
		ui.pathSerz->setText(p);
		ui.pathSerz->setEnabled(false);
	}
	else
	{
		ui.pathSerz->setText(manualSerzPath);
		ui.pathSerz->setEnabled(true);
	}
}

/*!
	When the serz browse button is clicked get the file's path from the
	user.
*/
void Preferences::on_pathSerzBrowse_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, "Locate a serz.exe file within the simulator", ui.pathSerz->text(), "serz.exe");
	QFileInfo fi(file);
	if (fi.exists() && fi.fileName() == "serz.exe")
	{
		manualSerzPath = file;
		ui.pathSerzAutolocate->setChecked(false); // cause manualpath set and enables path
		ui.pathSerz->setText(file);
	}
}

/*!
	If autolocate is enabled, setup the text in the path to the
	standard temporary folder; otherwise, set it to the manual path.
*/
void Preferences::on_pathTempFolderAutolocate_stateChanged(int s)
{
	if (ui.pathTempFolderAutolocate->isChecked())
	{
		manualTempPath = ui.pathTempFolder->text();
		QString loc = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		ui.pathTempFolder->setText(loc);
		ui.pathTempFolder->setEnabled(false);
	}
	else
	{
		ui.pathTempFolder->setText(manualTempPath);
		ui.pathTempFolder->setEnabled(true);
	}
}

/*!
	When the temp browse button is clicked get the path from the
	user.
*/
void Preferences::on_pathTempFolderBrowse_clicked()
{
    QString file = QFileDialog::getExistingDirectory(this, "Locate the folder to use for temporary file storage", ui.pathTempFolder->text());
	QFileInfo fi(file);
	if (fi.exists())
	{
		manualTempPath = file;
		ui.pathTempFolderAutolocate->setChecked(false); // cause manualpath set and enables path
		ui.pathTempFolder->setText(file);
	}
}


/*!
	If autolocate is enabled, setup the text in the path to the
	standard temporary folder; otherwise, set it to the manual path.
*/
void Preferences::on_pathRailworksRoutesAutolocate_stateChanged(int s)
{
	if (ui.pathRailworksRoutesAutolocate->isChecked())
	{
		manualRoutePath = ui.pathRailworksRoutes->text();
		QStringList f = locater->getRWFolders();
		QString text("[%1] %2");
		text = text.arg(f.size()).arg(f.join(", "));
		ui.pathRailworksRoutes->setText(text);
		ui.pathRailworksRoutes->setCursorPosition(0);
		ui.pathRailworksRoutes->setEnabled(false);
		ui.pathRailworksRoutesBrowse->setEnabled(false);
	}
	else
	{
		ui.pathRailworksRoutes->setText(manualRoutePath);
		ui.pathRailworksRoutes->setEnabled(true);
		ui.pathRailworksRoutesBrowse->setEnabled(true);
	}
}

/*!
	When the temp browse button is clicked get the path from the
	user.
*/
void Preferences::on_pathRailworksRoutesBrowse_clicked()
{
	QString file = QFileDialog::getExistingDirectory(this, "Locate the Railworks folder to use", QDir::rootPath());
	QFileInfo fi(file);
	if (fi.exists())
	{
		manualRoutePath = file;
		ui.pathRailworksRoutesAutolocate->setChecked(false); // cause manualpath set and enables path
		ui.pathRailworksRoutes->setText(file);
	}
}
/*!
	
*/
void Preferences::nameFonts()
{
	QString name;
	
	name = QString("%1 %2, %3pt").arg(gridFont.family()).arg(gridFont.styleName()).arg(gridFont.pointSizeF());
	ui.gridTextFontName->setText(name);

	name = QString("%1 %2, %3pt").arg(textFont.family()).arg(textFont.styleName()).arg(textFont.pointSizeF());
	ui.namesTextFontName->setText(name);
}

/*!
	Select the font to use for grid addresses on the map.
*/
void Preferences::on_gridTextFontChange_clicked()
{
	bool ok;
	QFont newFont = QFontDialog::getFont(&ok, gridFont, this, "Select the grid address font ...");
	if (ok)
	{
		gridFont = newFont;
		nameFonts();
	}
}

/*!
	Select the font to use for other text on the map.
*/
void Preferences::on_namesTextFontChange_clicked()
{
	bool ok;
	QFont newFont = QFontDialog::getFont(&ok, textFont, this, "Select the standard font ...");
	if (ok)
	{
		textFont = newFont;
		nameFonts();
	}
}

/*!
	
*/
Preferences::~Preferences()
{

}

/*!
	load preferences settings into the dialog so the user can edit them
*/
void Preferences::loadPrefs()
{
	// various pen colours
	ui.backgroundColour->setCurrentColor(map->backgroundBrush().color());
	ui.gridPenColour->setCurrentColor(map->getGridPen().color());
	ui.trackPenColour->setCurrentColor(map->getTrackPen().color());
	ui.roadPenColour->setCurrentColor(map->getRoadPen().color());
	ui.originPenColour->setCurrentColor(map->getOriginPen().color());
	ui.gridTextPenColour->setCurrentColor(map->getGridTextBrush().color());
	ui.gridTextFontName->setText(map->getGridTextFont().toString());
	ui.namesTextFontName->setText(map->getTextFont().toString());

	// track and road line widths
	ui.trackLineWidth->setValue(map->getTrackPen().widthF());
	//ui.trackScaleWithView->setChecked(map->getTrackPen().widthF() == 0.0);
	ui.roadLineWidth->setValue(map->getRoadPen().widthF());
	//ui.roadScaleWithView->setChecked(map->getRoadPen().widthF() == 0.0);
	ui.trackLineWidth->setEnabled(ui.trackScaleWithView->isChecked());
	ui.roadLineWidth->setEnabled(ui.roadScaleWithView->isChecked());

	ui.trackExaggerateSmall->setChecked(map->getTrackScaling() == MapWidget::Scale_Exaggerated);
	ui.roadExaggerateSmall->setChecked(map->getRoadScaling() == MapWidget::Scale_Exaggerated);

	// set the font labels to describe the fonts
	gridFont = map->getGridTextFont();
	textFont = map->getTextFont();

	nameFonts();

	manualRoutePath = locater->routeFolder();
	if (locater->isRoutefolderSpecified())
	{
		ui.pathRailworksRoutes->setText(manualRoutePath);
		ui.pathRailworksRoutesAutolocate->setChecked(false);
	}
	else
	{
		ui.pathRailworksRoutesAutolocate->setChecked(true);
	}

	manualSerzPath = ui.pathSerz->text();
	ui.pathSerz->setText(import->getSerzExe());
	ui.pathSerz->setEnabled(!import->getSerzAuto());
	ui.pathSerzAutolocate->setChecked(import->getSerzAuto());

	QString temploc = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString &tempPath = import->getTempPath();
    if (tempPath.isNull())
	{
        ui.pathTempFolder->setText(temploc);
		manualTempPath = temploc;
	}
	else
	{
        ui.pathTempFolder->setText(tempPath);
		manualTempPath = tempPath;
	}
	if (manualTempPath == temploc)
		ui.pathTempFolderAutolocate->setChecked(true);

	// 
	ui.markLoftSegments->setChecked(map->loftEndsShown());
	ui.fastRendering->setChecked(map->isFastRender());
	//ui.reportRouteInconsistencies->setChecked()
	ui.reverseMousewheelZoom->setChecked(map->getWheelZoom() == MapWidget::Zoom_InOut);

	QSettings settings;
	ui.logFilename->setText(settings.value("logFile", QString()).toString());
}

/*!
	save preferences settings from the dialog to the outside world...
*/
void Preferences::savePrefs()
{
	QPen pen;

	map->setBackgroundBrush(ui.backgroundColour->currentColor());
	map->setGridTextBrush(ui.gridTextPenColour->currentColor());
	
	// careful to retain any other properties of these pens... just change colour
	pen = map->getGridPen();   pen.setColor(ui.gridPenColour->currentColor());   map->setGridPen(pen);
	pen = map->getOriginPen(); pen.setColor(ui.originPenColour->currentColor()); map->setOriginPen(pen);

	// change colour and line width (but not other properties) of these...
	pen = map->getTrackPen();   pen.setColor(ui.trackPenColour->currentColor());  pen.setWidthF(ui.trackLineWidth->value()); map->setTrackPen(pen);
	pen = map->getRoadPen();    pen.setColor(ui.roadPenColour->currentColor());   pen.setWidthF(ui.roadLineWidth->value());  map->setRoadPen(pen);
	
	map->setTrackScaling(ui.trackExaggerateSmall->isChecked() ? MapWidget::Scale_Exaggerated : MapWidget::Scale_WithView);
	map->setRoadScaling(ui.roadExaggerateSmall->isChecked() ? MapWidget::Scale_Exaggerated : MapWidget::Scale_WithView);

	map->setGridTextFont(gridFont);
	map->setTextFont(textFont);

	//TODO: CHeck the path is OK before committing
	if (ui.pathSerzAutolocate->isChecked())
	{
		import->setSerzAuto(true);
	}
	else
	{
		import->setSerzAuto(false);
		import->setSerzPath(ui.pathSerz->text());
	}

	import->setTempPath(ui.pathTempFolder->text());

	// these two bools should always be opposite, so if the same a change needs to be made.
	if (ui.pathRailworksRoutesAutolocate->isChecked() && locater->isRoutefolderSpecified())
	{
		// the change is to use the default folders...
		locater->defaultRWLocations();
		locater->defaultRoutes();
	}
	else if (!ui.pathRailworksRoutesAutolocate->isChecked() && !locater->isRoutefolderSpecified())
	{
		// the change is to set a folder...
		locater->setSpecifiedRouteFolder(ui.pathRailworksRoutes->text());
	}
	// 
	map->showLoftEnds(ui.markLoftSegments->isChecked());
	map->setFastRender(ui.fastRendering->isChecked());
	//ui.reportRouteInconsistencies->isChecked()
	map->setWheelZoom(ui.reverseMousewheelZoom->isChecked() ? MapWidget::Zoom_InOut : MapWidget::Zoom_OutIn);

	QSettings settings;
	settings.setValue("logFile", ui.logFilename->text());
}

void Preferences::setMapWidget(MapWidget*w)
{
	map = w;
}

void Preferences::setImporter (RWImporter*i)
{
	import = i;
}

void Preferences::setMapMaker (RWMapMaker*m)
{
	maker = m;
}

void Preferences::setLocater (RWLocations*l)
{
	locater = l;
}

void Preferences::setColourPalette(QtColorPicker &p)
{
	const int numColours = 8;
	const int numShades = 5;
	struct { QColor c; char *n; int f; } colours[] =
	{
		QColor(240,100,0),		"orange",		115,
		QColor("darkred"),		"red",			125,
		QColor("darkolivegreen"),"olivegreen",	125,
		QColor("darkblue"),		"blue",			125,
		QColor("forestgreen"),	"forestgreen",	125,
		QColor("saddlebrown"),	"saddlebrown",	120,
		QColor(50,110,175),		"steelblue",	115,
		QColor(148,44,196),		"orchid",		115,
	};

	p.setPopupShape(Shape_FixedCols);
	p.setPopupRowsOrCols(numShades+1);

	for(int i = 0; i < numColours; i++)
	{
		QColor c(colours[i].c); // copy so can create lighter versions
		QString nm = QString("%1\n%2").arg(colours[i].n).arg(c.name().toUpper());
		p.insertColor(c, nm);

		for(int j = 0; j < numShades; j++)
		{
			c = c.lighter(colours[i].f);
			QString nm = QString("%1\n%2").arg(colours[i].n).arg(c.name().toUpper());
			p.insertColor(c, nm);
		}
	}

    p.insertColor(QColor("black"), tr("black"));			// 0
    p.insertColor(QColor("dimgrey"), tr("dimgrey"));		// 105
    p.insertColor(QColor("gray"), tr("gray"));				// 128
    p.insertColor(QColor("silver"), tr("silver"));			// 192
    p.insertColor(QColor("lightgrey"), tr("lightgrey"));	// 211
    p.insertColor(QColor("white"), tr("white"));			// 255

}

