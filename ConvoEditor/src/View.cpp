#include <utility>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QPropertyAnimation>
#include <QRect>
#include <QScrollArea>
#include <QScrollBar>
#include <QString>
#include <QStringList>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QtTypes>
#include <QVBoxLayout>
#include <QWidget>

#include "Coco/Io.h"
#include "Coco/Layout.h"
#include "Coco/Path.h"
#include "Coco/Utility.h"

#include "AutoSizeTextEdit.h"
#include "Element.h"
#include "Eot.h"
#include "InsertButton.h"
#include "LoadPlan.h"
#include "Utility.h"
#include "View.h"

constexpr auto EXPECTED = R"(
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

View::View(QWidget* parent)
    : QWidget(parent)
{
    initialize_();
}

View::~View()
{
    qDebug() << __FUNCTION__;
}

bool View::load(const Coco::Path& path)
{
    auto document = Coco::Io::Json::read(path);
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
        clearAllContent_();
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

bool View::save()
{
    if (currentPath_.isEmpty()) return false;
    if (currentEdit_) currentEdit_->simplify();

    auto document = compile_();
    if (document.isNull()) return false;

    return Coco::Io::Json::write(document, currentPath_);
}

void View::split(bool forceTripart, int tripartRole)
{
    // Splits text elements at cursor position in three ways:
    // 1. Bipart (2-way): No selection -> splits at cursor into before/after
    // 2. Empty tripart: No selection but forceTripart-> adds empty middle
    //    element
    // 3. Selection tripart: With selection -> places selected text in middle
    // 
    // Triggered by: right rocker or MMB click. Making a mouse chord (MMB + n)
    // will force tripart regardless of selection and assign n role to the
    // middle element (useful for quickly splitting as an interruption)
    // 
    // Automatically adjusts EOT based on punctuation.
    if (!currentEdit_) return;

    auto initial_element = Coco::findParent<Element>(currentEdit_);
    if (!initial_element) return;

    auto index = elements_.indexOf(initial_element);
    if (index < 0) return;

    auto cursor = currentEdit_->textCursor();
    auto position = cursor.position();
    auto has_selection = cursor.hasSelection();
    auto text = currentEdit_->toPlainText();

    QString before_text{};
    int scroll_to = -1;

    if (!has_selection && !forceTripart)
    {
        // 1. Break into 2

        // Validate we have text on both sides of cursor
        if (position <= 0 || position >= text.length()) return;

        /// TEST
        auto is_break = Utility::wouldBreakWord(text, position);

        // Split the text
        before_text = text.left(position).trimmed();
        auto after_text = text.mid(position).trimmed();
        Utility::shiftPunct(before_text, after_text);
        if (before_text.isEmpty() || after_text.isEmpty()) return;

        /// TEST
        if (is_break)
            Utility::applyBreakIndicators(before_text, after_text);

        // Create element plan
        LoadPlan::Item item
        {
            initial_element->role(),
            after_text,
            initial_element->eot()
        };

        scroll_to = insertElement_(index + 1, item);
    }
    else // has_selection || forceTripart
    {
        auto get_tripart_role =
            [](int role, const QStringList& roles, const QString& fallback) noexcept
            {
                return (role > -1)
                    ? roles.at(role)
                    : fallback;
            };

        auto tripart_insert =
            [](View* v, int insertIndex, const LoadPlan::Item& middle, const LoadPlan::Item& after) noexcept
            {
                // Insert the elements: after first, then middle (which puts
                // middle between initial and after)

                // Capturing after index is pointless, because it
                // immediately changes
                v->insertElement_(insertIndex, after);
                auto middle_index = v->insertElement_(insertIndex, middle);
                return std::pair<int, int>{ middle_index, middle_index + 1 };
            };

        if (!has_selection)
        {
            // 2. Break into 3 (with empty middle element)

            // Validate we have text on both sides of cursor
            if (position <= 0 || position >= text.length()) return;

            /// TEST
            auto is_break = Utility::wouldBreakWord(text, position);

            // Split the text
            before_text = text.left(position).trimmed();
            auto after_text = text.mid(position).trimmed();
            Utility::shiftPunct(before_text, after_text);
            if (before_text.isEmpty() || after_text.isEmpty()) return;

            /// TEST
            if (is_break)
                Utility::applyBreakIndicators(before_text, after_text);

            auto initial_role = initial_element->role();

            // Create element plans
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

            auto indexes = tripart_insert
            (
                this,
                index + 1,
                middle_item,
                after_item
            );

            scroll_to = indexes.second;
        }
        else // has_selection
        {
            // 3. Break into 3 with selection in the middle element

            auto selection_start = cursor.selectionStart();
            auto selection_end = cursor.selectionEnd();

            // Validate we have text on both sides of the selection
            if (selection_start <= 0 || selection_end >= text.length()) return;

            /// TEST
            auto first_is_break = Utility::wouldBreakWord(text, selection_start);
            /// TEST
            auto second_is_break = Utility::wouldBreakWord(text, selection_end);

            // Split the text
            before_text = text.left(selection_start).trimmed();
            auto middle_text = cursor.selection().toPlainText().trimmed();
            auto after_text = text.mid(selection_end).trimmed();
            Utility::shiftPunct(before_text, middle_text);
            Utility::shiftPunct(middle_text, after_text);
            if (before_text.isEmpty() || middle_text.isEmpty() || after_text.isEmpty()) return;

            /// TEST
            if (first_is_break)
                Utility::applyBreakIndicators(before_text, middle_text);
            /// TEST
            if (second_is_break)
                Utility::applyBreakIndicators(middle_text, after_text);

            // Create element plans
            auto initial_role = initial_element->role();

            LoadPlan::Item middle_item
            {
                get_tripart_role(tripartRole, roleChoices_, initial_role),
                middle_text,
                false
            };

            LoadPlan::Item after_item
            {
                initial_role,
                after_text,
                initial_element->eot()
            };

            auto indexes = tripart_insert
            (
                this,
                index + 1,
                middle_item,
                after_item
            );

            eotAdjust_(elements_.at(indexes.first));
            scroll_to = indexes.second;
        }
    }

    initial_element->setSpeech(before_text);
    eotAdjust_(initial_element);
    scrollToContent_(((scroll_to * 2) + 1) + 1); // Include the button container
}

void View::initialize_()
{
    // Scroll area setup
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setWidget(contentContainer_);

    // Set up layouts
    // Align center causes widgets to take up their preferred size rather
    // than stretching to fill the available width
    mainLayout_ = Coco::Layout::zeroPadded<QVBoxLayout>(this, Qt::AlignCenter);
    contentLayout_ = Coco::Layout::zeroPadded<QVBoxLayout>(contentContainer_, Qt::AlignCenter);

    mainLayout_->addWidget(scrollArea_);

    connect
    (
        qApp,
        &QApplication::focusChanged,
        this,
        &View::onQAppFocusChanged_
    );
}

void View::scrollToContent_(int contentIndex)
{
    // I hate timers.

    // Use QTimer to ensure widget is laid out before scrolling. When using
    // interval 0, the scroll doesn't work at end of document
    QTimer::singleShot(100, this, [=]() {
        auto scroll_bar = scrollArea_->verticalScrollBar();
        if (!scroll_bar) return;

        auto widget = contentLayout_->itemAt(contentIndex)->widget();
        if (!widget) return;

        /// Testing (don't scroll if widget is already visible)
        ///----------------------------------

        auto viewport = scrollArea_->viewport();
        auto widget_top_left = widget->mapTo(viewport, QPoint(0, 0));
        auto widget_bottom_right = widget->mapTo(viewport, QPoint(widget->width(), widget->height()));
        QRect viewport_rect(0, 0, viewport->width(), viewport->height());
        QRect widget_rect(widget_top_left, widget_bottom_right);
        if (viewport_rect.contains(widget_rect)) return;

        ///----------------------------------

        auto widget_bottom = widget->mapTo(contentContainer_, QPoint(0, widget->height())).y();
        auto viewport_height = scrollArea_->viewport()->height();
        auto to = widget_bottom - viewport_height;
        to = qBound(0, to, scroll_bar->maximum());
        auto from = scroll_bar->value();

        auto animation = new QPropertyAnimation(scroll_bar, "value");
        animation->setDuration(300);
        animation->setStartValue(from);
        animation->setEndValue(to);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        });
}

void View::eotAdjust_(Element* element)
{
    if (!element) return;

    auto speech = element->speech().trimmed();
    if (speech.isEmpty()) return;

    if (Eot::endsWithFiller(speech))
    {
        element->setEot(false);
        return;
    }

    element->setEot(Eot::hasTerminalPunct(speech));
}

LoadPlan View::parse_(const QJsonDocument& document)
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

QJsonDocument View::compile_()
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

void View::connectElement_(Element* element)
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
            if (ignoreNextSpeechEditMClick_)
                ignoreNextSpeechEditMClick_ = false;
            else
                split();
        }
    );

    connect
    (
        element->speechEdit(),
        &AutoSizeTextEdit::mouseChorded,
        this,
        &View::onSpeechEditMouseChorded_
    );

    connect
    (
        element,
        &Element::deleteRequested,
        this,
        &View::onElementDeleteRequested_
    );
}

void View::insertInsertButton_(int position)
{
    auto button_container = new QWidget(contentContainer_);

    auto container_layout = Coco::Layout::make<QHBoxLayout>
        (
            { 1, 6, 1, 6 }, 0,
            button_container,
            Qt::AlignCenter
        );

    auto button = new InsertButton(position, button_container);
    insertButtons_.insert(position, button);

    container_layout->addWidget(button);

    button->setText("+");
    button->setFixedSize(25, 25);

    connect
    (
        button,
        &InsertButton::insertRequested,
        this,
        [&](int pos)
        {
            // Need to fix element vs content vs button index :(((((
            auto element_index = insertElement_(pos);
            scrollToContent_(((element_index * 2) + 1) + 1); // Include the button
        }
    );

    // Button at position n goes at layout index n * 2
    contentLayout_->insertWidget((position * 2), button_container);
}

void View::populate_(const LoadPlan& plan)
{
    roleChoices_ = plan.roles();

    // Add the first insert button before any elements
    insertInsertButton_(0);

    for (int i = 0; i < plan.items().count(); ++i)
    {
        const auto& item = plan.items().at(i);

        auto element = new Element(contentContainer_);
        elements_ << element;

        element->setRoleChoices(roleChoices_);

        element->setRole(item.role);
        element->setSpeech(item.speech);
        element->setEot(item.eot);

        // Element goes at layout position (i * 2) + 1
        contentLayout_->addWidget(element);
        connectElement_(element);

        // Add insert button after this element
        insertInsertButton_(i + 1);
    }
}

// Does not return a content index!
int View::insertElement_(int position, const LoadPlan::Item item)
{
    auto element = new Element(contentContainer_);
    elements_.insert(position, element);
    element->setRoleChoices(roleChoices_);

    element->setRole(item.role);
    element->setSpeech(item.speech);
    element->setEot(item.eot);

    auto element_layout_index = (position * 2) + 1;

    contentLayout_->insertWidget(element_layout_index, element);
    connectElement_(element);

    insertInsertButton_(position + 1);
    updateInsertButtonPositions_(position + 1);

    // Focus new element
    auto new_speech_edit = element->speechEdit();
    auto cursor = new_speech_edit->textCursor();
    cursor.movePosition(QTextCursor::End);
    new_speech_edit->setTextCursor(cursor);
    new_speech_edit->setFocus();

    return elements_.indexOf(element);
}

void View::onElementRoleChangeRequested_(const QString& from, const QString& to)
{
    roleChoices_.removeAll(from);
    roleChoices_ << to;
    Coco::Utility::sort(roleChoices_);

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

void View::onElementRoleAddRequested_(const QString& role)
{
    roleChoices_ << role;
    Coco::Utility::sort(roleChoices_);

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

void View::onQAppFocusChanged_(QWidget* old, QWidget* now)
{
    auto to_edit = [](QWidget* widget)
        {
            return qobject_cast<AutoSizeTextEdit*>(widget);
        };

    if (auto edit = to_edit(old))
        if (edit == currentEdit_)
            currentEdit_ = nullptr;

    if (auto edit = to_edit(now))
        currentEdit_ = edit;
}

void View::onElementDeleteRequested_(Element* element)
{
    auto index = elements_.indexOf(element);
    if (index < 0) return;

    elements_.removeAt(index);

    auto element_layout_index = (index * 2) + 1;
    auto insert_button_layout_index = (index + 1) * 2;

    // Remove the element widget from layout
    removeContent_(element_layout_index);

    // Remove the trailing insert button from list
    insertButtons_.removeAt(index + 1);

    // Remove the trailing insert button widget from layout
    removeContent_(insert_button_layout_index - 1); // -1 because we already removed one item

    // Update positions of all subsequent insert buttons
    updateInsertButtonPositions_(index + 1);
}

// Revise (duh). We may want just a key to combo (alt + something/else) to
// add break indicators to beginning or end of text in field (not as part of
// split, because a tripart would still involve deducing the break, which is
// too complicated for smol bean brain
void View::onSpeechEditMouseChorded_(int key, Qt::KeyboardModifiers modifiers) // Qt::KeyCombo or whatever it is?
{
    // Don't continue with the split even if a no-op key is chorded
    ignoreNextSpeechEditMClick_ = true;

    auto i = -1;

    // May handle other chords later
    switch (key)
    {
    default: break;
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

    auto max = roleChoices_.count() - 1;
    split(true, qBound(0, i, max));
}
