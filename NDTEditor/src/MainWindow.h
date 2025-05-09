#pragma once

#include <QMainWindow>

#include "JsonView.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        setCentralWidget(jsonView_);
    }

private:
    JsonView* jsonView_ = new JsonView(this);
    // Model (later)
};
