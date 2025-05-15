#include "InsertButton.h"

#include <QDebug>
#include <QToolButton>
#include <QWidget>

InsertButton::InsertButton(int position, QWidget* parent)
    : QToolButton(parent)
    , position_(position)
{
    connect
    (
        this,
        &QToolButton::clicked,
        this,
        [&] { emit insertRequested(position_); }
    );
}

InsertButton::~InsertButton()
{
    qDebug() << __FUNCTION__;
}
