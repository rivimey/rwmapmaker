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

#ifndef SAVEPNGFILEDIALOG_H
#define SAVEPNGFILEDIALOG_H

#include <QFileDialog>
#include <QDoubleSpinBox>
#include <QCheckBox>

class SavePNGFileDialog : public QFileDialog
{
	Q_OBJECT;
	QLabel *transp_l;
	QLabel *dots_l;
	QDoubleSpinBox *dots;
	QCheckBox *transp;

public:
	SavePNGFileDialog(QWidget *parent = 0);
	~SavePNGFileDialog();

	bool isTransparentPNG();
	void setTransparentPNG(bool t);

	double getDotsPerKm();
	void setDotsPerKm(double d);
};


#endif // SAVEPNGFILEDIALOG_H
