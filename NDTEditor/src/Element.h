#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class Element : public QWidget
{
    Q_OBJECT

public:
    explicit Element(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        // Set properties
        editRole_->setText("Edit");
        addRole_->setText("Add");
        roleSelector_->setEditable(false);
        speechEdit_->setAcceptDrops(false);

        roleSelector_->installEventFilter(this);
        speechEdit_->installEventFilter(this);

        // Set up layouts
        mainLayout_ = new QVBoxLayout(this);
        //mainLayout_->setContentsMargins(0, 0, 0, 0);
        topLayout_ = new QHBoxLayout;
        topLayout_->addWidget(editRole_, 0);
        topLayout_->addWidget(addRole_, 0);
        topLayout_->addWidget(roleSelector_, 1);
        topLayout_->addWidget(eotSelector_, 0);
        mainLayout_->addLayout(topLayout_, 0);
        mainLayout_->addWidget(speechEdit_, 0);

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
    }

    virtual ~Element() override { qDebug() << __FUNCTION__; }

    QString role() const { return roleSelector_->currentText(); }
    void setRole(const QString& role) { roleSelector_->setCurrentText(role); }
    QString speech() const { return speechEdit_->toPlainText(); }
    void setSpeech(const QString& speech) { speechEdit_->setPlainText(speech); }
    bool eot() const { return eotSelector_->isChecked(); }
    void setEot(bool eot) { eotSelector_->setChecked(eot); }

    void setRoleChoices(const QStringList& roles)
    {
        roleSelector_->clear();
        roleSelector_->addItems(roles);
    }

signals:
    void roleChangeRequested(const QString& from, const QString& to);
    void roleAddRequested(const QString& role);

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
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;
    QToolButton* editRole_ = new QToolButton(this);
    QToolButton* addRole_ = new QToolButton(this);

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechEdit_ = new QPlainTextEdit(this);
    QCheckBox* eotSelector_ = new QCheckBox(this);

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
};
