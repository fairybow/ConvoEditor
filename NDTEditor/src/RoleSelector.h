#pragma once

#include <QComboBox>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionComboBox>
#include <QStylePainter>

class RoleSelector : public QComboBox
{
    Q_OBJECT

public:
    explicit RoleSelector(QWidget* parent = nullptr)
        : QComboBox(parent)
    {
    }

protected:
    virtual void paintEvent(QPaintEvent* event) override
    {
        QStylePainter painter(this);
        QStyleOptionComboBox option{};
        initStyleOption(&option);

        // Draw the combobox frame, button, etc.
        painter.drawComplexControl(QStyle::CC_ComboBox, option);

        auto index = currentIndex();
        auto display_text = currentText();
        constexpr auto format = "(%1) %2";

        if (index >= 0)
        {
            display_text = QString(format)
                .arg(index + 1)
                .arg(display_text);
        }

        // Draw label (selected text). Original model text is unchanged
        option.currentText = display_text;
        painter.drawControl(QStyle::CE_ComboBoxLabel, option);
    }
};
