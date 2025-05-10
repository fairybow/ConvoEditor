#pragma once

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QString>
#include <QTextStream>

namespace Io
{
    QJsonDocument read(const QString& path)
    {
        QFile file(path);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << "Failed to open file:" << path;
            return {};
        }

        QTextStream in(&file);
        auto string = in.readAll();
        file.close();

        QJsonParseError err{};
        auto document = QJsonDocument::fromJson(string.toUtf8(), &err);

        if (err.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error:" << err.errorString();
            return {};
        }

        return document;
    }
}
