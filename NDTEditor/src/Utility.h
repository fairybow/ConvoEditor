#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

#include <QChar>
#include <QColor>
#include <QList>
#include <QMargins>
#include <QSet>
#include <QString>

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
        // Try cached:
        static auto last_count = 0;
        static QColor last_start_color = Qt::red;
        static QList<QColor> cached{};

        if (count == last_count
            && startColor == last_start_color
            && !cached.isEmpty())
            return cached;

        // Otherwise:
        QList<QColor> result{};
        if (count < 1) return result;

        double h = startColor.hsvHueF();

        // Different levels for variety
        QList<double> saturations = { 0.9, 0.7, 0.8 };
        QList<double> values = { 0.9, 0.8, 0.95 };

        constexpr double conjugate = 1.0 / std::numbers::phi;

        for (auto i = 0; i < count; ++i)
        {
            // Select saturation and value based on position
            double s = saturations[i % saturations.size()];
            double v = values[i % values.size()];

            result << QColor::fromHsvF(h, s, v);

            // Advance hue by golden ratio conjugate and keep in range [0, 1)
            h += conjugate;
            h = fmod(h, 1.0);
        }

        return result;
    }

    inline void shiftPunct(QString& before, QString& after)
    {
        // Before: "This is before"
        // After: "... This is after."
        // Resolved: "This is before...", "This is after."
        if (after.isEmpty() || !after.at(0).isPunct()) return;

        // Find where punctuation ends and shift it
        auto punct_end = 0;
        while (punct_end < after.length()
            && after.at(punct_end).isPunct())
            punct_end++;

        before += after.left(punct_end);
        after.remove(0, punct_end);

        // Trim leading whitespace from after
        auto space_end = 0;
        while (space_end < after.length()
            && after.at(space_end).isSpace())
            space_end++;

        after.remove(0, space_end);
    }
}
