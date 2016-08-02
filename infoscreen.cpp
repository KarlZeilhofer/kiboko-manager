#include "infoscreen.h"
#include "mainwindow.h"
#include "QDebug"

#include <QScrollBar>
#include <QFont>
#include <QCoreApplication>
#include <QDir>

InfoScreen::InfoScreen(QWidget *parent) :
	QTextEdit(parent)
{
	setReadOnly(true);
	setFont(QFont("DejaVu Sans", 12, QFont::Bold));
	//verticalScrollBar()->setSingleStep(1);
}

void InfoScreen::appendInfo(QString text)
{
	QTime time=MainWindow::app()->getTimeBaseTime();
	setTextColor(QColor(20, 200, 20));
	append(time.toString("hh:mm:ss")+" Info: "+text);
	writeLogFile();
}

void InfoScreen::appendWarning(QString text)
{
	QTime time=MainWindow::app()->getTimeBaseTime();
	setTextColor(QColor(255, 132, 0));
	append(time.toString("hh:mm:ss")+" Warnung: "+text);
	writeLogFile();
}

void InfoScreen::appendError(QString text)
{
	QTime time=MainWindow::app()->getTimeBaseTime();
	setTextColor(QColor(255, 0, 0));
	append(time.toString("hh:mm:ss")+" Fehler: "+text);
	
	// play error sound
	MainWindow::app()->playErrorSound();
	
	writeLogFile();
}

void InfoScreen::writeLogFile()
{
	QString path=qApp->applicationDirPath();
    path.append("/logs/");

    // make the directory, if it does not exist
    QFileInfo dirInfo(path);
    if(dirInfo.exists()==false)
	{
        QDir dir(path);
        dir.mkpath(path);
    }
	QString filename=MainWindow::app()->getStartUpTime().toString("yyyy_MM_dd_hh:mm:ss")+"_log.html";
	QFile file(path+filename);
	if(file.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream out(&file);
		out << toHtml();
		file.close();
	}
	else
	{
		qDebug() << "ERROR writing log file";
	}
}
