#ifndef DIALOGCSVUPLOADCOMMAND_H
#define DIALOGCSVUPLOADCOMMAND_H

#include <QDialog>
#include <QString>
#include <QStringList>

namespace Ui {
class DialogCsvUploadCommand;
}

class DialogCsvUploadCommand : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogCsvUploadCommand(QWidget *parent = 0);
    ~DialogCsvUploadCommand();
    QString commandText();
    QStringList parameters();
    
private:
    Ui::DialogCsvUploadCommand *ui;
};

#endif // DIALOGCSVUPLOADCOMMAND_H
