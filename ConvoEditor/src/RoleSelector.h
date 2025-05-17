#pragma once

#include <QComboBox>
#include <QObject>
#include <QPaintEvent>
#include <QWidget>

class RoleSelector : public QComboBox
{
    Q_OBJECT

public:
    explicit RoleSelector(QWidget* parent = nullptr);
    virtual ~RoleSelector() override;

protected:
    virtual void paintEvent(QPaintEvent* event) override;
};
