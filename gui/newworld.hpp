#ifndef NEWWORLD_HPP
#define NEWWORLD_HPP

#include <QDialog>

namespace Ui {
    class NewWorldDialog;
}

class NewWorldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewWorldDialog(QWidget *parent = 0);
    ~NewWorldDialog();

private:
    Ui::NewWorldDialog *ui;
};

#endif // NEWWORLD_HPP
