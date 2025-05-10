#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QStringList>
#include <QPlainTextEdit>
#include <QString>
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

        // connections
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
        //...
    }

signals:
    void roleChangeRequested(const QString& from, const QString& to);
    void roleAddRequested(const QString& role);

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;
    QToolButton* editRole_ = new QToolButton(this);
    QToolButton* addRole_ = new QToolButton(this);

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechEdit_ = new QPlainTextEdit(this);
    QCheckBox* eotSelector_ = new QCheckBox(this);

private slots:
    //...
};
