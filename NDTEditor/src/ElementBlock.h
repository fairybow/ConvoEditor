#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ElementBlock : public QWidget
{
    Q_OBJECT

public:
    explicit ElementBlock(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        speechText_->setAcceptDrops(false);

        mainLayout_ = new QVBoxLayout(this);
        //mainLayout_->setContentsMargins(0, 0, 0, 0);

        // Set up layouts
        contentLayout_ = new QHBoxLayout;
        contentLayout_->addWidget(speechText_, 1);
        contentLayout_->addWidget(eotSelector_, 0);

        mainLayout_->addWidget(roleSelector_, 0);
        mainLayout_->addLayout(contentLayout_, 1);
    }

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* contentLayout_ = nullptr;

    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechText_ = new QPlainTextEdit(this);
    QCheckBox* eotSelector_ = new QCheckBox(this);
};
