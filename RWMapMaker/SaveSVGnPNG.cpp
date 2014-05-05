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
#include "qxtlogger.h"

QRectF operator *(QRectF op1, qreal mult)
{
	QRectF result;
	result.setBottomLeft(op1.bottomLeft() * mult);
	result.setTopRight(op1.topRight() * mult);
	return result;
}

QRectF &operator *=(QRectF &op1, qreal mult)
{
	op1.setBottomLeft(op1.bottomLeft() * mult);
	op1.setTopRight(op1.topRight() * mult);
	return op1;
}

QRectF operator /(QRectF op1, qreal mult)
{
	QRectF result;
	result.setBottomLeft(op1.bottomLeft() / mult);
	result.setTopRight(op1.topRight() / mult);
	return result;
}

QRectF &operator /=(QRectF &op1, qreal mult)
{
	op1.setBottomLeft(op1.bottomLeft() / mult);
	op1.setTopRight(op1.topRight() / mult);
	return op1;
}


void RWMapMaker::exportSVG(QString fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly))
	{
		qxtLog->error("Selected SVG export file not writable: ", fileName); 
		QMessageBox::warning(this, "RWMapMaker",
			QString("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
		return;
	}
	file.close();

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QGraphicsScene *scene = ui.map->getScene();
	QSvgGenerator *generator = new QSvgGenerator();
	QPainter *p = new QPainter(generator);

	QMessageBox mb;
	mb.setText("Painting...");
	mb.setModal(true);
	mb.setInformativeText("Please wait while the scene is painted...");
	mb.setIcon(QMessageBox::Information);
	mb.show();
	QApplication::processEvents();

	generator->setFileName(fileName);
	generator->setSize(QSize((int)(ui.map->getScene()->width()), (int)(ui.map->getScene()->height())));
	//generator->setViewBox(ui.map->getScene()->sceneRect().toRect());
	generator->setTitle(this->rwimporter->getRouteName());
	generator->setDescription("Map of a Train Simulator Route created by RWMapMaker");

	// the scene is the wrong way round for SVG
	p->scale(1,-1);

	// Render the current view...
	scene->render(p, ui.map->selection());

	// and save it.
	p->end();

	mb.hide();

	QApplication::restoreOverrideCursor();
	statusBar()->showMessage("File saved", 3000);
	delete generator;
	delete p;
}

void RWMapMaker::exportPNG(QString fileName, bool transparentPNG, double dotsPerKm)
{
	QFile file(fileName);

	if (!file.open(QFile::WriteOnly))
	{
		qxtLog->error("Selected PNG export file not writable: ", fileName); 
		QMessageBox::warning(this, "RWMapMaker",
			QString("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
		return;
	}
	file.close();

	QRectF selScene;
	QRectF trect = rwimporter->getTiledRegion();
	QRect selRect = ui.map->selection().normalized();
	if (selRect.isValid())
	{
		QPointF topleft = ui.map->mapToScene(selRect.topLeft());
		QPointF botRight = ui.map->mapToScene(selRect.bottomRight());
		selScene = QRectF(topleft,botRight).normalized();
	}
	else
	{
		selScene = trect * TILE_SIZE_METRES;
	}

	// TODO use ui.map->selection(); to influence this:

	int imgwidth = (int)(dotsPerKm * selScene.width() / 1000);
	int imgheight = (int)(dotsPerKm * selScene.height() / 1000);
	QImage *img;

	// message text shared between the following dialogs:
	QString msg("The combination of Dots per Kilometre and the route size in tiles\n"
				"results in an image of %2 * %3 pixels, requiring %4 MB\n"
				"of memory.\n"
				"\n"
				"%1\n"
				"\n"
				"(Dots per Kilometre is %5, route is %6 x %7 tiles)");

	long long isz = (long long)imgwidth * (long long)imgheight * 4L;
	isz /= 1024*1024;	// calculations in MB.
	if (isz > 1536 || imgwidth > 32760 || imgheight > 32760)	// 1.5GB or either dimension > 32k
	{
		QString info = msg.arg("RWMapMaker does not support creating images of this\n"
			          "size. The maximum dimension (height or width) is 32760.\n")
						.arg(imgwidth)
						.arg(imgheight)
						.arg(isz)
						.arg(dotsPerKm)
						.arg(trect.width())
						.arg(trect.height());

		QMessageBox mb;
		mb.setText("The image is too big to draw.");
		mb.setInformativeText(info);
		mb.setIcon(QMessageBox::Critical);
		mb.setStandardButtons(QMessageBox::Close);
		mb.setDefaultButton(QMessageBox::Close);
		mb.exec();
		statusBar()->showMessage("File save cancelled", 3000);
		return;
	}

	if (isz > 250)		// 250MB
	{
		QString info = msg.arg("Are you sure you want to continue?")
						.arg(imgwidth)
						.arg(imgheight)
						.arg(isz)
						.arg(dotsPerKm)
						.arg(trect.width())
						.arg(trect.height());

		QMessageBox mb;
		mb.setText("The image is rather big.");
		mb.setInformativeText(info);
		mb.setIcon(QMessageBox::Warning);
		mb.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
		mb.setDefaultButton(QMessageBox::No);
		int result = mb.exec();
		if (result == QMessageBox::No)
		{
			statusBar()->showMessage("File save cancelled", 3000);
			return;
		}
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	// Make an image to draw into. Try first for 8-bit RGB.
	if (transparentPNG)
		img = new QImage(imgwidth, imgheight, QImage::Format_ARGB32_Premultiplied);
	else
		img = new QImage(imgwidth, imgheight, QImage::Format_RGB888);

	// Try again with a lower depth (4-bit transparent, 5-6-5 for non-t).
	if (img->isNull())
	{
		if (transparentPNG)
			img = new QImage(imgwidth, imgheight, QImage::Format_ARGB4444_Premultiplied);
		else
			img = new QImage(imgwidth, imgheight, QImage::Format_RGB16);
	}

	if (img->isNull())
	{
		QString info = msg.arg("It was not possible to allocate enough memory for the "
			          "image, even with reduced pixel depth.\n")
						.arg(imgwidth)
						.arg(imgheight)
						.arg(isz)
						.arg(dotsPerKm)
						.arg(trect.width())
						.arg(trect.height());

		QMessageBox mb;
		mb.setText("The image is too big to draw.");
		mb.setInformativeText(info);
		mb.setIcon(QMessageBox::Critical);
		mb.setStandardButtons(QMessageBox::Close);
		mb.setDefaultButton(QMessageBox::Close);
		mb.exec();
		QApplication::restoreOverrideCursor();
		statusBar()->showMessage("File save cancelled", 3000);
		delete img;
		return;
	}

	// want a white background.
	if (!transparentPNG)
		img->fill(Qt::white);

	QPainter *p = new QPainter(img);
	p->setRenderHint(QPainter::Antialiasing);

	QMessageBox mb;
	mb.setText("Painting...");
	mb.setModal(true);
	mb.setInformativeText("Please wait while the scene is painted...");
	mb.setIcon(QMessageBox::Information);
	mb.show();
	QApplication::processEvents();
	
	// flip image so it's not upside-down
	p->scale(1,-1);

	// render the selected area into the full image
	ui.map->getScene()->render(p, QRectF(), selRect, Qt::KeepAspectRatio);
	
	p->end();

	// and save it.
	img->save(fileName);

	mb.hide();

	QApplication::restoreOverrideCursor();

	statusBar()->showMessage("File saved", 3000);
	delete img;
	delete p;
}
