#pragma once

#include <QObject>
#include <QPointer>

#include "Element.h"

/// @warning These function more as a record (or a command-on-redo-only). We are
/// not intercepting user input, merely observing and recording
class Command : public QObject
{
    Q_OBJECT

public:
    explicit Command(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class EotCheckCommand : public Command
{
    Q_OBJECT

public:
    EotCheckCommand
    (
        Element* element,
        bool old, bool now,
        QObject* parent = nullptr
    )
        : Command(parent)
        , element_(element)
        , old_(old), new_(now)
    {
    }

    virtual void execute() override
    {
        if (!element_) return;

        // It's dumb that I handle the EOT check box both directly and via its
        // parent element
        auto eot_check = element_->eotCheck();
        auto block = eot_check->blockSignals(true);
        element_->setEot(new_);
        eot_check->blockSignals(block);
    }

    virtual void undo() override
    {
        if (!element_) return;

        auto eot_check = element_->eotCheck();
        auto block = eot_check->blockSignals(true);
        element_->setEot(old_);
        eot_check->blockSignals(block);
    }

private:
    QPointer<Element> element_;
    bool old_;
    bool new_;
};
