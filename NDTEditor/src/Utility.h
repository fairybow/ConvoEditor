#pragma once

#include <algorithm>

#include <QColor>
#include <QList>
#include <QMargins>
#include <QSet>
#include <QString>
#include <QWidget>

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

    template <typename ParentWidgetT>
    inline ParentWidgetT* findParent(QWidget* widget)
    {
        ParentWidgetT* parent = nullptr;

        for (auto w = widget; w; w = w->parentWidget())
        {
            if (auto next = qobject_cast<ParentWidgetT*>(w))
            {
                parent = next;
                break;
            }
        }

        return parent;
    }

    QList<QColor> phiColors(int count, const QColor& startColor = Qt::red);
    void shiftPunct(QString& before, QString& after);
    bool wouldBreakWord(const QString& text, int splitPosition);
    void applyBreakIndicators(QString& beginning, QString& end, const char* indicator = "--");
}
