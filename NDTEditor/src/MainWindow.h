#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMainWindow>
#include <QObject>
#include <QToolButton>
#include <QWidget>

#include "View.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

private:
    View* view_ = new View(this);
    QToolButton* save_ = new QToolButton(this);
    QToolButton* autoEot_ = new QToolButton(this);
    QToolButton* split_ = new QToolButton(this);

    void initialize_();
};
