#include "QtScrollAccidentBlocker.h"
#include <QEvent>
#include <QWheelEvent>
#include <QWidget>
#include <QApplication>
#include <QDebug>

#include <QComboBox>
#include <QAbstractSpinBox>
#include <QAbstractSlider>
#include <QAbstractScrollArea>
#include <QScrollBar>

QtScrollAccidentBlocker::QtScrollAccidentBlocker(QObject *parent):
    QObject(parent),
    mScrollActiveWidget(nullptr)
{

}

bool QtScrollAccidentBlocker::eventFilter(QObject *watched, QEvent *event)
{
    if (watched->isWidgetType())
    {
        QWidget *w = (QWidget*)watched;

        switch (event->type())
        {
        //case QEvent::Type::MouseEnter:
        case QEvent::Type::MouseMove:
            // Reset scroll-active to 0 when the mouse moves.
            // This ensures that the next scroll event will be propagated to the widget under cursor
            // On the next scroll event the receiver object should be set as the scroll-active widget
            resetScrollActive();
            qDebug() << (QString("reset scrollA %1\n").arg((size_t)mScrollActiveWidget));
            break;
        case QEvent::Type::Wheel:
        {
            QWheelEvent *we = (QWheelEvent*) event;

            // Allow scrolling in another widget if current scroll-active widget isn't scrollable in the delta's direction anymore
            // (for example, allow scrolling parent scroll-area if the current one reached its end)
            if (mScrollActiveWidget && !isWidgetScrollable(mScrollActiveWidget, we->angleDelta()))
            {
                resetScrollActive();
            }

            // In case the receiver widget is a non-scrollable widget inside a scrollable area, find first scrollable parent
            if (QWidget *scrollW = getScrollableParent(w, we->angleDelta()))
            {
                // Change the scroll-active widget to the receiver only if the scroll-active variable has been reset
                // i.e. prevent changing the scroll-active variable unless explicitly required
                if (mScrollActiveWidget == nullptr)
                {
                    mScrollActiveWidget = scrollW;

                    // In case the scroll-active widget is deleted, forget about it
                    mScrollActiveWidget->connect(
                        mScrollActiveWidget, SIGNAL(destroyed(QObject*)),
                        this, SLOT(resetScrollActive())
                    );
                    //qDebug() << QString("changed cur %1  scrollA %2\n").arg((size_t)scrollW).arg((size_t)mScrollActiveWidget) << we->angleDelta();
                }
                else
                {
                    // If the receiver isn't the scroll-active widget, redirect the event to the scroll-active one
                    if (scrollW != mScrollActiveWidget/* && isWidgetScrollable(mScrollActiveWidget, we->angleDelta())*/)
                    {
                        //qDebug() << QString("stopped cur %1  scrollA %2 (%3 %4)\n").arg((size_t)scrollW).arg((size_t)mScrollActiveWidget) << we->angleDelta();
                        QApplication::instance()->sendEvent(mScrollActiveWidget, event);
                        return true;
                    }
                    else
                    {
                        //qDebug() << QString("filtered cur %1  scrollA %2 (%3 %4)\n").arg((size_t)scrollW).arg((size_t)mScrollActiveWidget) << we->angleDelta();
                    }
                }

            }
            break;
        }
        case QEvent::Type::ScrollPrepare:// This might be something of interest
            break;
        }
    }

    return false;
}

bool QtScrollAccidentBlocker::isWidgetScrollable(const QWidget* w, QPoint angleDelta)
{
    if (auto s=dynamic_cast<const QAbstractSlider*>(w)
    ) {
        int delta = (s->orientation() == Qt::Vertical)? angleDelta.y() : angleDelta.x();
        return (delta >= 0 && s->sliderPosition() > s->minimum()) || (delta <= 0 && s->sliderPosition() < s->maximum());
    }
    else if (
        dynamic_cast<const QComboBox*>(w) ||
        dynamic_cast<const QAbstractSpinBox*>(w) ||
        //dynamic_cast<const QAbstractScrollArea*>(w)||// The events are redirected to respective srollbars
        dynamic_cast<const QScrollBar*>(w)
    ) {
        // We could check if comboboxes and spinboxes have reached the end of their ranges,
        // but Qt by default doesn't propagate scroll events to theese widgets' parent(s),
        // and perhaps for the better
        return true;
    }

    return false;
}

QWidget* QtScrollAccidentBlocker::getScrollableParent(QWidget* w, QPoint angleDelta)
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
