#include <QtGui/QApplication>
#include "mainwindow.h"
#include "displaypreviewwidget.h"
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include "publicwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));		// all messages in tr() are interpreted in UTF-8 so the file encoding for all source-files should be UTF-8 too.
    QCoreApplication::setOrganizationName("kiboko-systems");
    QCoreApplication::setApplicationName("kiboko");
	MainWindow w;
	w.show();

//    PublicWidget p;
//    p.show();
	
	// translation
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
        QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    return a.exec();
}
