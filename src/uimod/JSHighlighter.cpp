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

#include "JSHighlighter.h"

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
#if QT_VERSION >= 0x050000
        rule.pattern = QRegularExpression(pattern);
#else
        rule.pattern = QRegExp(pattern);
#endif
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    QBrush doubleBrush(QColor::fromRgb(66, 137, 244));
    doubleFormat.setForeground(doubleBrush);
#if QT_VERSION >= 0x050000
    rule.pattern = QRegularExpression("[+-]?\\d*\\.?\\d+");
#else
    rule.pattern = QRegExp("[+-]?\\d*\\.?\\d+");
#endif
    rule.format = doubleFormat;
    highlightingRules.append(rule);

    QBrush quotationBrush(QColor::fromRgb(66, 244, 104));
    quotationFormat.setForeground(quotationBrush);
#if QT_VERSION >= 0x050000
    rule.pattern = QRegularExpression("\"[^\"]*\"");
#else
    rule.pattern = QRegExp("\"[^\"]*\"");
#endif
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    QBrush objectBrush(QColor::fromRgb(255, 80, 80));
    objectFormat.setForeground(objectBrush);
#if QT_VERSION >= 0x050000
    rule.pattern = QRegularExpression("\"[^\"]*\"(?=:)");
#else
    rule.pattern = QRegExp("\"[^\"]*\"(?=:)");
#endif
    rule.format = objectFormat;
    highlightingRules.append(rule);
}

void JSHighlighter::highlightBlock(const QString &text)
{
#if QT_VERSION >= 0x050000
    for (const HighlightingRule &rule : qAsConst(highlightingRules))
#else
    for (const HighlightingRule &rule : highlightingRules)
#endif
    {
#if QT_VERSION >= 0x050000
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
#else
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
#endif
    }
    setCurrentBlockState(0);
}
