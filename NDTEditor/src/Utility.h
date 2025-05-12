#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

#include <QColor>
#include <QList>
#include <QMargins>
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

    template <typename LayoutT>
    LayoutT* newLayout(QMargins margins, int spacing, QWidget* parent = nullptr, Qt::Alignment alignment = {})
    {
        auto layout = new LayoutT(parent);
        layout->setContentsMargins(margins);
        layout->setSpacing(spacing);

        if (alignment != Qt::AlignmentFlag(0))
            layout->setAlignment(alignment);

        return layout;
    }

    template <typename LayoutT>
    LayoutT* zeroPaddedLayout(QWidget* parent = nullptr, Qt::Alignment alignment = {})
    {
        return newLayout<LayoutT>({}, 0, parent, alignment);
    }

    inline QList<QColor> phiColors(int count, const QColor& startColor = Qt::red)
    {
        QList<QColor> result{};
        if (count < 1) return result;

        double h = startColor.hsvHueF();

        // Different saturation and value levels for more variety
        QList<double> saturations = { 0.9, 0.7, 0.8 };
        QList<double> values = { 0.9, 0.8, 0.95 };

        constexpr double conjugate = 1.0 / std::numbers::phi;

        for (auto i = 0; i < count; ++i)
        {
            // Select saturation and value based on position
            double s = saturations[i % saturations.size()];
            double v = values[i % values.size()];

            // Add color
            result << QColor::fromHsvF(h, s, v);

            // Advance hue by golden ratio conjugate
            h += conjugate;

            //if (h >= 1.0)
                //h -= 1.0;

            // Keep h in the range [0, 1) using fmod
            h = fmod(h, 1.0);
        }

        return result;
    }
}
