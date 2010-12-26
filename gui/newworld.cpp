#include "newworld.hpp"
#include "ui_newworld.h"

NewWorldDialog::NewWorldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewWorldDialog)
{
    ui->setupUi(this);
}

NewWorldDialog::~NewWorldDialog()
{
    delete ui;
}
