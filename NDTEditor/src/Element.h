#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "EotCheck.h"

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

    virtual void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        updateSpeechEditHeight_();
    }

private:
    static constexpr auto STYLE_SHEET = R"(
Element {
    border: none;
    padding: 0px;
    margin: 0px;

    background-color: red;
    border-radius: 10px;
}

QToolButton#Edit {
    background-color: goldenrod;
    border-top-left-radius: 8px;
}

QToolButton#Add {
    background-color: cadetblue;
    border-radius: 0px;
}

QComboBox {
    border: none;
    padding: 0px;
    margin: 0px;

    background-color: orange;
    border-radius: 0px;
    padding-left: 5px;
}

EotCheck {
    border: none;
    padding: 0px;
    margin: 0px;

    background-color: pink;
    border-top-right-radius: 8px;
}

QPlainTextEdit {
    border: none;
    padding: 0px;
    margin: 0px;

    background-color: skyblue;
    border-bottom-left-radius: 8px;
    border-bottom-right-radius: 8px;
}
)";

    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;
    QToolButton* editRole_ = new QToolButton(this);
    QToolButton* addRole_ = new QToolButton(this);

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechEdit_ = new QPlainTextEdit(this);
    EotCheck* eotCheck_ = new EotCheck(this);

    void initialize_()
    {
        // Set properties
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet(STYLE_SHEET);
        roleSelector_->setEditable(false);
        speechEdit_->setAcceptDrops(false);

        editRole_->setText("Edit");
        addRole_->setText("Add");

        editRole_->setObjectName("Edit");
        addRole_->setObjectName("Add");

        editRole_->setFixedHeight(25);
        addRole_->setFixedHeight(25);
        roleSelector_->setFixedHeight(25);
        eotCheck_->setFixedHeight(25);

        roleSelector_->installEventFilter(this);
        speechEdit_->installEventFilter(this);

        // Set up layouts
        mainLayout_ = new QVBoxLayout(this);
        mainLayout_->setContentsMargins(2, 2, 2, 2);
        mainLayout_->setSpacing(0);

        topLayout_ = new QHBoxLayout;
        topLayout_->setContentsMargins(0, 0, 0, 0);
        topLayout_->setSpacing(0);

        topLayout_->addWidget(editRole_, 0);
        topLayout_->addWidget(addRole_, 0);
        topLayout_->addWidget(roleSelector_, 0);
        topLayout_->addWidget(eotCheck_, 0);

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
            speechEdit_,
            &QPlainTextEdit::textChanged,
            this,
            [&] { updateSpeechEditHeight_(); }
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

    void updateSpeechEditHeight_()
    {
        // QTextDocument needs to know its width to calculate how many lines the
        // text will wrap into. Without setting the width, the document might
        // calculate height as if all text is on a single line
        auto doc = speechEdit_->document();
        doc->setTextWidth(speechEdit_->viewport()->width());

        auto height = doc->size().toSize().height()
            + speechEdit_->contentsMargins().top()
            + speechEdit_->contentsMargins().bottom();

        height = qBound(30, height, 200);
        speechEdit_->setFixedHeight(height);
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
