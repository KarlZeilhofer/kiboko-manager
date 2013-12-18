#include "welcomescreen.h"
#include "ui_welcomescreen.h"
#include "mainwindow.h"
#include "competition.h"
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

#define SS_RECENT_FILES "RecentFiles"

WelcomeScreen::WelcomeScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeScreen)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog);
    setWindowTitle("Willkommen");
    setModal(true);

    QSettings set;

    QStringList fn = set.value(SS_RECENT_FILES).toStringList();
    ui->listWidget_recentCompetitions->addItems(fn);
    if(fn.isEmpty()){
        ui->listWidget_recentCompetitions->addItem("musterbewerb.cpt");
    }

    ui->listWidget_recentCompetitions->setCurrentRow(0);
}

WelcomeScreen::~WelcomeScreen()
{
    delete ui;
}

void WelcomeScreen::on_pushButton_newCompetition_clicked()
{
    MainWindow::app()->on_actionCreateCompetition_triggered();
    hide();
}

void WelcomeScreen::on_pushButton_viewAndEditRecent_clicked()
{
    bool ok;
    QString fn = ui->listWidget_recentCompetitions->currentItem()->text();
    if(fn.isEmpty() == false){
        ok = MainWindow::app()->openFile(fn, false);
        if(ok){
            hide();
        }
    }
}

void WelcomeScreen::on_pushButton_continueOther_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Bewerb Fortsetzen"), MainWindow::app()->getCurrentFile().absolutePath(), tr("Bewerbsdateien (*.cpt)"));

    if(fileName!=0)		// if the user selected a file
    {
        if(MainWindow::app()->competition()->isModified()){
            MainWindow::app()->askForSave();
        }

		QMessageBox* box = new QMessageBox(QMessageBox::NoIcon, tr("Autosave Datei vorhanden"),
				  tr("Soll die Autosave-Datei geladen werden?"),
				  QMessageBox::Yes | QMessageBox::No, this);
		box->exec();
		if(box->result() == QMessageBox::Yes){
			fileName.replace(".cpt", "_autosave.cpt");
		}
        if(MainWindow::app()->openFile(fileName, true)){
            hide();
        }
    }
}

void WelcomeScreen::on_pushButton_viewAndEditOther_clicked()
{
    MainWindow::app()->on_actionViewAndEdit_triggered();
    hide();
}
