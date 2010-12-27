#ifndef NEWWORLD_HPP
#define NEWWORLD_HPP

#include <QDialog>
#include <QString>
#include <QMap>
#include <QListWidgetItem>

#include "../src/world.hpp"

namespace Ui {
    class NewWorldDialog;
}

class ObjectListItem : public QListWidgetItem
{
public:
    int number;
    Occupant* prototype;
    Class* mclass;

    ObjectListItem(Class* mc);
    ~ObjectListItem();

    void UpdateLabel();
    void SetNumber(int num);
};

class NewWorldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewWorldDialog(QWidget *parent = 0);
    ~NewWorldDialog();
    Occupant* selected_object;

private:
    Ui::NewWorldDialog *ui;

public slots:
    void AddObject();
    void RemoveObject();
    void SetObjectNumber(QListWidgetItem*);
    void SelectObject(QListWidgetItem*);
};

#endif // NEWWORLD_HPP
