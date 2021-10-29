#ifndef QTSCROLLACCIDENTBLOCKER_H
#define QTSCROLLACCIDENTBLOCKER_H

#include <QObject>
#include <QMainWindow>

class QtScrollAccidentBlocker : public QObject
{
    Q_OBJECT
public:
    explicit QtScrollAccidentBlocker(QObject *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;

    // Returns whether the widget is derived from a scrollable class AND if it can be scrolled further in the delta's direction
    bool isWidgetScrollable(const QWidget* w, QPoint angleDelta);
    // Finds the first scrollable widget in w's ancestry (including w) or returns nullptr if no scrollable widget can be found
    QWidget* getFirstScrollableInAncestry(QWidget* w, QPoint angleDelta);

    // Call this to enable event redirection. Returns whether successful
    bool enable();

    // Returns whether successful
    bool disable();

signals:

protected:
    QWidget *mScrollActiveWidget;
    bool mEnabled;

protected slots:
    // Disconnects this slot from the scroll-active widget and resets the scroll-active variable to nullptr
    void resetScrollActive();
};

#endif // QTSCROLLACCIDENTBLOCKER_H
