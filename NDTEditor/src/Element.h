#pragma once

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QList>
#include <QObject>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "AutoSizeTextEdit.h"
#include "EotCheck.h"
#include "RoleSelector.h"
#include "Utility.h"

// For role removal, we would want a pop-up that asks what to set all roles
// assigned to the removed role to. If we want to assign them to a new role, we
// can just redirect to edit role name

class Element : public QWidget
{
    Q_OBJECT

public:
    explicit Element(QWidget* parent = nullptr);
    virtual ~Element() override { qDebug() << __FUNCTION__; }

    QString role() const { return roleSelector_->currentText(); }
    void setRole(const QString& role) { roleSelector_->setCurrentText(role); }
    QString speech() const { return speechEdit_->toPlainText(); }
    void setSpeech(const QString& speech) { speechEdit_->setPlainText(speech); }
    bool eot() const { return eotCheck_->isChecked(); }
    void setEot(bool eot) { eotCheck_->setChecked(eot); }
    RoleSelector* roleSelector() const noexcept { return roleSelector_; }
    AutoSizeTextEdit* speechEdit() const noexcept { return speechEdit_; }
    EotCheck* eotCheck() const noexcept { return eotCheck_; }

    void setRoleChoices(const QStringList& roles)
    {
        roleSelector_->clear();
        visualCueColors_ = Utility::phiColors(roles.count());
        roleSelector_->addItems(roles);
    }

signals:
    void roleChangeRequested(const QString& from, const QString& to);
    void roleAddRequested(const QString& role);
    void deleteRequested(Element*);

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::Wheel)
        {
            // If the event is for our text edit or combo box, 
            // ignore it so it propagates to the parent scroll area
            if (watched == speechEdit_ || watched == roleSelector_)
            {
                event->ignore();
                return true; // We handled it by ignoring it
            }
        }

        // Let the base class handle other events
        return QWidget::eventFilter(watched, event);
    }

private:
    QHBoxLayout* mainLayout_ = nullptr;
    QVBoxLayout* controlLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;
    QHBoxLayout* bottomLayout_ = nullptr;
    QToolButton* editRole_ = new QToolButton(this);
    QToolButton* addRole_ = new QToolButton(this);
    QToolButton* delete_ = new QToolButton(this);
    QWidget* visualCue_ = new QWidget(this);
    QList<QColor> visualCueColors_{};

    // States
    RoleSelector* roleSelector_ = new RoleSelector(this);
    AutoSizeTextEdit* speechEdit_ = new AutoSizeTextEdit(this);
    EotCheck* eotCheck_ = new EotCheck(this);

    void initialize_();

    // Use findText, ding dong
    bool roleExists_(const QString& role) const
    {
        for (auto i = 0; i < roleSelector_->count(); ++i)
            if (role == roleSelector_->itemText(i))
                return true;

        return false;
    }

    QString getInput_(const QString& label, const QString& currentText = {})
    {
        return QInputDialog::getText
        (
            this,
            qApp->applicationName(),
            label,
            QLineEdit::Normal,
            currentText
        ).trimmed();
    }

private slots:
    void onEditRoleClicked_()
    {
        auto now = getInput_("Edit Role", roleSelector_->currentText());
        if (now.isEmpty() || roleExists_(now)) return;

        emit roleChangeRequested(roleSelector_->currentText(), now);
    }

    void onAddRoleClicked_()
    {
        auto role = getInput_("Add Role"); // Current text could be speaker +1
        if (role.isEmpty() || roleExists_(role)) return;

        emit roleAddRequested(role);
    }

    void onRoleSelectorIndexChanged_(int index)
    {
        if (index < 0) return;

        QPalette palette = visualCue_->palette();
        palette.setColor(QPalette::Window, visualCueColors_.at(index));
        visualCue_->setPalette(palette);
    }
};
