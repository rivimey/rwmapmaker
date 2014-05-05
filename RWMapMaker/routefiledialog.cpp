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

#include <QtWidgets/QGridLayout>

#include "routefiledialog.h"

RouteFileDialog::RouteFileDialog(QWidget *parent)
						: QFileDialog(parent)
{
	QRect geom = this->geometry();
	geom.setWidth(geom.width() * 4 / 3);
	geom.setHeight(geom.height() * 5 / 4);
	resize(geom.size());

	cb = new QCheckBox(this);
	
	cb_l = new QLabel(this);
	cb_l->setObjectName("routeTerrainLoadLab");
	cb_l->setBuddy(cb);
	cb_l->setText("Load Terrain Tiles (slower)");

	cb->setChecked(true);
	cb->setObjectName("routeTerrainLoad");
	cb->setText("");

	maxHeightSl = new QSlider(this);
	maxHeightSb = new QSpinBox(this);

	maxHeightSb->setRange(100, 10*1000);
	maxHeightSb->setSuffix("m");
	maxHeightSb->setSingleStep(100);
	maxHeightSb->setMinimumWidth(80);

	maxHeightSl->setOrientation(Qt::Horizontal);
	maxHeightSl->setTickInterval(1000);
	maxHeightSl->setTickPosition(QSlider::TicksBelow);
	maxHeightSl->setRange(100, 10*1000);

	mh_l = new QLabel(this);
	mh_l->setObjectName("maxHeightSelect");
	mh_l->setBuddy(cb);
	mh_l->setText("Max Terrain Height:");

	mh_l		->setToolTip("The maximum height terrain will be coloured. Cannot be changed after load.");
	maxHeightSl	->setToolTip("The maximum height terrain will be coloured. Cannot be changed after load.");

	QWidget *wi_l = new QWidget();
	QHBoxLayout *hb_l = new QHBoxLayout();
	hb_l->addWidget(maxHeightSl);
	hb_l->addWidget(maxHeightSb);
	wi_l->setLayout(hb_l);

	connect(maxHeightSl, SIGNAL(valueChanged(int)), maxHeightSb, SLOT(setValue(int)));
	connect(maxHeightSb, SIGNAL(valueChanged(int)), maxHeightSl, SLOT(setValue(int)));

	QGridLayout *layout = (QGridLayout*)this->layout();
	if (layout == 0) 
		layout = new QGridLayout(this);
	layout->addWidget(cb_l, 4, 0);
	layout->addWidget(cb, 4, 1);
	layout->addWidget(mh_l, 5, 0);
	layout->addWidget(wi_l, 5, 1);
}

RouteFileDialog::~RouteFileDialog()
{
}

bool RouteFileDialog::getLoadTerrainTiles()
{
	return cb->isChecked();
}
