#include "autosavemanager.h"
#include "mainwindow.h"
#include <QCoreApplication>
#include <QDir>
#include "competition.h"
#include "infoscreen.h"

AutoSaveManager::AutoSaveManager(QObject *parent) :
    QObject(parent)
{
    timer.setInterval(5000);
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerTimedOut()));
    saveOnTimeOut = false;
    timedOut = false;

    timer.start();
}

// private procedure, which does the autosave
// it is called from the slot doAutoSave()
void AutoSaveManager::save()
{
	QFileInfo currentFile = MainWindow::app()->getCurrentFile();

    // prepare the autosave-filename:
    QString path;
    QString fileName;

    if(currentFile.fileName().isEmpty()){ // if unnamed file, set path to <pragrammpath>/unnamed_autosave/
        path = qApp->applicationDirPath();
        path.append("/unnamed_autosave/");
        fileName = QString("unnamed");

        // make the directory, if it does not exist
        QFileInfo dirInfo(path);
        if(dirInfo.exists() == false){
            QDir dir(path);
            dir.mkpath(path);
        }
    }else{
        fileName = currentFile.baseName();
        path = currentFile.absolutePath();
        path.append("/");
    }

    fileName.append("_autosave.cpt");

    // write to file:
    if(MainWindow::app()->competition()->save(path+fileName)){
        qDebug() << "wrote autosave to:" << path+fileName;
        //infoscreen()->appendInfo(tr("Bewerbsdatei wurde gesichert"));
    }else{
        qDebug() << "ERROR on writing backup to:" << path+fileName;
        MainWindow::app()->infoscreen()->appendWarning(tr("Automatisches Speichern der Bewerbsdatei gescheitert!"));
    }
}

// this slot is connected to Competition::runChanged()
// and processes the autosave, max. every 5 seconds!
// if not processed, set a flag, that it should be processed after the timer finishes.
void AutoSaveManager::doAutoSave()
{
    if(timedOut){
        save();
        timedOut = false;
        saveOnTimeOut = false;
        timer.start();
    }else{
        saveOnTimeOut = true; // dirty-flag, the competition was changed while the timer is running.
    }
}

void AutoSaveManager::timerTimedOut()
{
    timedOut = true;
    timer.stop();

    if(saveOnTimeOut){
        doAutoSave(); // trigger the save() and restart the timer
    }
}
