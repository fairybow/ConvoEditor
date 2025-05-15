#pragma once

#include <QCheckBox>
#include <QDebug>
#include <QObject>
#include <QWidget>

class EotCheck : public QWidget
{
    Q_OBJECT

public:
    explicit EotCheck(QWidget* parent = nullptr);
    virtual ~EotCheck() override { qDebug() << __FUNCTION__; }

    bool isChecked() const { return checkBox_->isChecked(); }
    void setChecked(bool checked) { checkBox_->setChecked(checked); }

signals:
    void toggled(bool checked);

private:
    QCheckBox* checkBox_ = new QCheckBox(this);
};
