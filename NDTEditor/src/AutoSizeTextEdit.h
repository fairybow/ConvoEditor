#pragma once

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>
#include <QResizeEvent>
#include <QTextEdit>
#include <QWheelEvent>
#include <QWidget>

// Why doesn't this work with QPlainTextEdit?
// Set max auto height to begin using scroll bar instead
class AutoSizeTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit AutoSizeTextEdit(QWidget* parent = nullptr);

    void simplify();

signals:
    void rockeredLeft();
    void rockeredRight();
    void middleClicked();
    void mouseChorded(int key, Qt::KeyboardModifiers modifiers);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

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

private:
    bool lmbPressed_ = false;
    bool mmbPressed_ = false;
    bool rmbPressed_ = false;

private slots:
    void updateHeight_();
    void setCursorIfNoSelection_(QMouseEvent* event);

    constexpr bool isModifier_(int key) const noexcept
    {
        return key == Qt::Key_Shift
            || key == Qt::Key_Alt
            || key == Qt::Key_Control
            || key == Qt::Key_Meta;
    }
};
