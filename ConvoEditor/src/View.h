#pragma once

#include <QJsonDocument>
#include <QLayoutItem>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QScrollArea>
#include <QString>
#include <QtTypes>
#include <QVBoxLayout>
#include <QWidget>

#include "AutoSizeTextEdit.h"
#include "Element.h"
#include "InsertButton.h"
#include "LoadPlan.h"

// Rename element_layout vars (and etc.) to content_layout or content

// Could do contentIndex overload for buttons and elements, returning correct
// position based on type
class View : public QWidget
{
    Q_OBJECT

public:
    explicit View(QWidget* parent = nullptr);
    virtual ~View() override;

    void autoEot()
    {
        if (currentEdit_)
            currentEdit_->simplify();

        for (auto& element : elements_)
            eotAdjust_(element);
    }

    bool load(const QString& path);
    bool save();
    void split(bool forceTripart = false, int tripartRole = -1);

signals:
    void documentLoaded();

private:
    QVBoxLayout* mainLayout_ = nullptr;
    QScrollArea* scrollArea_ = new QScrollArea(this);
    QWidget* contentContainer_ = new QWidget(scrollArea_);
    QVBoxLayout* contentLayout_ = nullptr;

    QList<Element*> elements_{};
    QList<InsertButton*> insertButtons_{};
    QList<QString> roleChoices_{};

    QString currentPath_{};
    QPointer<AutoSizeTextEdit> currentEdit_{};

    // Click is a press & release
    bool ignoreNextSpeechEditMClick_ = false;

    void updateInsertButtonPositions_(int startPosition)
    {
        // Update positions for all buttons from startPosition onward
        for (auto i = startPosition; i < insertButtons_.size(); ++i)
            insertButtons_[i]->setPosition(i);
    }

    void deleteItemWidget_(QLayoutItem* item)
    {
        if (auto widget = item->widget())
            delete widget;

        delete item;
    }

    void removeContent_(int layoutIndex)
    {
        if (auto item = contentLayout_->takeAt(layoutIndex))
            deleteItemWidget_(item);
    }

    void clearAllContent_()
    {
        QLayoutItem* item = nullptr;

        while ((item = contentLayout_->takeAt(0)) != nullptr)
            deleteItemWidget_(item);
    }

    void initialize_();
    void scrollToContent_(int contentIndex);
    void eotAdjust_(Element* element);
    LoadPlan parse_(const QJsonDocument& document);
    QJsonDocument compile_();
    void connectElement_(Element* element);
    void insertInsertButton_(int position);
    void populate_(const LoadPlan& plan);
    int insertElement_(int position, const LoadPlan::Item item = {});

private slots:
    void onElementRoleChangeRequested_(const QString& from, const QString& to);
    void onElementRoleAddRequested_(const QString& role);
    void onQAppFocusChanged_(QWidget* old, QWidget* now);
    void onElementDeleteRequested_(Element* element);
    void onSpeechEditMouseChorded_(int key, Qt::KeyboardModifiers modifiers);
};
