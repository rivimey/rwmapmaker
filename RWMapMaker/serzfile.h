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

#ifndef SERZFILE_H
#define SERZFILE_H

#include <QtCore/QObject>

namespace pugi {
class xml_document;
};

class SerzFile : public QObject
{
	Q_OBJECT;
	Q_PROPERTY(QString serzExePath READ serzExePath WRITE setSerzExePath);
	Q_PROPERTY(bool useTempFolder READ useTempFolder WRITE setUseTempFolder);

public:
	enum SerzResult
	{
		Ok,
		CouldNotOpen,
		CouldNotDecode,
		CouldNotParse,
		DecodeBadInputFile,
		DecodeBadOutputFile,
		UnknownError
	};

private:
	SerzResult rslt;
	QString serzPath;
	QString tmpPath;
	QString supplErrorStr;
	bool useTemp;

public:
	SerzFile(QString exePath);
	~SerzFile();
	
	pugi::xml_document * SerzFile::loadSerzFile(QString filename);

	bool EncodeSerz(QString inPath, QString outPath);
	bool DecodeSerz(QString inPath, QString outPath);

	SerzResult lastResult() { return rslt; };
	QString lastResultStr();
	QString getSupplError() { return supplErrorStr; };

	bool useTempFolder() { return useTemp; };
	void setUseTempFolder(bool u);

	QString getTempFolder() { return tmpPath; };
	void setTempFolder(QString f) { tmpPath = f; };

	QString serzExePath();
	void setSerzExePath(QString p);

private:
	bool RunSerz(const QString op, QString inPath, QString outPath);
	void setResult(SerzResult r);
	void setSupplError(QString r) { supplErrorStr = r; };
};

#endif // SERZFILE_H
