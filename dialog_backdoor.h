#ifndef DIALOG_BACKDOOR_H
#define DIALOG_BACKDOOR_H

#include <QDialog>

namespace Ui {
class Dialog_BackDoor;
}

class Dialog_BackDoor : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_BackDoor(QWidget *parent = nullptr);
    ~Dialog_BackDoor();

    void setConnection();
    void setUI();
private:
    Ui::Dialog_BackDoor *ui;



public:
    QString path3_2;

};

#endif // DIALOG_BACKDOOR_H
