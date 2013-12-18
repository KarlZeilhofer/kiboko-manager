#include "dialogdisplaysettings.h"
#include "ui_dialogdisplaysettings.h"
#include <QFont>
#include <QFontDialog>
#include "displayrenderer.h"
#include "rundata.h"
#include <QDebug>
#include <QSettings>

DialogDisplaySettings::DialogDisplaySettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDisplaySettings)
{
    ui->setupUi(this);
	setModal(true);

    ui->displayPreviewWidget_1->setScale(8);
    ui->displayPreviewWidget_2->setScale(8);

    renderer = new DisplayRenderer(ui->displayPreviewWidget_1,
                                   ui->displayPreviewWidget_2, this);
    runData = new RunData(this);

    lineEdit_formats[1] = ui->lineEdit_line1Format;
    lineEdit_formats[2] = ui->lineEdit_line2Format;
    lineEdit_formats[3] = ui->lineEdit_line3Format;
    lineEdit_formats[4] = ui->lineEdit_line4Format;
    lineEdit_formats[5] = ui->lineEdit_line5Format;
    lineEdit_formats[6] = ui->lineEdit_line6Format;

    lineEdit_fonts[1] = ui->lineEdit_line1Font;
	lineEdit_fonts[2] = ui->lineEdit_line2Font;
	lineEdit_fonts[3] = ui->lineEdit_line3Font;
	lineEdit_fonts[4] = ui->lineEdit_line4Font;
	lineEdit_fonts[5] = ui->lineEdit_line5Font;
	lineEdit_fonts[6] = ui->lineEdit_line6Font;

    offsets[1] = ui->spinBox_1;
    offsets[2] = ui->spinBox_2;
    offsets[3] = ui->spinBox_3;
    offsets[4] = ui->spinBox_4;
    offsets[5] = ui->spinBox_5;
    offsets[6] = ui->spinBox_6;
	
	// restore settings
	QSettings set;
	set.beginGroup("display");
	for(int i=1; i<=6; i++)
	{
		lineEdit_formats[i]->setText(set.value(QString("format%1").arg(i), QString()).toString());
		QFont f;
		f.fromString(set.value(QString("font%1").arg(i), "DejaVu Sans,8,-1,5,50,0,0,0,0,0").toString());
		f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
		fonts[i]=f;
		offsets[i]->setValue(set.value(QString("offset%1").arg(i), 0).toInt());
	}
	ui->checkBox_enableDisplays->setChecked(set.value("enabled", true).toBool());
	ui->checkBox_flipDisplay1->setChecked(set.value("flipDisplay1", false).toBool());
	ui->checkBox_flipDisplay2->setChecked(set.value("flipDisplay2", false).toBool());
	set.endGroup();


	/*
    // Set Default Strings:
    QFont f;
    f.fromString("DejaVu Sans,8,-1,5,50,0,0,0,0,0");
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    fonts[1] = f;
    f.fromString("DejaVu Sans,8,-1,5,50,0,0,0,0,0");
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    fonts[2] = f;
    f.fromString("DejaVu Sans,8,-1,5,150,0,0,0,0,0");
    f.setLetterSpacing(QFont::AbsoluteSpacing, -1);
    fonts[3] = f;

    f.fromString("DejaVu Sans,12,-1,5,100,0,0,0,0,0");
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    fonts[4] = f;
    f.fromString("DejaVu Sans,10,-1,5,100,0,0,0,0,0");
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    fonts[5] = f;
    f.fromString("DejaVu Sans,8,-1,5,100,0,0,0,0,0");
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    fonts[6] = f;
	*/

    for(int n=1; n<=6; n++)
	{
        updateFontString(n);

        connect(lineEdit_formats[n], SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
        connect(offsets[n], SIGNAL(valueChanged(int)), this, SLOT(triggerRender()));
    }

    connect(ui->lineEdit_round, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_color, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_runID, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_boatID, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_runTime, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->checkBox_dsq, SIGNAL(toggled(bool)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_firstName1, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_lastName1, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_firstName2, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_lastName2, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));
    connect(ui->lineEdit_fireBrigade, SIGNAL(textChanged(QString)), this, SLOT(triggerRender()));

    triggerRender();
}

DialogDisplaySettings::~DialogDisplaySettings()
{
    delete ui;
}

QFont DialogDisplaySettings::getFont(int line)
{
    if(line>=1 && line <= 6){
        return fonts[line];
    }
    return QFont("Arial"); // default-font
}

QString DialogDisplaySettings::getFormat(int line)
{
    return lineEdit_formats[line]->text();
}

int DialogDisplaySettings::getOffset(int line)
{
    return offsets[line]->value();
}


void DialogDisplaySettings::fontDialog(int line)
{
	bool ok;
    QFont font = QFontDialog::getFont(&ok, fonts[line], this);

	if(ok)
	{
        fonts[line] = font;
        updateFontString(line);
    }
}

void DialogDisplaySettings::updateFontString(int line)
{
    QFont font = fonts[line];

    QString fontName;
    fontName.append(font.family());
    fontName.append(" ");
    fontName.append(QString::number(font.pointSize()));
    if(font.bold()){
        fontName.append(", fett");
    }
    if(font.italic()){
        fontName.append(tr(", kursiv"));
    }

    lineEdit_fonts[line]->setText(fontName);
    triggerRender();

//    qDebug()<<font.toString();
//    qDebug()<<font.key();
}

// build a RunData object from the sample-strings
void DialogDisplaySettings::buildRunData()
{
    runData->setRound(ui->lineEdit_round->text().toInt());
    runData->setColor(ui->lineEdit_color->text());
    runData->setID(ui->lineEdit_runID->text().toInt());
    runData->setBoatID(ui->lineEdit_boatID->text().toInt());
    runData->setStartTimeID(0);
    runData->setGoalTimeID(0);
    runData->setDsq(ui->checkBox_dsq->isChecked());
    runData->setFirstName1(ui->lineEdit_firstName1->text());
    runData->setLastName1(ui->lineEdit_lastName1->text());
    runData->setFirstName2(ui->lineEdit_firstName2->text());
    runData->setLastName2(ui->lineEdit_lastName2->text());
    runData->setFireBrigade(ui->lineEdit_fireBrigade->text());
}

void DialogDisplaySettings::on_pushButton_font1_clicked()
{
	fontDialog(1);
}

void DialogDisplaySettings::on_pushButton_font2_clicked()
{
	fontDialog(2);
}

void DialogDisplaySettings::on_pushButton_font3_clicked()
{
	fontDialog(3);
}

void DialogDisplaySettings::on_pushButton_font4_clicked()
{
	fontDialog(4);
}

void DialogDisplaySettings::on_pushButton_font5_clicked()
{
	fontDialog(5);
}

void DialogDisplaySettings::on_pushButton_font6_clicked()
{
    fontDialog(6);
}

// update the testdata runData, and send a signal to the renderer to render the runData
void DialogDisplaySettings::triggerRender()
{
    buildRunData();
    renderer->render(runData, false);
}

bool DialogDisplaySettings::getEnabled()
{
	return ui->checkBox_enableDisplays->isChecked();
}


void DialogDisplaySettings::on_pushButton_sendToDisplays_clicked()
{
    renderer->render(runData, true);
}

bool DialogDisplaySettings::getFlipDisplay(int displayNumber)
{
	if(displayNumber==1)
	{
		return ui->checkBox_flipDisplay1->isChecked();
	}
	else
	{
		return ui->checkBox_flipDisplay2->isChecked();
	}
}

DisplayRenderer* DialogDisplaySettings::getRenderer()
{
	return renderer;
}

void DialogDisplaySettings::on_buttonBox_accepted()
{
	// save settings
    QSettings set;
	set.beginGroup("display");
	for(int i=1; i<=6; i++)
	{
		set.setValue(QString("format%1").arg(i), lineEdit_formats[i]->text());
		set.setValue(QString("font%1").arg(i), fonts[i].toString());
		set.setValue(QString("offset%1").arg(i), offsets[i]->value());
	}
	set.setValue("enabled", ui->checkBox_enableDisplays->isChecked());
	set.setValue("flipDisplay1", ui->checkBox_flipDisplay1->isChecked());
	set.setValue("flipDisplay2", ui->checkBox_flipDisplay2->isChecked());
	
	set.endGroup();
	//qDebug() << "display settings saved"; 
}
