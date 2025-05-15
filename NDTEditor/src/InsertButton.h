#pragma once

#include <QObject>
#include <QToolButton>
#include <QWidget>

class InsertButton : public QToolButton
{
    Q_OBJECT

public:
    explicit InsertButton(int position, QWidget* parent = nullptr);
    virtual ~InsertButton() override;

    int position() const noexcept { return position_; }
    void setPosition(int position) { position_ = position; }

signals:
    void insertRequested(int position);

private:
    int position_;
};
