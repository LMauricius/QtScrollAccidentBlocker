#include "QtScrollAccidentBlocker.h"
#include <QEvent>
#include <QWheelEvent>
#include <QWidget>
#include <QApplication>
#include <QDebug>

#include <QTabBar>
#include <QComboBox>
#include <QAbstractSpinBox>
#include <QAbstractSlider>
#include <QAbstractScrollArea>
#include <QScrollBar>

QtScrollAccidentBlocker::QtScrollAccidentBlocker(QObject *parent):
    QObject(parent),
    mScrollActiveWidget(nullptr),
    mEnabled(false)
{

}

bool QtScrollAccidentBlocker::eventFilter(QObject *watched, QEvent *event)
{
    if (mEnabled && watched->isWidgetType())
    {
        QWidget *w = (QWidget*)watched;

        switch (event->type())
        {
        case QEvent::Type::MouseMove:
        case QEvent::Type::MouseButtonPress:
        case QEvent::Type::MouseButtonDblClick:
            // Reset scroll-active to 0 when the mouse moves.
            // This ensures that the next scroll event will be propagated to the widget under cursor
            // On the next scroll event the receiver object should be set as the scroll-active widget
            resetScrollActive();
            break;
        case QEvent::Type::Wheel:
        {
            QWheelEvent *we = (QWheelEvent*) event;

            // In case the receiver widget is a non-scrollable widget inside a scrollable area, find first scrollable parent
            if (QWidget *scrollable = getFirstScrollableInAncestry(w, we->angleDelta()))
            {
                // Redirect "scroll owner" from ScrollBar to owning ScrollArea
                if (dynamic_cast<QScrollBar*>(scrollable))
                {
                    QWidget *parentScrollable = getFirstScrollableInAncestry(scrollable->parentWidget(), we->angleDelta());
                    if (QAbstractScrollArea *parentScrollArea = dynamic_cast<QAbstractScrollArea*>(parentScrollable))
                    {
                        if (scrollable == parentScrollArea->horizontalScrollBar() || scrollable == parentScrollArea->verticalScrollBar())
                        {
                            scrollable = parentScrollArea;
                        }
                    }
                }

                // Change the scroll-active widget to the receiver only if the scroll-active variable has been reset
                // (i.e. prevent changing the scroll-active variable unless explicitly required)
                if (mScrollActiveWidget == nullptr)
                {
                    mScrollActiveWidget = scrollable;

                    // In case the scroll-active widget is deleted, forget about it
                    mScrollActiveWidget->connect(
                        mScrollActiveWidget, SIGNAL(destroyed(QObject*)),
                        this, SLOT(resetScrollActive())
                    );
                }
                else
                {
                    // If the receiver isn't the scroll-active widget, redirect the event to the scroll-active one
                    // DON'T DO IT if the receiver is parent of the previous scroll-active ScrollArea.
                    // (This happens when we reached the end of the scroll area, and scrolling continues in the parent scrollArea)
                    if (scrollable != mScrollActiveWidget &&
                        !(
                            dynamic_cast<QAbstractScrollArea*>(mScrollActiveWidget) &&
                            scrollable->isAncestorOf(mScrollActiveWidget)
                        )
                    )
                    {
                        QApplication::instance()->sendEvent(mScrollActiveWidget, event);
                        return true;
                    }
                    else
                    {
                        // Proceed normally
                    }
                }

            }
            break;
        }
        }
    }

    return false;
}

bool QtScrollAccidentBlocker::enable()
{
    mEnabled = true;
    return true;
}

bool QtScrollAccidentBlocker::disable()
{
    mEnabled = false;
    return true;
}

bool QtScrollAccidentBlocker::isWidgetScrollable(const QWidget* w, QPoint angleDelta)
{
    if (
        dynamic_cast<const QAbstractSlider*>(w) ||
        dynamic_cast<const QComboBox*>(w) ||
        dynamic_cast<const QTabBar*>(w) ||
        dynamic_cast<const QAbstractSpinBox*>(w) ||
        dynamic_cast<const QAbstractScrollArea*>(w)||// The events are redirected to respective srollbars
        dynamic_cast<const QScrollBar*>(w)
    ) {
        // We could check if comboboxes and spinboxes have reached the end of their ranges,
        // but Qt by default doesn't propagate scroll events to theese widgets' parent(s),
        // and perhaps for the better
        return true;
    }

    return false;
}

QWidget* QtScrollAccidentBlocker::getFirstScrollableInAncestry(QWidget* w, QPoint angleDelta)
{
    while (w)
    {
        if (isWidgetScrollable(w, angleDelta))
        {
            return w;
        }
        w = w->parentWidget();
    }
    return w;
}

void QtScrollAccidentBlocker::resetScrollActive()
{
    if (mScrollActiveWidget != nullptr)
    {
        mScrollActiveWidget->disconnect(
            mScrollActiveWidget, SIGNAL(destroyed(QObject*)),
            this, SLOT(resetScrollActive())
        );
    }
    mScrollActiveWidget = nullptr;
}
