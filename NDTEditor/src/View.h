#pragma once

#include <QApplication>
#include <QChar>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLayoutItem>
#include <QList>
#include <QPointer>
#include <QScrollArea>
#include <QScrollBar>
#include <QString>
#include <QStringList>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QVBoxLayout>
#include <QWidget>

#include "AutoSizeTextEdit.h"
//#include "Command.h"
//#include "CommandStack.h"
#include "Element.h"
#include "InsertButton.h"
#include "Io.h"
#include "Keys.h"
#include "LoadPlan.h"
#include "Utility.h"

class View : public QWidget
{
    Q_OBJECT

public:
    explicit View(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        initialize_();
    }

    /*CommandStack* commandStack() const noexcept
    {
        return commandStack_;
    }*/

    bool load(const QString& path)
    {
        auto document = Io::read(path);
        if (document.isNull()) return false;

        // No errors, so loading will proceed

        QList<Element*> old_elements = elements_;
        QList<InsertButton*> old_insert_buttons = insertButtons_;
        QList<QString> old_role_choices = roleChoices_;
        elements_.clear(); // move to clear all widgets?
        insertButtons_.clear(); // move to clear all widgets?
        roleChoices_.clear(); // Also, can't we just not clear these till after plan isn't null lol?

        auto plan = parse_(document);

        if (!plan.isNull())
        {
            currentPath_ = path;
            clearAllWidgets_();
            populate_(plan);
            scrollArea_->verticalScrollBar()->setValue(0);
            emit documentLoaded();
            return true;
        }

        qWarning() << "JSON format is incorrect. Expected:" << EXPECTED;
        elements_ = old_elements;
        insertButtons_ = old_insert_buttons;
        roleChoices_ = old_role_choices;
        return false;
    }

    bool save()
    {
        if (currentPath_.isEmpty()) return false;

        if (currentEdit_)
            currentEdit_->trim();

        auto document = compile_();
        if (document.isNull()) return false;
        return Io::write(document, currentPath_);
    }

    void split(bool forceTripart = false, int tripartRole = -1)
    {
        // Splits text elements at cursor position in three ways:
        // 1. Bipart (2-way): No selection -> splits at cursor into before/after
        // 2. Empty Tripart: No selection but forceTripart-> adds empty middle
        //    element
        // 3. Selection Tripart: With selection -> places selected text in
        //    middle
        // 
        // Triggered by: right rocker or MMB click. Making a mouse chord (MMB +
        // n) will force tripart regardless of selection and assign n role to
        // the middle element (useful for quickly splitting as an interruption)
        // 
        // Automatically adjusts EOT based on punctuation.
        if (!currentEdit_) return;

        // Find the parent Element by traversing up the widget hierarchy
        Element* initial_element = nullptr;

        for (auto widget = currentEdit_->parentWidget();
            widget;
            widget = widget->parentWidget())
        {
            if (auto next = qobject_cast<Element*>(widget))
            {
                initial_element = next;
                break;
            }
        }

        if (!initial_element) return;

        auto index = elements_.indexOf(initial_element);
        if (index < 0) return;

        auto cursor = currentEdit_->textCursor();
        auto position = cursor.position();
        auto has_selection = cursor.hasSelection();
        auto text = currentEdit_->toPlainText();

        QString before_text{};

        if (!has_selection && !forceTripart)
        {
            // 1. Break into 2

            // Validate we have text on both sides of cursor
            if (position <= 0 || position >= text.length()) return;

            // Split the text
            before_text = text.left(position).trimmed();
            auto after_text = text.mid(position).trimmed();
            if (before_text.isEmpty() || after_text.isEmpty()) return;

            // Set up the new element
            LoadPlan::Item item
            {
                initial_element->role(),
                after_text,
                initial_element->eot()
            };

            insertElement_((index + 1), item);
        }
        else // has_selection || forceTripart
        {
            constexpr auto get_tripart_role =
                [](int role, const QStringList& roles, const QString& fallback) noexcept
                {
                    return (role > -1)
                        ? roles.at(role)
                        : fallback;
                };

            constexpr auto tripart_insert =
                [](View* v, int insertIndex, const LoadPlan::Item& middle, const LoadPlan::Item& after) noexcept
                {
                    // Insert the elements: after first, then middle (which puts
                    // middle between initial and after)
                    v->insertElement_(insertIndex, after);
                    auto middle_index = v->insertElement_(insertIndex, middle);
                    return middle_index;
                };

            if (!has_selection)
            {
                // 2. Break into 3 (with empty middle element)

                // Validate we have text on both sides of cursor
                if (position <= 0 || position >= text.length()) return;

                // Split the text
                before_text = text.left(position).trimmed();
                auto after_text = text.mid(position).trimmed();
                if (before_text.isEmpty() || after_text.isEmpty()) return;

                auto initial_role = initial_element->role();

                // Set up the new elements
                LoadPlan::Item middle_item
                {
                    get_tripart_role(tripartRole, roleChoices_, initial_role),
                    {}, false
                };

                LoadPlan::Item after_item
                {
                    initial_role,
                    after_text,
                    initial_element->eot()
                };

                tripart_insert
                (
                    this,
                    index + 1,
                    middle_item,
                    after_item
                );
            }
            else // has_selection
            {
                // 3. Break into 3 with selection in the middle element

                auto selection_start = cursor.selectionStart();
                auto selection_end = cursor.selectionEnd();

                // Validate we have text on both sides of the selection
                if (selection_start <= 0 || selection_end >= text.length()) return;

                // Split the text
                before_text = text.left(selection_start).trimmed();
                auto middle_text = cursor.selection().toPlainText().trimmed();
                auto after_text = text.mid(selection_end).trimmed();
                if (before_text.isEmpty() || middle_text.isEmpty() || after_text.isEmpty()) return;

                auto initial_role = initial_element->role();

                // Create middle element with selection text
                LoadPlan::Item middle_item
                {
                    get_tripart_role(tripartRole, roleChoices_, initial_role),
                    middle_text,
                    false
                };

                // Create after element
                LoadPlan::Item after_item
                {
                    initial_role,
                    after_text,
                    initial_element->eot()
                };

                auto middle_index = tripart_insert
                (
                    this, 
                    index + 1,
                    middle_item,
                    after_item
                );

                eotAdjust_(elements_.at(middle_index));
            }
        }

        initial_element->setSpeech(before_text);
        eotAdjust_(initial_element);
    }

    void autoEot()
    {
        if (currentEdit_)
            currentEdit_->trim();

        for (auto& element : elements_)
            eotAdjust_(element);
    }

signals:
    void documentLoaded();

private:
    static constexpr auto EXPECTED = R"(
{
    "results": [
        {
            "Role": "Speaker 0",
            "Content" : "Hello. How are you?",
            "EndOfTurn" : true
        },
        {
            "Role": "Speaker 1",
            "Content" : "Hi, um",
            "EndOfTurn" : false
        }
    ]
}
)";

    QVBoxLayout* mainLayout_ = nullptr;
    QScrollArea* scrollArea_ = new QScrollArea(this);
    QWidget* elementsLayoutContainer_ = new QWidget(scrollArea_);

    // Not keeping JsonDocument. View is SSOT
    QVBoxLayout* elementsLayout_ = nullptr;
    QList<Element*> elements_{};
    QList<InsertButton*> insertButtons_{};
    QList<QString> roleChoices_{};

    QString currentPath_{};
    QPointer<AutoSizeTextEdit> currentEdit_{};

    // Click is a press & release
    bool ignoreNextElementSpeechEditMiddleClick_ = false;

    //CommandStack* commandStack_ = new CommandStack(this);

    void initialize_()
    {
        //setAttribute(Qt::WA_StyledBackground, true);
        //scrollArea_->setStyleSheet("background: transparent;");
        //scrollArea_->viewport()->setStyleSheet("background: transparent;");
        //elementsLayoutContainer_->setStyleSheet("background: transparent;");

        // Scroll area setup
        scrollArea_->setWidgetResizable(true);
        scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea_->setWidget(elementsLayoutContainer_);

        // Set up layouts
        // Align center causes widgets to take up their preferred size rather
        // than stretching to fill the available width
        mainLayout_ = Utility::zeroPaddedLayout<QVBoxLayout>(this, Qt::AlignCenter);
        elementsLayout_ = Utility::zeroPaddedLayout<QVBoxLayout>(elementsLayoutContainer_, Qt::AlignCenter);

        mainLayout_->addWidget(scrollArea_);

        connect
        (
            qApp,
            &QApplication::focusChanged,
            this,
            &View::onQAppFocusChanged_
        );
    }

    void eotAdjust_(Element* element)
    {
        if (!element) return;

        auto speech = element->speech().trimmed();
        if (speech.isEmpty()) return;

        // Set EOT based on whether the speech ends with terminal punctuation
        constexpr auto set_eot = [](const QString& s)
            {
                return s.endsWith('.') ||
                    s.endsWith('!') ||
                    s.endsWith('?');
            };

        element->setEot(set_eot(speech));
    }

    LoadPlan parse_(const QJsonDocument& document)
    {
        LoadPlan plan{};

        if (document.isObject())
        {
            auto root = document.object();

            if (!root.contains(Keys::RESULTS_ARRAY))
                return plan;

            // Check if the value at the "results" key is an array
            if (!root[Keys::RESULTS_ARRAY].isArray())
                return plan;

            // Check for erroneous keys, etc? Would need to set to null?

            auto array = root[Keys::RESULTS_ARRAY].toArray();

            for (const auto& value : array)
            {
                if (!value.isObject()) continue;
                plan.add(value);
            }
        }

        return plan;
    }

    QJsonDocument compile_()
    {
        QJsonObject root{};
        QJsonArray array{};

        for (auto& element : elements_)
        {
            QJsonObject object{};

            object[Keys::ROLE] = element->role();
            object[Keys::SPEECH] = element->speech();
            object[Keys::EOT] = element->eot();

            array << object;
        }

        root[Keys::RESULTS_ARRAY] = array;
        return QJsonDocument(root);
    }

    // Clears element and insert button container widgets
    void clearAllWidgets_()
    {
        QLayoutItem* item = nullptr;

        while ((item = elementsLayout_->takeAt(0)) != nullptr)
        {
            if (auto widget = item->widget())
                delete widget;

            delete item;
        }
    }

    void connectElement_(Element* element)
    {
        connect
        (
            element,
            &Element::roleChangeRequested,
            this,
            &View::onElementRoleChangeRequested_
        );

        connect
        (
            element,
            &Element::roleAddRequested,
            this,
            &View::onElementRoleAddRequested_
        );

        connect
        (
            element->speechEdit(),
            &AutoSizeTextEdit::rockeredLeft,
            this,
            [&] { split(); }
        );

        connect
        (
            element->speechEdit(),
            &AutoSizeTextEdit::rockeredRight,
            this,
            [&] { split(); }
        );

        connect
        (
            element->speechEdit(),
            &AutoSizeTextEdit::middleClicked,
            this,
            [&]
            {
                if (ignoreNextElementSpeechEditMiddleClick_)
                    ignoreNextElementSpeechEditMiddleClick_ = false;
                else
                    split();
            }
        );

        connect
        (
            element->speechEdit(),
            &AutoSizeTextEdit::mouseChorded,
            this,
            &View::onElementSpeechEditMouseChorded_
        );

        connect
        (
            element,
            &Element::deleteRequested,
            this,
            &View::onElementDeleteRequested_
        );

        /*auto eot_check = element->eotCheck();

        connect
        (
            eot_check,
            &EotCheck::toggled,
            this,
            [=](bool checked) { onElementEotCheckToggled_(eot_check, checked); }
        );*/
    }

    InsertButton* insertInsertButton_(int position)
    {
        auto button_container = new QWidget(elementsLayoutContainer_);

        auto container_layout = Utility::newLayout<QHBoxLayout>
            (
                { 1, 6, 1, 6 }, 0,
                button_container,
                Qt::AlignCenter
            );

        auto button = new InsertButton(position, button_container);
        insertButtons_.insert(position, button);

        container_layout->addWidget(button);

        // Button at position n goes at layout index n * 2
        elementsLayout_->insertWidget((position * 2), button_container);

        button->setText("+");
        button->setFixedSize(25, 25);

        connect
        (
            button,
            &InsertButton::insertRequested,
            this,
            [&](int pos) { insertElement_(pos); }
        );

        return button;
    }

    void updateInsertButtonPositions_(int startPosition)
    {
        // Update positions for all buttons from startPosition onward
        for (auto i = startPosition; i < insertButtons_.size(); ++i)
            insertButtons_[i]->setPosition(i);
    }

    void populate_(const LoadPlan& plan)
    {
        roleChoices_ = plan.roles();

        // Add the first insert button before any elements
        insertInsertButton_(0);

        for (int i = 0; i < plan.items().count(); ++i)
        {
            const auto& item = plan.items().at(i);

            auto element = new Element(elementsLayoutContainer_);
            elements_ << element;

            element->setRoleChoices(roleChoices_);

            element->setRole(item.role);
            element->setSpeech(item.speech);
            element->setEot(item.eot);

            // Element goes at layout position (i * 2) + 1
            elementsLayout_->addWidget(element);
            connectElement_(element);

            // Add insert button after this element
            insertInsertButton_(i + 1);
        }
    }

    int insertElement_(int position, const LoadPlan::Item item = {})
    {
        auto element = new Element(elementsLayoutContainer_);
        elements_.insert(position, element);
        element->setRoleChoices(roleChoices_);

        element->setRole(item.role);
        element->setSpeech(item.speech);
        element->setEot(item.eot);

        auto element_layout_lndex = (position * 2) + 1;

        elementsLayout_->insertWidget(element_layout_lndex, element);
        connectElement_(element);

        insertInsertButton_(position + 1);
        updateInsertButtonPositions_(position + 1);

        // Focus new element? May want a bool, if we ever use this in a way that
        // isn't response to user action

        auto new_speech_edit = element->speechEdit();
        auto cursor = new_speech_edit->textCursor();
        cursor.movePosition(QTextCursor::End);
        new_speech_edit->setTextCursor(cursor);
        new_speech_edit->setFocus();

        return elements_.indexOf(element);
    }

    /*void onElementEotCheckToggled_(EotCheck* eotCheck, bool now)
    {
        auto command = std::make_unique<EotCheckCommand>(eotCheck, !now, now);
        commandStack_->push(std::move(command));
    }*/

private slots:
    void onElementRoleChangeRequested_(const QString& from, const QString& to)
    {
        roleChoices_.removeAll(from);
        roleChoices_ << to;
        Utility::sort(roleChoices_);

        for (auto i = 0; i < elements_.count(); ++i)
        {
            if (auto element = elements_.at(i))
            {
                // Recall current selection
                auto current = element->role();
                element->setRoleChoices(roleChoices_);
                element->setRole((current == from) ? to : current);
            }
        }
    }

    void onElementRoleAddRequested_(const QString& role)
    {
        roleChoices_ << role;
        Utility::sort(roleChoices_);

        for (auto i = 0; i < elements_.count(); ++i)
        {
            if (auto element = elements_.at(i))
            {
                // Recall current selection
                auto current = element->role();
                element->setRoleChoices(roleChoices_);
                element->setRole(current);
            }
        }
    }

    void onQAppFocusChanged_(QWidget* old, QWidget* now)
    {
        constexpr auto to_edit = [](QWidget* w)
            {
                return qobject_cast<AutoSizeTextEdit*>(w);
            };

        if (auto edit = to_edit(old))
            if (edit == currentEdit_)
                currentEdit_ = nullptr;

        if (auto edit = to_edit(now))
            currentEdit_ = edit;
    }

    void onElementDeleteRequested_(Element* element)
    {
        auto index = elements_.indexOf(element);
        if (index < 0) return;

        elements_.removeAt(index);

        auto element_layout_index = (index * 2) + 1;
        auto insert_button_layout_index = (index + 1) * 2;

        // Remove the element widget from layout
        auto element_item = elementsLayout_->takeAt(element_layout_index);
        if (element_item)
        {
            if (auto widget = element_item->widget())
                delete widget;  // This deletes the Element widget

            delete element_item;
        }

        // Remove the trailing insert button from list
        insertButtons_.removeAt(index + 1);

        // Remove the trailing insert button widget from layout
        auto button_item = elementsLayout_->takeAt(insert_button_layout_index - 1); // -1 because we already removed one item
        if (button_item)
        {
            if (auto widget = button_item->widget())
                delete widget;  // This deletes the button container widget

            delete button_item;
        }

        // Update positions of all subsequent insert buttons
        updateInsertButtonPositions_(index + 1);
    }

    void onElementSpeechEditMouseChorded_(int key)
    {
        // This is definitely dumbly coded:

        if (key < 0) return;

        // Leave this function open to handle other keys, but pass to an
        // interrupt_ function that takes a role index arg for keys 1 through 9

        auto i = -1;

        switch (key)
        {
        default: break; // Leave -1 if not 1-9, could handle other keys later for other ops
        case Qt::Key_1: i = 0; break;
        case Qt::Key_2: i = 1; break;
        case Qt::Key_3: i = 2; break;
        case Qt::Key_4: i = 3; break;
        case Qt::Key_5: i = 4; break;
        case Qt::Key_6: i = 5; break;
        case Qt::Key_7: i = 6; break;
        case Qt::Key_8: i = 7; break;
        case Qt::Key_9: i = 8; break;
        }

        if (i == -1) return;

        ignoreNextElementSpeechEditMiddleClick_ = true;
        i = qBound(0, i, (roleChoices_.count() - 1));
        split(true, i);
    }
};
