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
#include "splashscreen.h"
#include <QtWidgets/QSplashScreen>
#include <QDate>
#include <QtGui/QPixmap>

SplashScreen::SplashScreen()
	: QDialog(0, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) 
{
	ui.setupUi(this);
	timerId = startTimer(4000); // 4 seconds
}

SplashScreen::~SplashScreen()
{
}

void SplashScreen::timerEvent(QTimerEvent *ev)
{
	killTimer(timerId);
	this->hide();
}
