/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2017 Syping
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

#include "JSHighlighter.h"
#include <QRegExp>

JSHighlighter::JSHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QBrush keywordBrush(QColor::fromRgb(66, 137, 244));
    keywordFormat.setForeground(keywordBrush);
    keywordFormat.setFontItalic(true);
    QStringList keywordPatterns;
    keywordPatterns << "\\btrue\\b" << "\\bfalse\\b";
    for (QString pattern : keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    QBrush doubleBrush(QColor::fromRgb(66, 137, 244));
    doubleFormat.setForeground(doubleBrush);
    rule.pattern = QRegExp("[+-]?\\d*\\.?\\d+");
    rule.format = doubleFormat;
    highlightingRules.append(rule);

    QBrush quotationBrush(QColor::fromRgb(66, 244, 104));
    quotationFormat.setForeground(quotationBrush);
    rule.pattern = QRegExp("\"[^\"]*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    QBrush objectBrush(QColor::fromRgb(255, 80, 80));
    objectFormat.setForeground(objectBrush);
    rule.pattern = QRegExp("\"[^\"]*\"(?=:)");
    rule.format = objectFormat;
    highlightingRules.append(rule);
}

void JSHighlighter::highlightBlock(const QString &text)
{
    for (HighlightingRule rule : highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}
