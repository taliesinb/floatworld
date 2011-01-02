#include "newworld.hpp"
#include "widgets.hpp"
#include "mainwindow.hpp"
#include "ui_newworld.h"
#include "src/metaclass.hpp"

#include <fstream>
#include <sstream>
#include <QFileDialog>

ObjectListItem::ObjectListItem()
{
    number = 0;
    prototype = NULL;
}

ObjectListItem::ObjectListItem(Class* mc)
{
    number = 1;
    prototype = mc->maker();
    UpdateLabel();
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
        setText(prototype->Name());
    else
        setText(QString("%2 (%1)").arg(number).arg(prototype->Name()));
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

    ui->objectTable->setDragDropMode(QAbstractItemView::InternalMove);

    ui->splitter->setStretchFactor(0,3);
    ui->splitter->setStretchFactor(1,2);
    ui->objectTable->setFocus();

    ui->containerObject->setLayout(new QGridLayout);

    ui->commentBox->setVisible(false);

    connect(ui->objectTable, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), ui->prototypeList, SLOT(clearSelection()));
    connect(ui->prototypeList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(DeselectObject()));
    connect(ui->saveTemplate, SIGNAL(released()), this, SLOT(SaveTemplate()));
    connect(ui->loadTemplate, SIGNAL(released()), this, SLOT(LoadTemplate()));
    connect(ui->copyObject, SIGNAL(released()), this, SLOT(CopyObject()));
    connect(ui->addObject, SIGNAL(released()), this, SLOT(AddObject()));
    connect(ui->removeObject, SIGNAL(released()), this, SLOT(RemoveObject()));
    connect(ui->objectTable, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(SelectObject(QListWidgetItem*)));
    connect(ui->numberBox, SIGNAL(valueChanged(int)), this, SLOT(SetObjectNumber(int)));
    connect(ui->commentVisibleBox, SIGNAL(toggled(bool)), ui->commentBox, SLOT(setVisible(bool)));
    connect(this, SIGNAL(accepted()), SLOT(CreateWorld()));
}

void NewWorldDialog::WorldSizeChanged()
{
    int rows, cols;
    if (ui->radioSmallSize->isChecked()) rows = cols = 50;
    if (ui->radioMediumSize->isChecked()) rows = cols = 100;
    if (ui->radioLargeSize->isChecked()) rows = cols = 150;
    if (ui->radioCustomSize->isChecked()) {
        rows = ui->rowsBox->value();
        cols = ui->columnsBox->value();
    }
    GetWorld()->SetSize(rows, cols);
}

void NewWorldDialog::CreateDefaultObjects()
{
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
    world->SetNumber(0);
    ui->objectTable->addItem(world);
    ui->radioSmallSize->setChecked(true);
    WorldSizeChanged();

    ObjectListItem* circle = new ObjectListItem(Class::Lookup("EnergyDisk"));
    circle->SetNumber(5);
    ui->objectTable->addItem(circle);

    ObjectListItem* adam = new ObjectListItem(Class::Lookup("Creat"));
    adam->prototype = creat;
    adam->SetNumber(40);
    ui->objectTable->addItem(adam);

}

NewWorldDialog::~NewWorldDialog()
{
    delete ui;
}

std::ostream& operator<<(std::ostream& os, ObjectListItem* & item)
{
    os << item->number << ": ";
    return os << *item->prototype;
}

std::istream& operator>>(std::istream& is, ObjectListItem* & item)
{
    item = new ObjectListItem;
    is >> item->number >> ": ";
    item->prototype = Class::Create(is);
    return is;
}

QLinkedList<ObjectListItem*> NewWorldDialog::AllItems()
{
    QLinkedList<ObjectListItem*> items;
    for (int i = 0; i < ui->objectTable->count(); i++)
        items.push_back(dynamic_cast<ObjectListItem*>(ui->objectTable->item(i)));
    return items;
}

void NewWorldDialog::SaveTemplate()
{
    std::ofstream f;

    QString fileName = QFileDialog::getSaveFileName(this,\
    "Save prototype as", QDir::homePath(), tr("Floatworld prototypes (*.proto)"));
    if (fileName.size() <= 0) return;

    f.open(fileName.toUtf8());
    f << this;
    f.close();
}

void NewWorldDialog::LoadTemplate()
{
    std::ifstream f;

    QString fileName = QFileDialog::getOpenFileName(this,\
    "Load prototype", QDir::homePath(), tr("Floatworld prototypes (*.proto)"));
    if (fileName.size() <= 0) return;

    f.open(fileName.toUtf8());
    f >> this;
    f.close();
}

World* NewWorldDialog::GetWorld()
{
    foreach(ObjectListItem* item, AllItems())
        if (World* world = dynamic_cast<World*>(item->prototype)) return world;
    return NULL;
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
        selected_object->DeletePanel();

    if (!_item) return;

    ObjectListItem* item = dynamic_cast<ObjectListItem*>(_item);
    selected_object = item->prototype;

    selected_object->SetupPanel(true);
    ui->containerObject->layout()->addWidget(selected_object->panel);

    if (item->number)
        ui->numberBox->setValue(item->number);
}

void NewWorldDialog::DeselectObject()
{
    SelectObject(NULL);
    ui->objectTable->setCurrentRow(-1);
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
    WorldSizeChanged();

    MainWindow* mw = new MainWindow();

    World* w = GetWorld();
    std::stringstream s2;
    s2 << *w;
    s2 >> *mw->world;

    mw->qworld->SetSize(w->rows, w->cols);

    foreach(ObjectListItem* item, AllItems())
    {
        if (Creat* creat = dynamic_cast<Creat*>(item->prototype))
        {
            mw->world->initial_brain = &creat->weights;
            mw->world->AddCreats(item->number, true);
        } else if (dynamic_cast<Occupant*>(item->prototype))
        {
            std::stringstream s;
            s << *item->prototype;
            for(int i = 0; i < item->number; i++)
            {
                std::stringstream s2(s.str());
                Occupant *occ = dynamic_cast<Occupant*>(Class::Create(s2));
                mw->world->Attach(occ, mw->world->RandomCell(), true);
                mw->world->AssignID(occ);
                if (Shape* shape = dynamic_cast<Shape*>(occ))
                    for (int j = 0; j < 10; j++) shape->Draw(mw->world->energy);
            }
        }
    }

    std::ostringstream os;
    os << this;
    mw->prototype = os.str().c_str();

    //mw->resize(5000,5000); // force a resize to the maximum size
    QSize sz = mw->sizeHint();
    mw->resize(sz.width(), sz.height());
    mw->qworld->Draw();
    mw->show();
    mw->update();
}

std::ostream& operator<<(std::ostream& s, NewWorldDialog* d)
{
    d->WorldSizeChanged();

    QString str = d->ui->commentBox->toPlainText();
    s << "\"comment\": " << str << std::endl;

    QLinkedList<ObjectListItem*> items = d->AllItems();
    s << items;
    s << std::endl;
    return s;
}

std::istream& operator>>(std::istream& s, NewWorldDialog* d)
{
    d->ui->objectTable->clear();
    d->ui->commentBox->clear();

    QString str;
    s >> "\"comment\": " >> str >> whitespace;
    d->ui->commentBox->setPlainText(str);

    QLinkedList<ObjectListItem*> items;
    s >> items;

    d->selected_object = NULL;
    foreach(ObjectListItem* item, items)
    {
        d->ui->objectTable->addItem(item);
        item->UpdateLabel();
    }

    World* w = d->GetWorld();
    int r = w->rows, c = w->cols;
    if      (r == 50  && c == 50)  d->ui->radioSmallSize->setChecked(true);
    else if (r == 100 && c == 100) d->ui->radioMediumSize->setChecked(true);
    else if (r == 150 && c == 150) d->ui->radioLargeSize->setChecked(true);
    else {
        d->ui->radioCustomSize->setChecked(true);
        d->ui->rowsBox->setValue(r);
        d->ui->columnsBox->setValue(c);
    }

    d->ui->commentVisibleBox->setChecked(d->ui->commentBox->toPlainText().length() > 0);

    return s;
}


