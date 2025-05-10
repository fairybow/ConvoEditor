#pragma once

#include <algorithm>

#include <QJsonValue>
#include <QStringList>
#include <QObject>
#include <QSet>
#include <QString>

#include "Keys.h"

class LoadPlan
{
public:
    bool isNull() const noexcept
    {
        return jsonValues_.isEmpty();
    }

    void add(const QJsonValue& jsonValue)
    {
        jsonValues_ << jsonValue;
        auto obj = jsonValue.toObject();
        roles_ << obj[Keys::ROLE].toString(); // Will not add duplicates
    }

    const QList<QJsonValue>& jsonValues() const noexcept
    {
        return jsonValues_;
    }

    QStringList roles() const
    {
        QStringList r{ roles_.begin(), roles_.end() };
        std::sort(r.begin(), r.end());
        return r;
    }

private:
    QList<QJsonValue> jsonValues_{};
    QSet<QString> roles_{};
};
