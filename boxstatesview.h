#ifndef BOXSTATESVIEW_H
#define BOXSTATESVIEW_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include "boxstates.h"

class BoxStatesView : public QWidget
{
    Q_OBJECT
public:
    explicit BoxStatesView(BoxStates *bs, QWidget *parent = 0);
    
signals:
    
public slots:

private:
    QList<QLabel*> topHeadings;
    QList<QLabel*> leftHeadings;
    QList<QLabel*> entries;

    // time-base:
};

#endif // BOXSTATESVIEW_H
