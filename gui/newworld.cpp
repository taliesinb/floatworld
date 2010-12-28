#include "newworld.hpp"
#include "widgets.hpp"
#include "mainwindow.hpp"
#include "ui_newworld.h"
#include <sstream>

ObjectListItem::ObjectListItem(Class* mc)
{
    number = 1;
    mclass = mc;
    UpdateLabel();
    prototype = mc->maker();
}

ObjectListItem::~ObjectListItem()
{
    delete prototype;
}

void ObjectListItem::SetNumber(int num)
{
    number = num;
    UpdateLabel();
}

void ObjectListItem::UpdateLabel()
{
    if (number == 0)
        setText(mclass->name);
    else
        setText(QString("%1 x %2").arg(number).arg(mclass->name));
}

NewWorldDialog::NewWorldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewWorldDialog)
{
    ui->setupUi(this);
    selected_object = NULL;

    for (int i = 0; i < Class::nmetaclasses; i++)
    {
        Class* c = Class::metaclasses[i];
        if (c->abstract) continue;
        std::cout << "Consdering class " << c->name << std::endl;
        for (Class* p = c; p; p = Class::Lookup(p->pname))
        {
            if (strcmp(p->name, "Block") == 0)
            {
                ui->prototypeList->addItem(c->name);
                break;
            }
            if (strcmp(p->name, "Shape") == 0)
            {
                ui->prototypeList->addItem(c->name);
                break;
            }
        }
    }
    ui->prototypeList->setCurrentRow(0);

    Creat* creat = new Creat();

    enum {
        energyF = 0, energyL, energyR, creatF, creatL, creatR, dirA, dirB,
        cons, energy, age, random,
        move = Creat::num_inputs + Creat::num_hidden,
        left,
        right,
        breed,
    };
    int offset = Creat::num_inputs;

    creat->weights(breed - offset, energy) = 0.75;
    creat->weights(move - offset, cons) = 1.1;
    creat->weights(left - offset, random) = 1.5;

    ObjectListItem* world = new ObjectListItem(Class::Lookup("World"));
    dynamic_cast<World*>(world->prototype)->SetSize(10,10);
    world->SetNumber(0);
    ui->objectTable->addItem(world);

    ObjectListItem* circle = new ObjectListItem(Class::Lookup("Circle"));
    circle->SetNumber(10);
    ui->objectTable->addItem(circle);

    ObjectListItem* adam = new ObjectListItem(Class::Lookup("Creat"));
    adam->prototype = creat;
    adam->SetNumber(40);
    ui->objectTable->addItem(adam);

    ui->objectTable->setDragDropMode(QAbstractItemView::InternalMove);

    connect(ui->copyObject, SIGNAL(released()), this, SLOT(CopyObject()));
    connect(ui->addObject, SIGNAL(released()), this, SLOT(AddObject()));
    connect(ui->removeObject, SIGNAL(released()), this, SLOT(RemoveObject()));
    connect(ui->objectTable, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(SelectObject(QListWidgetItem*)));
    connect(ui->numberBox, SIGNAL(valueChanged(int)), this, SLOT(SetObjectNumber(int)));
    connect(this, SIGNAL(accepted()), SLOT(CreateWorld()));
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
    if (dynamic_cast<World*>(CurrentItem()->prototype)) return;

    delete list.takeItem(list.currentRow());
    if (list.count() == 0)
        selected_object = NULL;
}

void NewWorldDialog::CopyObject()
{
    QListWidget& list = *ui->objectTable;

    if (ObjectListItem* item = CurrentItem())
    {
        if (dynamic_cast<World*>(item->prototype)) return;
        if (Occupant* occ = dynamic_cast<Occupant*>(item->prototype))
        {
            std::stringstream s;
            s << *occ;

            ObjectListItem* new_item = new ObjectListItem(&occ->GetClass());
            s >> *new_item->prototype;
            new_item->SetNumber(ui->numberBox->value());
            list.addItem(new_item);
            list.setCurrentItem(new_item);
        }
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

    if (item->number)
        ui->numberBox->setValue(item->number);
}

ObjectListItem* NewWorldDialog::CurrentItem()
{
    return dynamic_cast<ObjectListItem*>(ui->objectTable->currentItem());
}

void NewWorldDialog::SetObjectNumber(int number)
{
    ObjectListItem* item = CurrentItem();
    if (item && !dynamic_cast<World*>(item->prototype))
        item->SetNumber(number);
}

void NewWorldDialog::CreateWorld()
{
    MainWindow* mw = new MainWindow();

    World* w = NULL;
    for(int i = 0; i < ui->objectTable->count(); i++)
    {
        w = dynamic_cast<World*>(dynamic_cast<ObjectListItem*>(ui->objectTable->item(i))->prototype);
        if (w)
        {
            std::stringstream s2;
            s2 << *w;
            s2 >> *mw->world;
            break;
        }
    }

    int rows, cols;
    if (ui->radioSmallSize->isChecked()) rows = cols = 50;
    if (ui->radioMediumSize->isChecked()) rows = cols = 100;
    if (ui->radioLargeSize->isChecked()) rows = cols = 150;
    if (ui->radioCustomSize->isChecked()) {
        rows = ui->rowsBox->value();
        cols = ui->columnsBox->value();
    }

    mw->qworld->SetSize(rows, cols);

    ObjectListItem* item;
    for(int i = 0; i < ui->objectTable->count(); i++)
    {
        item = dynamic_cast<ObjectListItem*>(ui->objectTable->item(i));
        if (Creat* creat = dynamic_cast<Creat*>(item->prototype))
        {
            mw->world->initial_brain = &creat->weights;
            mw->world->AddCreats(item->number, true);
        } else {
            std::stringstream s;
            s << *item->prototype;
            for(int i = 0; i < item->number; i++)
            {
                std::stringstream s2(s.str());
                Occupant *occ = dynamic_cast<Occupant*>(Class::Create(s2));
                mw->world->Attach(occ, mw->world->RandomCell());
                mw->world->AssignID(occ);
                if (Shape* shape = dynamic_cast<Shape*>(occ))
                    for (int j = 0; j < 10; j++) shape->Draw(mw->world->energy);
            }
        }

    }

    mw->resize(5000,5000); // force a resize to the maximum size
    mw->qworld->Draw();
    mw->show();
}
