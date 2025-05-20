#pragma once

#include <QString>

namespace Utility
{
    void shiftPunct(QString& before, QString& after);
    bool wouldBreakWord(const QString& text, int splitPosition);
    void applyBreakIndicators(QString& beginning, QString& end, const char* indicator = "--");
}
