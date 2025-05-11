#pragma once

#include <QComboBox>
#include <QObject>
#include <QPointer>

#include "EotCheck.h"

/// @warning These function more as a record (or a command-on-redo-only). We are
/// not intercepting user input, merely observing and recording for reuse
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

signals:
    void invalidated(Command*);
};

class EotCheckCommand : public Command
{
    Q_OBJECT

public:
    EotCheckCommand
    (
        EotCheck* eotCheck,
        bool old, bool now,
        QObject* parent = nullptr
    )
        : Command(parent)
        , eotCheck_(eotCheck)
        , old_(old), new_(now)
    {
        connect
        (
            eotCheck_,
            &QObject::destroyed,
            this,
            [&] { emit invalidated(this); }
        );
    }

    virtual void execute() override
    {
        if (!eotCheck_) return;

        auto block = eotCheck_->blockSignals(true);
        eotCheck_->setChecked(new_);
        eotCheck_->blockSignals(block);
    }

    virtual void undo() override
    {
        if (!eotCheck_) return;

        auto block = eotCheck_->blockSignals(true);
        eotCheck_->setChecked(old_);
        eotCheck_->blockSignals(block);
    }

private:
    QPointer<EotCheck> eotCheck_;
    bool old_;
    bool new_;
};
