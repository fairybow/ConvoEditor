#pragma once

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValueRef>
#include <QList>
#include <QObject>
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
        auto docuemnt = QJsonDocument::fromJson(string.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error:" << error.errorString();
            return false;
        }

        // No errors, so loading will proceed
        document_ = docuemnt;
        parse_();
        emit loaded();
        
        return true;
    }

    QJsonDocument document() const
    {
        return document_;
    }

    const QList<Element>& elements() const
    {
        return elements_;
    }

signals:
    void loaded();
    // element adjusted/removed/added

private:
    static constexpr auto ROLE_KEY = "Role";
    static constexpr auto SPEECH_KEY = "Content";
    static constexpr auto EOT_KEY = "EndOfTurn";
    QJsonDocument document_{};
    QList<Element> elements_{};

    void parse_()
    {
        elements_.clear();

        // This is our expected structure:
        //{
        //    "results": [
        //        {
        //            "Role": "Speaker 0",
        //            "Content" : "Hello. How are you?",
        //            "EndOfTurn" : true
        //        },
        //        {
        //            "Role": "Speaker 1",
        //            "Content" : "Hi, um",
        //            "EndOfTurn" : false
        //        }
        //    ]
        //}

        if (document_.isObject())
        {
            auto root = document_.object();
            auto array = root["results"].toArray();

            for (const auto& value : array)
            {
                if (!value.isObject()) continue;
                elements_ << toElement_(value);
            }
        }
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
