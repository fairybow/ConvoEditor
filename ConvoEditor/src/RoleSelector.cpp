#include <QComboBox>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QString>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QStylePainter>
#include <QWidget>

#include "RoleSelector.h"

RoleSelector::RoleSelector(QWidget* parent)
    : QComboBox(parent)
{
}

RoleSelector::~RoleSelector()
{
    qDebug() << __FUNCTION__;
}

void RoleSelector::paintEvent(QPaintEvent* event)
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
