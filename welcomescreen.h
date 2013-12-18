#ifndef WELCOMESCREEN_H
#define WELCOMESCREEN_H

#include <QDialog>

namespace Ui {
class WelcomeScreen;
}

class WelcomeScreen : public QDialog
{
    Q_OBJECT
    
public:
    explicit WelcomeScreen(QWidget *parent = 0);
    ~WelcomeScreen();
    
private slots:
    void on_pushButton_newCompetition_clicked();

    void on_pushButton_viewAndEditRecent_clicked();

    void on_pushButton_continueOther_clicked();

    void on_pushButton_viewAndEditOther_clicked();

private:
    Ui::WelcomeScreen *ui;
};

#endif // WELCOMESCREEN_H
