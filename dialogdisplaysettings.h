#ifndef DIALOGDISPLAYSETTINGS_H
#define DIALOGDISPLAYSETTINGS_H

#include <QDialog>
#include <QFont>
#include <QLineEdit>
#include <QSpinBox>

class DisplayRenderer;
class RunData;


namespace Ui {
    class DialogDisplaySettings;
}

class DialogDisplaySettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDisplaySettings(QWidget *parent = 0);
    ~DialogDisplaySettings();
    QFont getFont(int line);
    QString getFormat(int line);
    int getOffset(int line);
	bool getEnabled();
	bool getFlipDisplay(int displayNumber);
	DisplayRenderer* getRenderer();

private:
    void fontDialog(int line);
    void updateFontString(int line);
    void buildRunData();


private slots:
	void on_pushButton_font1_clicked();
    void on_pushButton_font2_clicked();
	void on_pushButton_font3_clicked();
	void on_pushButton_font4_clicked();
	void on_pushButton_font5_clicked();
	void on_pushButton_font6_clicked();
    void triggerRender();
	void on_pushButton_sendToDisplays_clicked();
	void on_buttonBox_accepted();
	
private:
    Ui::DialogDisplaySettings *ui;
    QFont fonts[7];
	QLineEdit* lineEdit_fonts[7]; // start with index 1
	QLineEdit* lineEdit_formats[7]; // start with index 1
    QSpinBox* offsets[7];
    DisplayRenderer* renderer;
    RunData* runData;
};

#endif // DIALOGDISPLAYSETTINGS_H
