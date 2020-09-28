/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2017-2020 Syping
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef JSHIGHLIGHTER_H
#define JSHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextFormat>
#include <QStringList>
#include <QVector>
#include <QHash>

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

class QTextDocument;

class JSHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    struct HighlightingRule
    {
#if QT_VERSION >= 0x050000
        QRegularExpression pattern;
#else
        QRegExp pattern;
#endif
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat doubleFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat objectFormat;

    JSHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) override;
};

#endif // JSHIGHLIGHTER_H
