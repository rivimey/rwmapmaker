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

#include <QString>
#include <QStringList>
#include <QProcess>

#include "serzfile.h"
#include "pugixml.hpp"
#include "rwlocations.h"
#include "qxtlogger.h"

class SerfFileLocator
{
	
public:
	SerfFileLocator();
	~SerfFileLocator();

	
};



SerzFile::SerzFile(QString exePath)
	: QObject(NULL), useTemp(true)
{
	setSerzExePath(exePath);
	tmpPath = QDir::tempPath();
}

SerzFile::~SerzFile()
{
}


QString SerzFile::lastResultStr()
{
	switch (rslt)
	{
	case SerzFile::Ok:
		return "Ok";
	case SerzFile::CouldNotOpen:
		return "Could not open file: " + supplErrorStr;
	case SerzFile::CouldNotDecode:
		return "Could not decode file";
	case SerzFile::CouldNotParse:
		return "Could not parse xml: " + supplErrorStr;
	case SerzFile::DecodeBadInputFile:
		return "Serz failed to open input file";
	case SerzFile::DecodeBadOutputFile:
		return "Serz failed to open output file";
	case SerzFile::UnknownError:
	default:
		return "Unknown error code";
	}
}


pugi::xml_document * SerzFile::loadSerzFile(QString filename)
{
	QString fname, extn, pname, origname, tempname;
	pugi::xml_document *xmlDoc = NULL;

	setSupplError("");

	if (filename.endsWith(".bin") ||
		filename.endsWith(".TgPcDx") ||
		filename.endsWith(".proxybin"))
	{
		QFileInfo fi(filename);

		if (!(fi.exists() && fi.isFile()))
		{
			setResult(SerzFile::CouldNotOpen);
			setSupplError("Encoded file not found or not readable.");
			return NULL;
		}

		extn = fi.suffix();
		fname = fi.completeBaseName();
		pname = fi.absolutePath();
		
		origname = pname + "/" + fname + "." + extn;
		if (useTemp)
		{
			tempname = tmpPath + "/_" + fname + "_" + extn + ".xml";
		}
		else
		{
			tempname = pname + "/_" + fname + "_" + extn + ".xml";
		}
		if (!DecodeSerz(origname, tempname))
		{
			setSupplError("Decoding " + origname);
			return NULL;
		}
		filename = tempname;
	}

	if (filename.endsWith(".xml"))
	{
		QFile xmlFile(filename);
		if (xmlFile.open(QIODevice::ReadOnly))
		{
			QByteArray bytes = xmlFile.readAll();

			xmlDoc = new pugi::xml_document();
			pugi::xml_parse_result result = xmlDoc->load(bytes.constData(), pugi::encoding_auto);
			if (result)
			{
				setResult(SerzFile::Ok);
				setSupplError("");
			}
			else
			{
				delete xmlDoc;
				xmlDoc = NULL;
				setResult(SerzFile::CouldNotParse);
				setSupplError(result.description());
			}
		}
		else
		{
			setResult(SerzFile::CouldNotOpen);
			setSupplError(xmlFile.errorString());
		}
	}
	else
		qxtLog->error("Don't understand file extension of file: " + filename);

	if (!tempname.isEmpty())
	{
		QDir d;
		d.remove(tempname);
	}

	return xmlDoc;
}

void SerzFile::setResult(SerzResult r)
{
	rslt = r;
}

void SerzFile::setUseTempFolder(bool u)
{
	useTemp = u;
}

QString SerzFile::serzExePath()
{
	return serzPath;
}

void SerzFile::setSerzExePath(QString p)
{
	serzPath = RWLocations::resolveSearchPath(p);
}

bool SerzFile::EncodeSerz(QString inPath, QString outPath)
{
	return RunSerz("/bin:", inPath, outPath);
}

bool SerzFile::DecodeSerz(QString inPath, QString outPath)
{
	return RunSerz("/xml:", inPath, outPath);
}

bool SerzFile::RunSerz(const QString op, QString inPath, QString outPath)
{
	bool ok = false;
	int retCode;
	int tries = 0;
	QString in(QDir::toNativeSeparators(inPath));
	QString out(QDir::toNativeSeparators(outPath));
	
	QStringList args = QStringList() << in << op + out;
	QString exe(serzExePath());
	QString wd = QFileInfo(exe).absolutePath();

retry:
	{
		QProcess p;
		QByteArray output;
		tries++;
		p.setProcessChannelMode(QProcess::MergedChannels);
		p.setWorkingDirectory(wd);
		p.start(exe, args, QIODevice::ReadWrite);

		// TODO: make async
		QApplication::processEvents();
		p.waitForFinished(10000);
		output = p.readAllStandardOutput();
		if (output.contains("Conversion complete"))
		{
			setResult(SerzFile::Ok);
			ok = true;
		}
		else if (output.contains("Could not open input file"))
		{
			setResult(SerzFile::DecodeBadInputFile);
			ok = false;
		}
		else if (output.contains("Could not open output file"))
		{
			setResult(SerzFile::DecodeBadOutputFile);
			ok = false;
		}
		else
		{
			if (tries < 2)
				goto retry;
			setResult(SerzFile::UnknownError);
			QString e = p.errorString();
			ok = false;
		}
		retCode = p.exitCode(); // sadly it's always 0...
	}

	return ok;
}
