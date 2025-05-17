#include <cmath>
#include <numbers>

#include <QChar>
#include <QColor>
#include <QList>
#include <QString>
#include <QtTypes>

#include "Utility.h"

namespace Utility
{
    // https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
    QList<QColor> phiColors(int count, const QColor& startColor)
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
            h = std::fmod(h, 1.0);
        }

        return result;
    }

    void shiftPunct(QString& before, QString& after)
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

    bool wouldBreakWord(const QString& text, int splitPosition)
    {
        if (splitPosition <= 0 || splitPosition >= text.length()) return false;

        auto size = static_cast<qsizetype>(splitPosition);

        // Get characters at split position
        auto c_before = text.at(size - 1);
        auto c_at = text.at(size);

        // Special handling for apostrophes
        auto is_apostrophe_before = c_before == '\'';
        auto is_apostrophe_at = c_at == '\'';

        // Look at surrounding context for apostrophes
        if (is_apostrophe_before || is_apostrophe_at)
        {
            // If split position is at an apostrophe, check surrounding
            // characters
            if (is_apostrophe_before)
            {
                // Check if apostrophe is part of a word (contraction or
                // possessive) Look at char before apostrophe and char after
                // split position
                if (splitPosition > 1 && splitPosition < text.length())
                {
                    auto c_before_apostrophe = text.at(size - 2);

                    return c_before_apostrophe.isLetterOrNumber()
                        && (c_at.isLetterOrNumber() || is_apostrophe_at);
                }
            }

            if (is_apostrophe_at)
            {
                // Check if apostrophe is beginning of a quoted word or
                // contraction Look at char before split and char after
                // apostrophe
                if (splitPosition < text.length() - 1)
                {
                    auto c_after_apostrophe = text.at(size + 1);

                    return c_before.isLetterOrNumber()
                        && c_after_apostrophe.isLetterOrNumber();
                }
            }
        }

        // Standard case: check if both sides are part of a word
        auto c_before_split_is_word_part = c_before.isLetterOrNumber()
            || (is_apostrophe_before
                && splitPosition > 1
                && text.at(size - 2).isLetterOrNumber());

        auto c_at_split_is_word_part = c_at.isLetterOrNumber()
            || (is_apostrophe_at
                && splitPosition < text.length() - 1
                && text.at(size + 1).isLetterOrNumber());

        // If both characters are part of words, then splitting would break a
        // word
        return c_before_split_is_word_part && c_at_split_is_word_part;
    }

    void applyBreakIndicators(QString& beginning, QString& end, const char* indicator)
    {
        if (beginning.isEmpty() || end.isEmpty()) return;
        beginning += indicator;
        end.prepend(indicator);
    }
}
