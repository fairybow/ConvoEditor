#pragma once

#include <QFocusEvent>
#include <QMargins>
#include <QResizeEvent>
#include <QSize>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

// Why doesn't this work with QPlainTextEdit?
class AutoSizeTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit AutoSizeTextEdit(QWidget* parent = nullptr)
        : QTextEdit(parent)
    {
        // Connect to signals that indicate content or size changes
        connect
        (
            this,
            &QTextEdit::textChanged,
            this,
            &AutoSizeTextEdit::updateHeight_
        );

        auto doc = document();

        connect
        (
            doc,
            &QTextDocument::documentLayoutChanged,
            this,
            &AutoSizeTextEdit::updateHeight_
        );

        connect
        (
            doc,
            &QTextDocument::contentsChanged,
            this,
            &AutoSizeTextEdit::updateHeight_
        );

        // Initial configuration
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        // Set initial height
        updateHeight_();
    }

    void trim()
    {
        auto current = toPlainText();
        auto trimmed = current.trimmed();

        // Only update if the text actually changed
        if (current != trimmed)
        {
            // Save cursor position relative to non-whitespace content
            auto cursor = textCursor();

            auto original_cursor_pos = cursor.position();
            auto leading_spaces = 0;

            // Count leading whitespace in original text
            for (auto i = 0; i < current.length() && current[i].isSpace(); ++i)
            {
                leading_spaces++;
            }

            // Set the trimmed text
            setPlainText(trimmed);

            // Adjust cursor position
            if (original_cursor_pos > leading_spaces)
            {
                // Cursor was after leading whitespace
                auto new_cursor_pos = original_cursor_pos - leading_spaces;
                new_cursor_pos = qBound(0, new_cursor_pos, trimmed.length());
                cursor.setPosition(new_cursor_pos);
            }
            else
            {
                // Cursor was in leading whitespace, move to beginning
                cursor.setPosition(0);
            }

            setTextCursor(cursor);
        }
    }

protected:
    virtual void resizeEvent(QResizeEvent* event) override
    {
        QTextEdit::resizeEvent(event);
        updateHeight_();
    }

    virtual void focusOutEvent(QFocusEvent* event) override
    {
        QTextEdit::focusOutEvent(event);
        trim();
    }

private slots:
    void updateHeight_()
    {
        // Get the document's size for the current width
        auto doc_size = document()->size().toSize();

        // Calculate height
        auto margins = contentsMargins();
        auto y = doc_size.height()
            + margins.top()
            + margins.bottom();

        // Add frame width if there is one
        if (frameWidth() > 0)
            y += frameWidth() * 2;

        // Only update if height actually changed to avoid infinite resize loops
        if (y != height())
            setFixedHeight(y);
    }
};
