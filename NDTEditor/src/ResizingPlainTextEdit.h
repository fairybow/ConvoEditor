#pragma once

#include <QDebug>
#include <QPlainTextEdit>

class ResizingPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ResizingPlainTextEdit(QWidget* parent = nullptr)
        : QPlainTextEdit(parent)
    {
        setAcceptDrops(false);
    }

    virtual ~ResizingPlainTextEdit() override { qDebug() << __FUNCTION__; }
};
