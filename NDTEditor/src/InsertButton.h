#pragma once

#include <QDebug>
#include <QHBoxLayout>
#include <QToolButton>
#include <QWidget>

class InsertButton : public QWidget
{
    Q_OBJECT

public:
    explicit InsertButton(int position, QWidget* parent = nullptr)
        : QWidget(parent)
        , position_(position)
    {
        auto layout = new QHBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 5, 0, 5);

        button_->setText("+");
        button_->setFixedSize(30, 30);

        layout->addWidget(button_);

        connect
        (
            button_,
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
    QToolButton* button_ = new QToolButton(this);
};
