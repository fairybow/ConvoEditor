#pragma once

#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QWidget>

#include "Utility.h"

class EotCheck : public QWidget
{
    Q_OBJECT

public:
    explicit EotCheck(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        //setAttribute(Qt::WA_StyledBackground, true);

        checkBox_->setLayoutDirection(Qt::RightToLeft);
        checkBox_->setText("End of turn:");

        auto layout = Utility::zeroPaddedLayout<QHBoxLayout>(this, Qt::AlignCenter);
        layout->addWidget(checkBox_);

        connect
        (
            checkBox_,
            &QCheckBox::toggled,
            this,
            [&](bool checked) { emit toggled(checked); }
        );
    }

    virtual ~EotCheck() override { qDebug() << __FUNCTION__; }

    bool isChecked() const { return checkBox_->isChecked(); }
    void setChecked(bool checked) { checkBox_->setChecked(checked); }

signals:
    void toggled(bool checked);

private:
    QCheckBox* checkBox_ = new QCheckBox(this);
};
