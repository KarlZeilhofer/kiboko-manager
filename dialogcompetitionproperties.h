#ifndef DIALOGCOMPETITIONPROPERTIES_H
#define DIALOGCOMPETITIONPROPERTIES_H

#include <QDialog>

namespace Ui {
class DialogCompetitionProperties;
}

class DialogCompetitionProperties : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogCompetitionProperties(QWidget *parent = 0);
    ~DialogCompetitionProperties();
    
private slots:
	
	
private:
	
	void closeEvent(QCloseEvent *event);
    Ui::DialogCompetitionProperties *ui;
};

#endif // DIALOGCOMPETITIONPROPERTIES_H
