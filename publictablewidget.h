#ifndef PUBLICTABLEWIDGET_H
#define PUBLICTABLEWIDGET_H

#include <QTableWidget>
#include <QTimer>

class RunData;
class QTableWidgetItem;


class PublicTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit PublicTableWidget(int numOfLines, QWidget *parent = 0);
    void setRunsList(QList<RunData*> list);
    int numOfRuns(); // returns the visible number of rows


    
signals:
    
public slots:
    void regenerateTable();

private slots:
    void refreshRunTimes();

private:
    QList<RunData*> runs;
    QTableWidgetItem* getItem(QString text);
    void resizeEvent(QResizeEvent *event);
    qreal getFontSize();
//    QTimer timer;
};

#endif // PUBLICTABLEWIDGET_H
