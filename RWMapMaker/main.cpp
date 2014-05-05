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
#include <QtWidgets/QApplication>

#include "RWMapMaker.h"
#include "splashscreen.h"
#include "qxtbasicfileloggerengine.h"

void setupLogEngine()
{
	QSettings settings;

	// set up the event logger
	bool logToFile = settings.value("logToFile", false).toBool();
	if (logToFile)
	{
		QString logFilename = settings.value("logFile", QString("C:/Temp/rwmapmaker.log")).toString();
		if (!logFilename.isEmpty())
		{
			qxtLog->addLoggerEngine("logFile", new QxtBasicFileLoggerEngine(logFilename));
		}
	}
	qxtLog->disableLogLevels(QxtLogger::DebugLevel|QxtLogger::TraceLevel);
}

int main(int argc, char *argv[])
{
	SplashScreen *ss;
	QApplication a(argc, argv);
	QString routeFile;

	QCoreApplication::setOrganizationName("IvimeyCom");
	QCoreApplication::setOrganizationDomain("ivimey.com");
	QCoreApplication::setApplicationName(PROGRAM_NAME);

	setupLogEngine();

	for(int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (strcmp(argv[i], "--help") == 0)
			{
				printf(PROGRAM_NAME " (c) 2013-2014 IvimeyCom, by Ruth Ivimey-Cook\n"
						"\nUsage: RWMapMaker [--help] [path..to..RouteProperties.xml]\n");
				exit(0);
			}
		}
		else
		{
			routeFile = argv[i];
			break;
		}
	}

	ss = new SplashScreen();
	ss->show();
	try
	{
		RWMapMaker *w = new RWMapMaker(routeFile);
		w->show();
		return a.exec();
	}
	catch(std::exception ex)
	{
		printf(PROGRAM_NAME " Exception caught:\n%s", ex.what());
		exit(1);
	}
	catch(...)
	{
		printf(PROGRAM_NAME " Exception caught:\n");
		exit(2);
	}
}
