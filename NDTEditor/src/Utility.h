#pragma once

#include <algorithm>

#include <QList>
#include <QSet>

namespace Utility
{
    enum class Sort { No = 0, Yes };

    template <typename T>
    inline void sort(QList<T>& list)
    {
        std::sort(list.begin(), list.end());
    }

    template <typename T>
    inline QList<T> toList(const QSet<T>& set, Sort shouldSort = Sort::No)
    {
        QList<T> list{ set.begin(), set.end() };
        if (shouldSort == Sort::Yes) sort<T>(list);
        return list;
    }
}
