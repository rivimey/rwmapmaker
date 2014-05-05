
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

#include <QtWidgets/QApplication>
#include "qxtwidgetloggerengine.h"

/*!
    \class QxtWidgetLoggerEngine
    \brief The QxtWidgetLoggerEngine class is the base class of qt widget logger engines.
    \inmodule QxtCore

    \sa QxtLogger
 */


/*!
    Constructs a QxtWidgetLoggerEngine with \a fileName and open \a mode.
 */
QxtWidgetLoggerEngine::QxtWidgetLoggerEngine(LogWidgetType *widg)
        : QxtLoggerEngine(), widget(widg)
{
}

/*!
    Destructs the file logger engine.
 */
QxtWidgetLoggerEngine::~QxtWidgetLoggerEngine()
{
    killLoggerEngine();
}

/*!
    \reimp
 */
void QxtWidgetLoggerEngine::initLoggerEngine()
{
    // Are we already logging?  If so, close it and disable logging.
    killLoggerEngine();

	// if there's a widget set, enable the engine
    if (widget != NULL)
	{
	    enableLogging();
	}
}

/*!
    \reimp
 */
void QxtWidgetLoggerEngine::killLoggerEngine()
{
    if (getWidget() != NULL)
    {
		setWidget(NULL);
    }
}

/*!
    \reimp
 */
bool QxtWidgetLoggerEngine::isInitialized() const
{
    return (getWidget() != NULL);
}


/*!
    \reimp
 */
void QxtWidgetLoggerEngine::writeFormatted(QxtLogger::LogLevel level, const QList<QVariant> &messages)
{
	if (messages.isEmpty()) return;

	const qreal MARGIN = 5;

	QString levelStr = QxtLogger::logLevelToStringEx(level);
    QString header = "[" + levelStr + "]\t";

    LogWidgetType* w = getWidget();
    Q_ASSERT(w);

	QTextBlockFormat block;
	block.setIndent(MARGIN);
	block.setTextIndent(0);
	QList<QTextOption::Tab> tabs;
	tabs << QTextOption::Tab(MARGIN, QTextOption::LeftTab);
	tabs << QTextOption::Tab(MARGIN*2, QTextOption::LeftTab);
	block.setTabPositions(tabs);

	QTextCharFormat italic;
	italic.setFontItalic(true);
	QTextCharFormat normal;

	QTextCursor cursor(w->textCursor());
	cursor.movePosition(QTextCursor::End);
	cursor.setBlockFormat(block);
	cursor.insertText(header, italic);

    Q_FOREACH(const QVariant& out, messages)
    {
        if (!out.isNull())
        {
            cursor.insertText(out.toString(), normal);
        }
    }
	cursor.insertBlock();
	w->setTextCursor(cursor);
	QApplication::processEvents();
}

