#pragma once

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValueRef>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTextStream>

class JsonModel : public QObject
{
    Q_OBJECT

public:
    struct Element
    {
        QString role{};
        QString speech{};
        bool eot = true;
    };

    explicit JsonModel(QObject* parent = nullptr)
        : QObject(parent)
    {
        QJsonObject root_obj{};
        root_obj["elements"] = QJsonArray{};
        document_.setObject(root_obj);
    }

    bool load(const QString& filePath)
    {
        QFile file(filePath);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << "Failed to open file:" << filePath;
            return false;
        }

        QTextStream in(&file);
        auto string = in.readAll();
        file.close();

        QJsonParseError error{};
        auto document = QJsonDocument::fromJson(string.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error:" << error.errorString();
            return false;
        }

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
            //qWarning() << "JSON format is incorrect. Expected:" << EXPECTED;
            qWarning() << "JSON format is incorrect.";
            document_ = old_document;
            elements_ = old_elements;
            roles_ = old_roles;
            return false;
        }
    }

    QJsonDocument document() const
    {
        return document_;
    }

    const QList<Element>& elements() const
    {
        return elements_;
    }

    const QSet<QString>& roles() const
    {
        return roles_;
    }

    void replaceRole(const QString& from, const QString& to)
    {
        // Update JSON
        auto root = document_.object();
        auto array = root["results"].toArray();

        // We need to use an index-based loop because we're modifying elements
        for (auto i = 0; i < array.size(); ++i)
        {
            if (!array[i].isObject()) continue;

            // Get object, modify if needed, then replace in array
            auto obj = array[i].toObject();

            if (obj[ROLE_KEY].toString() == from)
            {
                obj[ROLE_KEY] = to;
                array[i] = obj;
            }
        }

        // Update the root object with our modified array
        root["results"] = array;
        document_.setObject(root);

        // Update elements
        for (auto& element : elements_)
            if (element.role == from)
                element.role = to;

        // Update roles
        if (roles_.contains(from))
        {
            roles_.remove(from);
            roles_.insert(to);

            // If we want to emit a signal about roles changing. Unsure if needed...
            // emit rolesChanged();
        }
    }

signals:
    void loaded();
    //void rolesChanged();
    // element at index adjusted/removed/added (impl later)

private:
    // We want to preserve JSON structure, and it might be easiest to just
    // ensure we make all edits to data synchronized
    static constexpr auto ROLE_KEY = "Role";
    static constexpr auto SPEECH_KEY = "Content";
    static constexpr auto EOT_KEY = "EndOfTurn";
    QJsonDocument document_{};
    QList<Element> elements_{};
    QSet<QString> roles_{};

/*    static constexpr auto EXPECTED = R"(
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
}";
)";*/

    bool parse_(const QJsonDocument& document)
    {
        document_ = document;
        elements_.clear();
        roles_.clear();

        if (document_.isObject())
        {
            auto root = document_.object();
            auto array = root["results"].toArray();

            for (const auto& value : array)
            {
                if (!value.isObject()) continue;
                auto element = toElement_(value);
                elements_ << element;
                roles_ << element.role;
            }

            return true;
        }

        return false;

        // Don't need a rolesChanged emission, since this is only called on
        // model load, implying fresh view with no need to receive this signal
        // yet
    }

    Element toElement_(const QJsonValueRef& value)
    {
        Element element{};
        auto obj = value.toObject();

        element.role = obj[ROLE_KEY].toString();
        element.speech = obj[SPEECH_KEY].toString();
        element.eot = obj[EOT_KEY].toBool();

        return element;
    }
};
