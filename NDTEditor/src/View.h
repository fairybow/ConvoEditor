#pragma once

#include <QList>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "Element.h"
#include "Io.h"

class View : public QWidget
{
    Q_OBJECT

public:
    explicit View(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        initialize_();
    }

    void load(const QString& path)
    {
        auto document = Io::read(path);
        if (document.isNull()) return;

        // No errors, so loading will proceed
        QJsonDocument old_document = document_;
        QList<Element> old_elements = elements_;
        QSet<QString> old_roles = roles_;

        if (parse_(document))
        {
            emit loaded();
            return true;
        }
        else
        {
            qWarning() << "JSON format is incorrect. Expected:" << EXPECTED;
            document_ = old_document;
            elements_ = old_elements;
            roles_ = old_roles;
            return false;
        }
    }

signals:
    //...

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QScrollArea* scrollArea_ = new QScrollArea(this);
    QWidget* elementsLayoutContainer_ = new QWidget(scrollArea_);
    QVBoxLayout* elementsLayout_ = nullptr;

    // Not keeping JsonDocument. View is SSOT
    QList<Element*> elements_{};
    QList<QString> roles_{};
    // Undo/redo stack QList<Snapshot>

    void initialize_()
    {
        // Scroll area setup
        scrollArea_->setWidgetResizable(true);
        scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea_->setWidget(elementsLayoutContainer_);

        // Set up layouts
        mainLayout_ = new QVBoxLayout(this);
        //mainLayout_->setContentsMargins(0, 0, 0, 0);
        elementsLayout_ = new QVBoxLayout(elementsLayoutContainer_);
        elementsLayout_->setAlignment(Qt::AlignTop);
        elementsLayout_->setSpacing(5);
        mainLayout_->addWidget(scrollArea_);
    }

private slots:
    //...
};
