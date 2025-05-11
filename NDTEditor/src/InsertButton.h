#pragma once

#include <QDebug>
#include <QToolButton>
#include <QWidget>

class InsertButton : public QToolButton
{
    Q_OBJECT

public:
    explicit InsertButton(int position, QWidget* parent = nullptr)
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

    virtual ~InsertButton() override { qDebug() << __FUNCTION__; }

    int position() const noexcept { return position_; }
    void setPosition(int position) { position_ = position; }

signals:
    void insertRequested(int position);

private:
    int position_;
};
