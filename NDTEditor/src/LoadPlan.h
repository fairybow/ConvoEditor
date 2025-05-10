#pragma once

#include <algorithm>

#include <QJsonValue>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

#include "Keys.h"

class LoadPlan
{
public:
    struct Item
    {
        QString role{};
        QString speech{};
        bool eot = true;
    };

    bool isNull() const noexcept
    {
        return items_.isEmpty();
    }

    void add(const QJsonValue& jsonValue)
    {
        auto obj = jsonValue.toObject();
        auto role = obj[Keys::ROLE].toString();

        Item item
        {
            role,
            obj[Keys::SPEECH].toString(),
            obj[Keys::EOT].toBool()
        };

        items_ << item;
        roles_ << role; // Will not add duplicates
    }

    const QList<Item>& items() const noexcept
    {
        return items_;
    }

    QStringList roles() const
    {
        QStringList r{ roles_.begin(), roles_.end() };
        std::sort(r.begin(), r.end());
        return r;
    }

private:
    QList<Item> items_{};
    QSet<QString> roles_{};
};
