#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMainWindow>
#include <QMimeData>
#include <QStatusBar>
#include <QToolButton>
#include <QUrl>

#include "View.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        setAcceptDrops(true);
        setCentralWidget(view_);

        save_->setText("Save");
        autoEot_->setText("Auto EOT");
        split_->setText("Split");

        auto status_bar = new QStatusBar(this);
        status_bar->addWidget(save_);
        status_bar->addWidget(autoEot_);
        status_bar->addWidget(split_);
        setStatusBar(status_bar);

        connect
        (
            save_,
            &QToolButton::clicked,
            this,
            [&] {}
        );

        connect
        (
            autoEot_,
            &QToolButton::clicked,
            this,
            [&] {}
        );

        connect
        (
            split_,
            &QToolButton::clicked,
            this,
            [&] { view_->split(); }
        );
    }

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override
    {
        // Accept .json file paths
        if (event->mimeData()->hasUrls())
        {
           auto urls = event->mimeData()->urls();

           // We can only open one
           if (!urls.isEmpty())
           {
               QUrl url = urls.at(0);

               if (url.isLocalFile()
                   && QFileInfo(url.toLocalFile()).suffix().toLower() == "json")
               {
                   event->acceptProposedAction();
                   return;
               }
           }
        }

        event->ignore();
    }

    virtual void dropEvent(QDropEvent* event) override
    {
        // We know we have URLs because dragEnterEvent already verified this
        // We also know the first URL has .json extension
        QUrl url = event->mimeData()->urls().at(0);
        auto path = url.toLocalFile();

        if (view_->load(path))
        {
            setWindowTitle(QFileInfo(path).fileName());
            event->acceptProposedAction();
            activateWindow();

            return;
        }

        // If we get here, loading failed
        event->ignore();
    }

private:
    View* view_ = new View(this);
    QToolButton* save_ = new QToolButton(this);
    QToolButton* autoEot_ = new QToolButton(this);
    QToolButton* split_ = new QToolButton(this);
};
