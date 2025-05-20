#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QWidget>

#include "Coco/Layout.h"

#include "EotCheck.h"

EotCheck::EotCheck(QWidget* parent)
    : QWidget(parent)
{
    //setAttribute(Qt::WA_StyledBackground, true);

    checkBox_->setLayoutDirection(Qt::RightToLeft);
    checkBox_->setText("End of turn:");

    auto layout = Coco::Layout::zeroPadded<QHBoxLayout>(this, Qt::AlignCenter);
    layout->addWidget(checkBox_);

    connect
    (
        checkBox_,
        &QCheckBox::toggled,
        this,
        [&](bool checked) { emit toggled(checked); }
    );
}

EotCheck::~EotCheck()
{
    qDebug() << __FUNCTION__;
}
