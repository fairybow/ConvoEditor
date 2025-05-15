#pragma once

#include <QJsonDocument>
#include <QString>

namespace Io
{
    QJsonDocument read(const QString& path);
    bool write(const QJsonDocument& jsonDocument, const QString& path);
}
