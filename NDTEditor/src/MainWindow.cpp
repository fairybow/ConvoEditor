#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMainWindow>
#include <QMimeData>
#include <QStatusBar>
#include <QString>
#include <QToolButton>
#include <QUrl>
#include <QWidget>

#include "MainWindow.h"
#include "View.h"

/// For testing:
#include <QDir>
#include <QStandardPaths>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    initialize_();

    /// For testing
    auto path = QDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).filePath("test.json");
    QTimer::singleShot(1000, this, [=] { view_->load(path); });
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
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

void MainWindow::dropEvent(QDropEvent* event)
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

void MainWindow::initialize_()
{
    setGeometry(100, 100, 400, 400);
    setAcceptDrops(true);
    setCentralWidget(view_);

    save_->setText("Save");
    autoEot_->setText("Auto EOT");
    split_->setText("Split");
    //undo_->setText("Undo");
    //redo_->setText("Redo");

    save_->setEnabled(false);
    autoEot_->setEnabled(false);
    split_->setEnabled(false);
    //undo_->setEnabled(false);
    //redo_->setEnabled(false);

    auto status_bar = new QStatusBar(this);
    status_bar->addWidget(save_);
    status_bar->addWidget(autoEot_);
    status_bar->addWidget(split_);
    //status_bar->addWidget(undo_);
    //status_bar->addWidget(redo_);
    setStatusBar(status_bar);

    connect
    (
        save_,
        &QToolButton::clicked,
        this,
        [&] { view_->save(); }
    );

    connect
    (
        autoEot_,
        &QToolButton::clicked,
        this,
        [&] { view_->autoEot(); }
    );

    connect
    (
        split_,
        &QToolButton::clicked,
        this,
        [&] { view_->split(); }
    );

    connect
    (
        view_,
        &View::documentLoaded,
        this,
        [&]
        {
            // Later, use a "modificationChanged" signal for save button
            // enabled
            save_->setEnabled(true);
            autoEot_->setEnabled(true);
            split_->setEnabled(true);
        }
    );
}
