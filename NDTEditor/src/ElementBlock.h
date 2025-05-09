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
        speechTextView_->setAcceptDrops(false);

        mainLayout_ = new QVBoxLayout(this);
        //mainLayout_->setContentsMargins(0, 0, 0, 0);

        // Set up layouts
        contentLayout_ = new QHBoxLayout;
        contentLayout_->addWidget(speechTextView_, 1);
        contentLayout_->addWidget(eotSelector_, 0);

        mainLayout_->addWidget(roleSelector_, 0);
        mainLayout_->addLayout(contentLayout_, 1);
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
        speechTextView_->setPlainText(speech);
    }

    void setEot(bool eot)
    {
        eotSelector_->setChecked(eot);
    }

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QHBoxLayout* contentLayout_ = nullptr;

    // States
    QComboBox* roleSelector_ = new QComboBox(this);
    QPlainTextEdit* speechTextView_ = new QPlainTextEdit(this);
    QCheckBox* eotSelector_ = new QCheckBox(this);
};
