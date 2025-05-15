#include <QHBoxLayout>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "AutoSizeTextEdit.h"
#include "Element.h"
#include "EotCheck.h"
#include "RoleSelector.h"
#include "Utility.h"

Element::Element(QWidget* parent)
    : QWidget(parent)
{
    initialize_();
}

void Element::initialize_()
{
    // Set properties
    //setAttribute(Qt::WA_StyledBackground, true);
    roleSelector_->setEditable(false);
    speechEdit_->setAcceptDrops(false);
    speechEdit_->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    //speechEdit_->setUndoRedoEnabled(false);
    visualCue_->setAutoFillBackground(true);

    editRole_->setText("Edit");
    addRole_->setText("Add");
    delete_->setText("x");

    //editRole_->setObjectName("Edit");
    //addRole_->setObjectName("Add");

    editRole_->setFixedHeight(25);
    addRole_->setFixedHeight(25);
    roleSelector_->setFixedHeight(25);
    eotCheck_->setFixedHeight(25);
    delete_->setFixedHeight(25);
    visualCue_->setFixedWidth(5);

    roleSelector_->installEventFilter(this);
    speechEdit_->installEventFilter(this);

    // Set up layouts
    mainLayout_ = Utility::zeroPaddedLayout<QHBoxLayout>(this);
    controlLayout_ = Utility::zeroPaddedLayout<QVBoxLayout>();
    topLayout_ = Utility::zeroPaddedLayout<QHBoxLayout>();
    bottomLayout_ = Utility::zeroPaddedLayout<QHBoxLayout>();

    topLayout_->addWidget(editRole_, 0);
    topLayout_->addWidget(addRole_, 0);
    topLayout_->addWidget(roleSelector_, 0);
    topLayout_->addWidget(eotCheck_, 0);

    bottomLayout_->addWidget(speechEdit_, 0);
    bottomLayout_->addWidget(delete_, 0, Qt::AlignTop);

    controlLayout_->addLayout(topLayout_, 0);
    controlLayout_->addLayout(bottomLayout_, 1);

    mainLayout_->addWidget(visualCue_, 0);
    mainLayout_->addSpacing(5);
    mainLayout_->addLayout(controlLayout_, 0);

    connect
    (
        editRole_,
        &QToolButton::clicked,
        this,
        &Element::onEditRoleClicked_
    );

    connect
    (
        addRole_,
        &QToolButton::clicked,
        this,
        &Element::onAddRoleClicked_
    );

    connect
    (
        roleSelector_,
        &RoleSelector::currentIndexChanged,
        this,
        &Element::onRoleSelectorIndexChanged_
    );

    connect
    (
        delete_,
        &QToolButton::clicked,
        this,
        [&] { emit deleteRequested(this); }
    );
}
