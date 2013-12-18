#ifndef AUTOSAVEMANAGER_H
#define AUTOSAVEMANAGER_H

#include <QObject>
#include <QTimer>

class AutoSaveManager : public QObject
{
    Q_OBJECT
public:
    explicit AutoSaveManager(QObject *parent = 0);
private:
    void save();
    
signals:
    
public slots:
    void doAutoSave();

private slots:
    void timerTimedOut();

private:
    QTimer timer;
    bool timedOut; // is set, when the timer timed out, which is started on an auto-save
    bool saveOnTimeOut;
    
};

#endif // AUTOSAVEMANAGER_H
