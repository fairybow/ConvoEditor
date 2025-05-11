#pragma once

#pragma once

#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QWidget>

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

        auto layout = new QHBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        layout->addWidget(checkBox_);
    }

    virtual ~EotCheck() override { qDebug() << __FUNCTION__; }

    bool isChecked() const { return checkBox_->isChecked(); }
    void setChecked(bool checked) { checkBox_->setChecked(checked); }

private:
    QCheckBox* checkBox_ = new QCheckBox(this);
};
