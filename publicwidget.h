#ifndef PUBLICWIDGET_H
#define PUBLICWIDGET_H

#include <QWidget>
#include <QTimer>


class PublicTableWidget;
class RunData;
class QLabel;

class PublicWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PublicWidget(QWidget *parent = 0);
    
signals:
    
public slots:
    void refreshRuns();
    void reloadCountDown();

private slots:
    void oneSecondElapsed();

private:
    PublicTableWidget* tableOfBoats;
    QLabel* startLabel;
    int countDownCounter; // in seconds, is reloaded in the slot reloadCountDown()
    QTimer timer;
    void resizeEvent(QResizeEvent * event);
};

#endif // PUBLICWIDGET_H
