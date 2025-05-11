#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QObject>
#include <QPointer>

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

class CheckBoxCommand : public Command
{
    Q_OBJECT

public:
    CheckBoxCommand
    (
        QCheckBox* checkBox,
        bool old, bool now,
        QObject* parent = nullptr
    )
        : Command(parent)
        , checkBox_(checkBox)
        , old_(old), new_(now)
    {
        connect
        (
            checkBox_,
            &QObject::destroyed,
            this,
            [&] { emit invalidated(this); }
        );
    }

    virtual void execute() override
    {
        if (!checkBox_) return;

        auto block = checkBox_->blockSignals(true);
        checkBox_->setChecked(new_);
        checkBox_->blockSignals(block);
    }

    virtual void undo() override
    {
        if (!checkBox_) return;

        auto block = checkBox_->blockSignals(true);
        checkBox_->setChecked(old_);
        checkBox_->blockSignals(block);
    }

private:
    QPointer<QCheckBox> checkBox_;
    bool old_;
    bool new_;
};

// Would maybe need to use text and not index, since if an item is removed,
// index is no longer valid. If text is no longer valid, it won't be reassigned
// like index, so we can check it.
/*class ComboBoxSwitchCommand : public Command
{
    Q_OBJECT

public:
    ComboBoxSwitchCommand
    (
        QComboBox* comboBox,
        int old, int now,
        QObject* parent = nullptr
    )
        : Command(parent)
        , comboBox_(comboBox)
        , old_(old), new_(now)
    {
        connect
        (
            comboBox_,
            &QObject::destroyed,
            this,
            [&] { emit invalidated(this); }
        );
    }

    virtual void execute() override
    {
        if (!comboBox_) return;

        auto block = comboBox_->blockSignals(true);
        comboBox_->setCurrentIndex(new_);
        comboBox_->blockSignals(block);
    }

    virtual void undo() override
    {
        if (!comboBox_) return;

        auto block = comboBox_->blockSignals(true);
        comboBox_->setCurrentIndex(old_);
        comboBox_->blockSignals(block);
    }

private:
    QPointer<QComboBox> comboBox_;
    int old_;
    int new_;
};*/
