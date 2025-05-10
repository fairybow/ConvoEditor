#pragma once

#include <algorithm>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLayoutItem>
#include <QList>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "Element.h"
#include "Io.h"
#include "Keys.h"
#include "LoadPlan.h"

class View : public QWidget
{
    Q_OBJECT

public:
    explicit View(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        initialize_();
    }

    bool load(const QString& path)
    {
        auto document = Io::read(path);
        if (document.isNull()) return false;

        // No errors, so loading will proceed

        QList<Element*> old_elements = elements_;
        QList<QString> old_role_choices = roleChoices_;
        elements_.clear();
        roleChoices_.clear();

        auto plan = parse_(document);

        if (!plan.isNull())
        {
            clearElements_();
            populate_(plan);
            return true;
        }

        qWarning() << "JSON format is incorrect. Expected:" << EXPECTED;
        elements_ = old_elements;
        roleChoices_ = old_role_choices;
        return false;
    }

private:
    static constexpr auto EXPECTED = R"(
{
    "results": [
        {
            "Role": "Speaker 0",
            "Content" : "Hello. How are you?",
            "EndOfTurn" : true
        },
        {
            "Role": "Speaker 1",
            "Content" : "Hi, um",
            "EndOfTurn" : false
        }
    ]
}
)";

    QVBoxLayout* mainLayout_ = nullptr;
    QScrollArea* scrollArea_ = new QScrollArea(this);
    QWidget* elementsLayoutContainer_ = new QWidget(scrollArea_);

    // Not keeping JsonDocument. View is SSOT
    QVBoxLayout* elementsLayout_ = nullptr;
    QList<Element*> elements_{};
    QList<QString> roleChoices_{};
    // Later, undo/redo stack QList<Snapshot>

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

    LoadPlan parse_(const QJsonDocument& document)
    {
        LoadPlan plan{};

        if (document.isObject())
        {
            auto root = document.object();

            if (!root.contains(Keys::RESULTS_ARRAY))
                return plan;

            // Check if the value at the "results" key is an array
            if (!root[Keys::RESULTS_ARRAY].isArray())
                return plan;

            // Check for erroneous keys, etc?

            auto array = root["results"].toArray();

            for (const auto& value : array)
            {
                if (!value.isObject()) continue;
                plan.add(value);
            }
        }

        return plan;
    }

    void clearElements_()
    {
        QLayoutItem* item = nullptr;

        while ((item = elementsLayout_->takeAt(0)) != nullptr)
        {
            if (auto widget = item->widget())
                delete widget;

            delete item;
        }
    }

    void populate_(const LoadPlan& plan)
    {
        roleChoices_ = plan.roles();

        for (auto& item : plan.items())
        {
            auto element = new Element(elementsLayoutContainer_);
            elements_ << element;

            element->setRoleChoices(roleChoices_);

            element->setRole(item.role);
            element->setSpeech(item.speech);
            element->setEot(item.eot);

            elementsLayout_->addWidget(element);

            connect
            (
                element,
                &Element::roleChangeRequested,
                this,
                &View::onElementRoleChangeRequested_
            );

            connect
            (
                element,
                &Element::roleAddRequested,
                this,
                &View::onElementRoleAddRequested_
            );
        }
    }

    void sortRoleChoices_()
    {
        std::sort(roleChoices_.begin(), roleChoices_.end());
    }

private slots:
    void onElementRoleChangeRequested_(const QString& from, const QString& to)
    {
        roleChoices_.removeAll(from);
        roleChoices_ << to;
        sortRoleChoices_();

        for (auto i = 0; i < elements_.count(); ++i)
        {
            if (auto element = elements_.at(i))
            {
                // Recall current selection
                auto current = element->role();
                element->setRoleChoices(roleChoices_);
                element->setRole((current == from) ? to : current);
            }
        }
    }
    
    void onElementRoleAddRequested_(const QString& role)
    {
        roleChoices_ << role;
        sortRoleChoices_();

        for (auto i = 0; i < elements_.count(); ++i)
        {
            if (auto element = elements_.at(i))
            {
                // Recall current selection
                auto current = element->role();
                element->setRoleChoices(roleChoices_);
                element->setRole(current);
            }
        }
    }
};
