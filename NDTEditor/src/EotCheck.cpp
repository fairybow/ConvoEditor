#include <QCheckBox>
#include <QHBoxLayout>
#include <QWidget>

#include "EotCheck.h"
#include "Utility.h"

EotCheck::EotCheck(QWidget* parent)
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
