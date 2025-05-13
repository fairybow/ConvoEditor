#pragma once

//#include <memory>

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

    void split(bool forceTripart = false)
    {
        // Prabably should no longer return an index, given that for tripart it isn't clear which we would return

        // State will depend on mouse chords. No chords will just copy state (the new element(s) will have same states). This function won't handle chords/keys, but will need parameters to mark "interruption" vs just splitting (the latter implying the 1st element should be marked EOT false). Other chords will set role for the new element (either 2nd or middle)

        // For example, splitting with highlight by using MMB would split into 3 elements that all share state (role, speech, eot). Splitting same highlighted with MMB+2 would split into 3 elements where 1st's role is same, 2nd's role is role 2 (our key), and 3rd's role is same as 1st. And doing something like MMB+Alt+2 would be an interruption, where we do the same as before (with new 2 role in middle) but also mark the 1st element as EOT false (this speaker role was interrupted).

        // If cursor is chilling somewhere, without a highlight, we split the element into 2, placing the text after the cursor in the new element.

        // If the cursor is highlighting something, we split the element into 3, placing the highlighted text in the 2nd, and the text after it in the 3rd, leaving the portion before the highlight in the 1st.

        // If there's no highlight, we can still provide a force parameter to make 3 elements instead of 2.

        if (!currentEdit_) return;

        // Find the parent Element by traversing up the widget hierarchy
        Element* initial_element = nullptr;
        auto widget = currentEdit_->parentWidget();

        while (widget)
        {
            if (auto next = qobject_cast<Element*>(widget))
            {
                initial_element = next;
                break;
            }

            widget = widget->parentWidget();
        }

        if (!initial_element) return;

        auto index = elements_.indexOf(initial_element);
        if (index < 0) return;

        auto cursor = currentEdit_->textCursor();
        auto text = currentEdit_->toPlainText();

        if (!cursor.hasSelection() && !forceTripart)
        {
            auto position = cursor.position();

            // Validate we have text on both sides of cursor
            if (position <= 0 || position >= text.length()) return;

            // Split the text
            auto before = text.left(position);
            auto after = text.mid(position);

            // Trim whitespace to check if we have actual content
            auto before_trimmed = before.trimmed();
            auto after_trimmed = after.trimmed();
            if (before_trimmed.isEmpty() || after_trimmed.isEmpty()) return;

            // Set up the new element
            LoadPlan::Item item
            {
                initial_element->role(),
                after_trimmed,
                initial_element->eot()
            };

            auto new_element_index = insertElement_((index + 1), item);

            // Update the intial element's speech
            initial_element->setSpeech(before_trimmed);
        }
        else // cursor.hasSelection() || forceTripart
        {
            auto selection_start = cursor.selectionStart();
            auto selection_end = cursor.selectionEnd();

            // Validate we have text on both sides of the selection
            if (selection_start <= 0 || selection_end >= text.length()) return;

            // Split the text
            auto before = text.left(selection_start);
            auto middle = cursor.selection().toPlainText();
            auto after = text.mid(selection_end);

            // Check that we don't have only whitespace (may want to double
            // check the logic here--what kind of highlights will we allow to
            // cause tripart? A highlighted space in between real content? Etc.
            // Check logic in bipart section, too.

            // 2 LoadPlan::Items

            //...
        }
    }

    void autoEot()
    {
        if (currentEdit_)
            currentEdit_->trim();

        for (auto& element : elements_)
        {
            auto speech = element->speech().trimmed();
            if (speech.isEmpty()) continue;

            if (speech.endsWith('.') || speech.endsWith('!') || speech.endsWith('?'))
            {
                element->setEot(true);
                continue;
            }

            element->setEot(false);
        }
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
            &AutoSizeTextEdit::rockeredRight,
            this,
            [&] { split(); }
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

        /*if (key < 0) return;

        // Leave this function open to handle other keys, but pass to an
        // interrupt_ function that takes a role index arg for keys 1 through 9

        auto i = -1;

        switch (key)
        {
        default: break; // Leave -1 if not 1-9, could handle other keys later for other ops
        case Qt::Key_1: i = 1; break;
        case Qt::Key_2: i = 2; break;
        case Qt::Key_3: i = 3; break;
        case Qt::Key_4: i = 4; break;
        case Qt::Key_5: i = 5; break;
        case Qt::Key_6: i = 6; break;
        case Qt::Key_7: i = 7; break;
        case Qt::Key_8: i = 8; break;
        case Qt::Key_9: i = 9; break;
        }

        i = qBound(1, i, roleChoices_.count());
        auto new_element_index = split();

        // Unsure whether to use new_element_index > -1 or new_element_index > 0
        if (i > -1 && new_element_index > -1)
        {
            auto interruption_index = insertElement_(new_element_index, { roleChoices_.at(i - 1) });
            elements_.at(static_cast<qsizetype>(interruption_index - 1))->setEot(false);
        }*/
    }
};
