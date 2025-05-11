#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QString>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "AutoSizeTextEdit.h"
#include "EotCheck.h"
#include "Utility.h"

class Element : public QWidget
{
    Q_OBJECT

public:
    explicit Element(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        initialize_();
    }

    virtual ~Element() override { qDebug() << __FUNCTION__; }

    QString role() const { return roleSelector_->currentText(); }
    void setRole(const QString& role) { roleSelector_->setCurrentText(role); }
    QString speech() const { return speechEdit_->toPlainText(); }
    void setSpeech(const QString& speech) { speechEdit_->setPlainText(speech); }
    bool eot() const { return eotCheck_->isChecked(); }
    void setEot(bool eot) { eotCheck_->setChecked(eot); }
    AutoSizeTextEdit* speechEdit() const noexcept { return speechEdit_; }
    EotCheck* eotCheck() const noexcept { return eotCheck_; }

    void setRoleChoices(const QStringList& roles)
    {
        roleSelector_->clear();
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
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;
    QToolButton* editRole_ = new QToolButton(this);
    QToolButton* addRole_ = new QToolButton(this);
    QToolButton* delete_ = new QToolButton(this);

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    AutoSizeTextEdit* speechEdit_ = new AutoSizeTextEdit(this);
    EotCheck* eotCheck_ = new EotCheck(this);

    void initialize_()
    {
        // Set properties
        //setAttribute(Qt::WA_StyledBackground, true);
        roleSelector_->setEditable(false);
        speechEdit_->setAcceptDrops(false);
        speechEdit_->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

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

        roleSelector_->installEventFilter(this);
        speechEdit_->installEventFilter(this);

        // Set up layouts
        mainLayout_ = Utility::zeroPaddedLayout<QVBoxLayout>(this);
        topLayout_ = Utility::zeroPaddedLayout<QHBoxLayout>();

        topLayout_->addWidget(editRole_, 0);
        topLayout_->addWidget(addRole_, 0);
        topLayout_->addWidget(roleSelector_, 0);
        topLayout_->addWidget(eotCheck_, 0);
        topLayout_->addWidget(delete_, 0);

        mainLayout_->addLayout(topLayout_, 0);
        mainLayout_->addWidget(speechEdit_, 1);

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
            delete_,
            &QToolButton::clicked,
            this,
            [&] { emit deleteRequested(this); }
        );
    }

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
