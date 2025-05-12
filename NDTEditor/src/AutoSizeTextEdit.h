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
#include <QTimer>
#include <QWheelEvent>

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

        /// WELP: we also want to debounce other editors, because we will be
        /// switching focus when we use the MMB gesture.
        mmbHeldKeyDebouncer_.setSingleShot(true);

        connect
        (
            &mmbHeldKeyDebouncer_,
            &QTimer::timeout,
            this,
            [&] { mmbHeldKey_ = -1; }
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

signals:
    void leftRockered();
    void rightRockered();
    void middleClickReleased(int key = -1);

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
                emit leftRockered();
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
                emit rightRockered();
                event->accept();
                return;
            }

            rmbPressed_ = true;
        }
        else if (event->button() == Qt::MiddleButton)
        {
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
            emit middleClickReleased(mmbHeldKey_);

            // When a key is held, it will auto-repeat. So, the press & release
            // events will continuously fire one after the other. We can't rely
            // on keyReleaseEvent for our exit condition, and we still need to
            // block mmbHeldKey_ till its release. Blocking this prevents the
            // awkward "I accidentally typed my gesture key because I released
            // MMB a little early" issue
            if (mmbHeldKey_ > -1)
                mmbHeldKeyDebouncer_.start(DEBOUNCE_INTERVAL);
        }

        QTextEdit::mouseReleaseEvent(event);
    }

    virtual void keyPressEvent(QKeyEvent* event) override
    {
        auto key = event->key();

        if (mmbPressed_)
        {
            mmbHeldKey_ = key;
            event->ignore();
            return;
        }

        if (key == mmbHeldKey_ && mmbHeldKeyDebouncer_.isActive())
        {
            mmbHeldKeyDebouncer_.start(DEBOUNCE_INTERVAL);
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

    //virtual void keyReleaseEvent(QKeyEvent* event) override
    //{
    //    QTextEdit::keyReleaseEvent(event);
    //}

private:
    bool lmbPressed_ = false;
    bool mmbPressed_ = false;
    bool rmbPressed_ = false;
    int mmbHeldKey_ = -1;
    static constexpr auto DEBOUNCE_INTERVAL = 300;
    QTimer mmbHeldKeyDebouncer_{};

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
};
