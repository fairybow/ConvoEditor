#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QPlainTextEdit>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>

class ElementBlock : public QWidget
{
    Q_OBJECT

public:
    explicit ElementBlock(QWidget* parent = nullptr)
        : QWidget(parent)
    {
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
        // Find or add?
    }

    void setSpeech(const QString& speech)
    {
        speechEdit_->setPlainText(speech);
    }

    void setEot(bool eot)
    {
        eotSelector_->setChecked(eot);
    }

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* topLayout_ = nullptr;

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechEdit_ = new QPlainTextEdit(this);
    QCheckBox* eotSelector_ = new QCheckBox(this);
};
