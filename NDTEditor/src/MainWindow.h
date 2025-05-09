#pragma once

#include <QMainWindow>

#include "JsonView.h"
#include "JsonModel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        setCentralWidget(jsonView_);
    }

private:
    JsonView* jsonView_ = new JsonView(this);
    JsonModel* jsonModel_ = new JsonModel(this);
};
