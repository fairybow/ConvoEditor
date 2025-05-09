#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QList>
#include <QPlainTextEdit>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

/// There may be an issue. Let's say We have many speakers, and we have Speaker
/// 10. We want to change it to Speaker 20 or something like that. When we
/// delete the 0, we will have double Speaker 1 items. This may be a problem.
/// Then we will also be updating Speaker 1, once we delete the 1, we will edit
/// both former Speaker 10 and Speaker 1
/// 
/// Instead, we could just make the boxes non-editable, then add buttons for
/// opening a field to edit (rename and add). We could disallow the edit/add if
/// the role name is the same as an existing role.

class ElementBlock : public QWidget
{
    Q_OBJECT

public:
    explicit ElementBlock(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        roleSelector_->setEditable(true);
        speechEdit_->setAcceptDrops(false);

        mainLayout_ = new QVBoxLayout(this);
        //mainLayout_->setContentsMargins(0, 0, 0, 0);

        // Set up layouts
        topLayout_ = new QHBoxLayout;
        topLayout_->addWidget(roleSelector_, 1);
        topLayout_->addWidget(eotSelector_, 0);

        mainLayout_->addLayout(topLayout_, 0);
        mainLayout_->addWidget(speechEdit_, 0);
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
    void roleChanged(const QString& from, const QString& to);

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechEdit_ = new QPlainTextEdit(this);
    QCheckBox* eotSelector_ = new QCheckBox(this);
};
