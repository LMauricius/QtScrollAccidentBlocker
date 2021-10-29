#ifndef QTSCROLLACCIDENTBLOCKER_H
#define QTSCROLLACCIDENTBLOCKER_H

#include <QObject>
#include <QMainWindow>

/*
An event filter that prevents accidental changes when scrolling over value-holding widgets.
If the user scrolls over a widget that responds to wheel events,
the widget that the user scrolled in before will receive the event instead.
Currently works on QAbstractSlider, QComboBox, QTabBar, QAbstractSpinBox, QAbstractScrollArea, QScrollBar.

To use, simply install QtScrollAccidentBlocker as an event filter of QApplication,
and call QtScrollAccidentBlocker::enable().
*/
class QtScrollAccidentBlocker : public QObject
{
    Q_OBJECT
public:
    explicit QtScrollAccidentBlocker(QObject *parent = nullptr);

    // Call this to enable event redirection. Returns whether successful
    bool enable();

    // Call this to disable event redirection. Returns whether successful
    bool disable();

    bool eventFilter(QObject *watched, QEvent *event) override;
    bool isWidgetScrollable(const QWidget* w, QPoint angleDelta);// Returns whether the widget is derived from a scrollable class AND if it can be scrolled further in the delta's direction
    QWidget* getFirstScrollableInAncestry(QWidget* w, QPoint angleDelta);// Finds the first scrollable widget in w's ancestry (including w) or returns nullptr if no scrollable widget can be found

protected:
    QWidget *mScrollActiveWidget;
    bool mEnabled;

protected slots:
    // Disconnects this slot from the scroll-active widget and resets the scroll-active variable to nullptr
    void resetScrollActive();
};

#endif // QTSCROLLACCIDENTBLOCKER_H
