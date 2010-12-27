#include "newworld.hpp"
#include "widgets.hpp"
#include "ui_newworld.h"

ObjectListItem::ObjectListItem(Class* mc)
{
    number = 1;
    mclass = mc;
    UpdateLabel();
    prototype = dynamic_cast<Occupant*>(mc->maker());
}

ObjectListItem::~ObjectListItem()
{
    delete prototype;
}

void ObjectListItem::SetNumber(int num)
{
    UpdateLabel();
    number = num;
}

void ObjectListItem::UpdateLabel()
{
    setText(QString("%1 x %2").arg(number).arg(mclass->name));
}

NewWorldDialog::NewWorldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewWorldDialog)
{
    ui->setupUi(this);
    selected_object = NULL;

    QList<Class*> objects;
    for (int i = 0; i < Class::nmetaclasses; i++)
    {
        Class* c = Class::metaclasses[i];
        if (c->abstract) continue;
        for (Class* p = c; p; p = Class::Lookup(p->pname))
        {
            if (strcmp(p->name, "Block") == 0)
            {
                objects.push_back(c);
                break;
            }
            if (strcmp(p->name, "Shape") == 0)
            {
                objects.push_back(c);
                break;
            }
        }
    }
    foreach(Class* c, objects)
    {
        const char* name = c->name;
        ui->prototypeList->addItem(name);
    }
    ui->prototypeList->setCurrentRow(0);

    connect(ui->addObject, SIGNAL(released()), this, SLOT(AddObject()));
    connect(ui->removeObject, SIGNAL(released()), this, SLOT(RemoveObject()));
    connect(ui->objectTable, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(SelectObject(QListWidgetItem*)));
    connect(ui->numberBox, SIGNAL(valueChanged(int)), this, SLOT(SetObjectNumber(int)));
}

NewWorldDialog::~NewWorldDialog()
{
    delete ui;
}

void NewWorldDialog::AddObject()
{
    QListWidget& list = *ui->objectTable;
    QString type = ui->prototypeList->currentItem()->text();

    ObjectListItem* item = new ObjectListItem(Class::Lookup(type.toAscii()));
    item->SetNumber(ui->numberBox->value());
    list.addItem(item);
    list.setCurrentItem(item);
}

void NewWorldDialog::RemoveObject()
{
    QListWidget& list = *ui->objectTable;
    delete list.takeItem(list.currentRow());
    if (list.count() == 0)
    {
        selected_object = NULL;
    }
}

void NewWorldDialog::SelectObject(QListWidgetItem* _item)
{
    if (selected_object)
        selected_object->DeleteQtHook();

    if (!_item) return;

    ObjectListItem* item = dynamic_cast<ObjectListItem*>(_item);
    selected_object = item->prototype;

    selected_object->SetupQtHook(true);
    ui->objectPanel->setLayout(selected_object->panel);

    SetObjectNumber(item->number);
}

ObjectListItem* NewWorldDialog::CurrentItem()
{
    return dynamic_cast<ObjectListItem*>(ui->objectTable->currentItem());
}

void NewWorldDialog::SetObjectNumber(int number)
{
    if (ObjectListItem* item = CurrentItem())
        item->SetNumber(number);
}
