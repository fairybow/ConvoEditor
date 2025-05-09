#pragma once

#include <QList>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "ElementBlock.h"

class JsonView : public QWidget
{
    Q_OBJECT

public:
    JsonView(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        mainLayout_ = new QVBoxLayout(this);
        //mainLayout_->setContentsMargins(0, 0, 0, 0);

        // Scroll area setup
        scrollArea_->setWidgetResizable(true);
        scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        // Content widget that holds all blocks
        scrollAreaLayout_ = new QVBoxLayout(scrollAreaContainer_);
        scrollAreaLayout_->setAlignment(Qt::AlignTop);
        scrollAreaLayout_->setSpacing(5);
        scrollArea_->setWidget(scrollAreaContainer_);

        // Set up main layout
        mainLayout_->addWidget(scrollArea_);

        /// Testing:

        scrollAreaLayout_->addWidget(new ElementBlock(this));
        scrollAreaLayout_->addWidget(new ElementBlock(this));
        scrollAreaLayout_->addWidget(new ElementBlock(this));
        scrollAreaLayout_->addWidget(new ElementBlock(this));
    }

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QVBoxLayout* scrollAreaLayout_ = nullptr;
    
    QScrollArea* scrollArea_ = new QScrollArea(this);
    QWidget* scrollAreaContainer_ = new QWidget(scrollArea_);
    
    QList<ElementBlock*> elementBlocks_{};
};
