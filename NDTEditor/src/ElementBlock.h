#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QList>
#include <QPlainTextEdit>
#include <QSet>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class ElementBlock : public QWidget
{
    Q_OBJECT

public:
    explicit ElementBlock(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        editRole_->setText("Edit");
        addRole_->setText("Add"); // icons later
        roleSelector_->setEditable(false);
        speechEdit_->setAcceptDrops(false);

        mainLayout_ = new QVBoxLayout(this);
        //mainLayout_->setContentsMargins(0, 0, 0, 0);

        // Set up layouts
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
            &ElementBlock::onEditRoleClicked_
        );

        connect
        (
            addRole_,
            &QToolButton::clicked,
            this,
            &ElementBlock::onAddRoleClicked_
        );
    }

    virtual ~ElementBlock() override
    {
        qDebug() << __FUNCTION__;
    }

    void setRole(const QString& role)
    {
        roleSelector_->setCurrentText(role);
    }

    void setSpeech(const QString& speech)
    {
        speechEdit_->setPlainText(speech);
    }

    void setEot(bool eot)
    {
        eotSelector_->setChecked(eot);
    }

    QString role() const
    {
        return roleSelector_->currentText();
    }

    void setRoles(const QList<QString>& roles)
    {
        roleSelector_->clear();

        for (auto& role : roles)
            roleSelector_->addItem(role);
    }

signals:
    // Current role changed, to pass to view, which will pass it to present (main window), which will edit the model lol
    void roleChanged(const QString& from, const QString& to);
    void roleAdded(const QString& role);

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechEdit_ = new QPlainTextEdit(this);
    QCheckBox* eotSelector_ = new QCheckBox(this);

    QToolButton* editRole_ = new QToolButton(this);
    QToolButton* addRole_ = new QToolButton(this);

    QSet<QString> itemTexts() const
    {
        QSet<QString> texts{};

        for (auto i = 0; i < roleSelector_->count(); ++i)
            texts << roleSelector_->itemText(i);

        return texts;
    }

private slots:
    void onEditRoleClicked_()
    {
        auto now = QInputDialog::getText
        (
            this,
            qApp->applicationName(),
            "Edit Role",
            QLineEdit::Normal,
            roleSelector_->currentText()
        ).trimmed();

        if (now.isEmpty() || itemTexts().contains(now)) return;

        emit roleChanged(roleSelector_->currentText(), now);
    }

    void onAddRoleClicked_()
    {
        auto role = QInputDialog::getText
        (
            this,
            qApp->applicationName(),
            "Add Role",
            QLineEdit::Normal
        ).trimmed();

        if (role.isEmpty() || itemTexts().contains(role)) return;

        emit roleAdded(role);
    }
};
