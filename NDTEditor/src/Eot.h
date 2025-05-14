#pragma once

#include <QChar>
#include <QRegularExpression>
#include <QString>

namespace Eot
{
    inline bool endsWithFiller(const QString& string)
    {
        QString last_word = string
            .split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)
            .last()
            .toLower();

        QString cleaned{};

        for (auto& c : last_word)
            if (!c.isPunct())
                cleaned.append(c);

        return cleaned == "um"
            || cleaned == "uh"
            || cleaned == "er"
            || cleaned == "erm"
            || cleaned == "hm"
            || cleaned == "hmm";
    };

    inline bool hasTerminalPunc(const QString& string)
    {
        return string.endsWith('.')
            || string.endsWith('!')
            || string.endsWith('?')
            || string.endsWith(".\"")
            || string.endsWith("!\"")
            || string.endsWith("?\"")
            || string.endsWith(".\'")
            || string.endsWith("!\'")
            || string.endsWith("?\'");
    };
}
