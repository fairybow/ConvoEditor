#pragma once

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMargins>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QSize>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QWheelEvent>
#include <QList>

// Why doesn't this work with QPlainTextEdit?
// Set max auto height to begin using scroll bar instead
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

    void simplify()
    {
        auto current = toPlainText();
        auto simplified = current.simplified();
        if (current == simplified) return;

        auto cursor = textCursor();
        auto initial_cursor_pos = cursor.position();

        // Build position mapping between original and normalized text
        QList<int> map(current.length());
        auto simplified_pos = 0;

        // Skip leading whitespace in original
        auto leading_spaces = 0;

        while (leading_spaces < current.length() && current[leading_spaces].isSpace())
            leading_spaces++;

        auto last_c_was_space = false;

        for (auto i = 0; i < current.length(); i++)
        {
            map[i] = simplified_pos;
            if (i < leading_spaces) continue;

            if (current[i].isSpace())
            {
                if (!last_c_was_space)
                {
                    simplified_pos++;
                    last_c_was_space = true;
                }
            }
            else
            {
                simplified_pos++;
                last_c_was_space = false;
            }
        }

        setPlainText(simplified);

        // Adjust cursor position (if it was at the end, map length may be
        // exceeded)
        auto new_pos = (initial_cursor_pos >= map.size())
            ? simplified.length()
            : map[initial_cursor_pos];

        new_pos = qBound(0, new_pos, simplified.length());
        cursor.setPosition(new_pos);
        setTextCursor(cursor);
    }

signals:
    void rockeredLeft();
    void rockeredRight();
    void middleClicked();
    void mouseChorded(Qt::Key key, Qt::KeyboardModifiers modifiers);

protected:
    virtual void resizeEvent(QResizeEvent* event) override
    {
        QTextEdit::resizeEvent(event);
        updateHeight_();
    }

    virtual void focusOutEvent(QFocusEvent* event) override
    {
        QTextEdit::focusOutEvent(event);
        simplify();
    }

    virtual void wheelEvent(QWheelEvent* event) override
    {
        // Prevent scrolling
        event->ignore();
    }

    virtual void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            if (rmbPressed_)
            {
                // Right button is already held, left click detected
                emit rockeredLeft();
                event->accept();
                return;
            }

            lmbPressed_ = true;
        }
        else if (event->button() == Qt::RightButton)
        {
            if (lmbPressed_)
            {
                // Left button is already held, right click detected
                emit rockeredRight();
                event->accept();
                return;
            }

            // Allow RMB to set cursor unless there's a selection
            setCursorOnNoSelection_(event);
            rmbPressed_ = true;
        }
        else if (event->button() == Qt::MiddleButton)
        {
            // Allow MMB to set cursor unless there's a selection
            setCursorOnNoSelection_(event);
            mmbPressed_ = true;
        }

        QTextEdit::mousePressEvent(event);
    }

    virtual void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
            lmbPressed_ = false;
        else if (event->button() == Qt::RightButton)
            rmbPressed_ = false;
        else if (event->button() == Qt::MiddleButton)
        {
            mmbPressed_ = false;
            emit middleClicked();
        }

        QTextEdit::mouseReleaseEvent(event);
    }

    virtual void keyPressEvent(QKeyEvent* event) override
    {
        auto key = event->key();

        if (mmbPressed_)
        {
            if (key != Qt::Key_unknown && key > 0)
            {
                if (!isModifier_(key))
                    emit mouseChorded(static_cast<Qt::Key>(key), event->modifiers());
            }

            event->ignore();
            return;
        }

        switch (key)
        {
            // Prevent scrolling
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            event->ignore();
            return;

        default:
            QTextEdit::keyPressEvent(event);
        }
    }

private:
    bool lmbPressed_ = false;
    bool mmbPressed_ = false;
    bool rmbPressed_ = false;

private slots:
    void updateHeight_()
    {
        // There's some extra space below the text. However, for now, it seems
        // preventing scrolling makes this not an issue

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

    void setCursorOnNoSelection_(QMouseEvent* event)
    {
        auto cursor = textCursor();
        if (cursor.hasSelection()) return;

        cursor = cursorForPosition(event->pos());
        setTextCursor(cursor);
    }

    constexpr bool isModifier_(int key) const noexcept
    {
        return key == Qt::Key_Shift
            || key == Qt::Key_Alt
            || key == Qt::Key_Control
            || key == Qt::Key_Meta;
    }
};
