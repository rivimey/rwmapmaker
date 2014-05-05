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

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>

#include "savepngfiledialog.h"

SavePNGFileDialog::SavePNGFileDialog(QWidget *parent)
						: QFileDialog(parent)
{
	transp = new QCheckBox(this);
	
	transp_l = new QLabel(this);
	transp_l->setObjectName("savePngTransparentLab");
	transp_l->setBuddy(transp);
	transp_l->setText("Save as Transparent PNG");

	transp->setChecked(true);
	transp->setObjectName("savePngTransparent");
	transp->setText("");
	
	dots = new QDoubleSpinBox(this);
	
	dots_l = new QLabel(this);
	dots_l->setObjectName("savePngDotsPerKmLab");
	dots_l->setBuddy(dots);
	dots_l->setText("Image Pixels Per Kilometre (large for detail)");

	dots->setValue(100);		// ~0.1 dots per metre (overridable)
	dots->setMinimum(8);		// ~0.01 dots per metre
	dots->setMaximum(4096);		// 4 dots per metre
	dots->setObjectName("savePngDotsPerKm");

	QGridLayout *layout = (QGridLayout*)this->layout();
	if (layout == 0) 
		layout = new QGridLayout(this);
	layout->addWidget(transp_l, 4, 0);
	layout->addWidget(transp, 4, 1);

	layout->addWidget(dots_l, 5, 0);
	layout->addWidget(dots, 5, 1);
}

SavePNGFileDialog::~SavePNGFileDialog()
{
}

bool SavePNGFileDialog::isTransparentPNG()
{
	return transp->isChecked();
}

void SavePNGFileDialog::setTransparentPNG(bool t)
{
	transp->setChecked(t);
}

double SavePNGFileDialog::getDotsPerKm()
{
	return dots->value();
}

void SavePNGFileDialog::setDotsPerKm(double v)
{
	dots->setValue(v);
}

