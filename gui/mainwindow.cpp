#include "mainwindow.hpp"

#include <sstream>
#include <fstream>
#include <QFileDialog>
#include <QToolButton>
#include <QWidget>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    adam(Creat::num_hidden + Creat::num_outputs, Creat::num_neurons),
    speed(0), stepper(0), last_stepper(0), block_draw(false),
    speed_group(this), selected_object(NULL)
{
    setupUi(this);

    world = qworld->world;

    // setup creatures
    Creat::Setup();

    world->interaction_type = Penalty;

    // setup adam:
    world->initial_brain = &adam;
    adam.SetZero();
    enum {
        energyF = 0, energyL, energyR, creatF, creatL, creatR, dirA, dirB,
        cons, energy, age, random,
        move = Creat::num_inputs + Creat::num_hidden,
        left,
        right,
        breed,
    };
    int offset = Creat::num_inputs;
    adam(breed - offset, energy) = 0.75;
    adam(move - offset, cons) = 1.1;
    adam(left - offset, random) = 1.5;

    world->max_age = 50;

    for (int i = 0; i < 15; i++)
    {
        Circle* c = new Circle;
        c->Attach(*world, world->RandomCell());
        c->AssignID();
        c->radius = RandInt(5,17);
        c->threshold = 2 + max(25 - (c->radius - 5) * (c->radius - 5) / 3, 0);
        c->energy = 2;
        c->p_jump = 0.01;
        //c->ratio = 0.2;
        for (int k = 0; k < 10; k++) c->Update();
    }

    world->energy_decay_rate = 0.08;
    world->enable_respawn = true;
    world->mutation_prob = 0.2;
    world->path_energy = 0;

    world->AddCreats(300, true);

    for (int k = 0; k < 0; k++)
    {
        Occupant* block = new SkinnerBlock();
        block->Attach(*world, world->RandomCell());
        block->AssignID();
    }

    for (int k = 0; k < 0; k++)
    {
        Occupant* block = new ActiveTrap();
        block->Attach(*world, world->RandomCell());
        block->AssignID();
    }

    ticker.setInterval(1);
    connect(&ticker, SIGNAL(timeout()), this, SLOT(Tick()));

    world->interaction_type = Attack;
    qworld->Draw();

    world->SetupQtHook(false);
    gridBox->setLayout(world->panel);

    speed_group.addAction(actionPlaySlowest);
    speed_group.addAction(actionPlaySlow);
    speed_group.addAction(actionPlayNormal);
    speed_group.addAction(actionPlayFast);
    speed_group.addAction(actionPlayFastest);
    speed_group.addAction(actionStop);
    actionStop->setChecked(true);
    speed_group.setExclusive(true);
    connect(&speed_group, SIGNAL(triggered(QAction*)), this, SLOT(speed_trigger(QAction*)));
    actionFF->setAutoRepeat(false);

    renderSettingsBox->setLayout(qworld->panel);

    connect(qworld, SIGNAL(OccupantSelected(Occupant*)), this, SLOT(DisplayInspector(Occupant*)));

    qworld->setMaximumSize(qworld->sizeHint());
    resize(5000,5000); // force a resize to the maximum size

    // hunt out the fast forward button and hack some callbacks onto it
    QList<QWidget*> list = actionFF->associatedWidgets();
    for_iterate(it, list)
    {
        QToolButton* button = dynamic_cast<QToolButton*>(*it);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(ff_pressed()));
            connect(button, SIGNAL(released()), this, SLOT(ff_released()));
            button->setAutoRepeat(false);
        }
    }
    qworld->Draw();

    std::list<Class*> objects;
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
    objectComboBox->addItem("None");
    for_iterate(it2, objects)
    {
        const char* name = (*it2)->name;
        objectComboBox->addItem(name);
    }
    objectComboBox->setCurrentIndex(0);
    connect(objectComboBox, SIGNAL(activated(QString)), this, SLOT(ObjectSelected(QString)));
    connect(qworld, SIGNAL(CellClicked(Pos)), this, SLOT(CreateObjectAt(Pos)));
    update();
}

void MainWindow::ObjectSelected(QString s)
{   
    if (selected_object)
        selected_object->DeleteQtHook();

    if (s == "None")
    {
        selected_object = NULL;
        return;
    }

    if (prototypes.contains(s))
        selected_object = prototypes[s];
    else
        selected_object = prototypes[s] = dynamic_cast<Occupant*>(Class::MakeNew(s.toAscii()));

    selected_object->SetupQtHook(true);
    objectPanel->setLayout(selected_object->panel);
}

void MainWindow::CreateObjectAt(Pos pos)
{
    if (selected_object && !world->OccupantAt(pos))
    {
        stringstream s;
        s << *selected_object;
        Occupant *occ = dynamic_cast<Occupant*>(Class::Create(s));
        occ->Attach(*world, pos);
        occ->AssignID();
    }
    qworld->Draw();
}

void MainWindow::speed_trigger(QAction* action)
{
    if (action == actionPlaySlowest) SetSpeed(0.05);
    if (action == actionPlaySlow) SetSpeed(0.2);
    if (action == actionPlayNormal) SetSpeed(0.5);
    if (action == actionPlayFast) SetSpeed(2.0);
    if (action == actionPlayFastest) SetSpeed(8.0);
    if (action == actionStop) {
        speed = 0;
        ticker.stop();
        stepper = ceil(stepper);
        qworld->SetDrawFraction(1.0);
        qworld->Draw();
    }
}

void MainWindow::SetSpeed(float s)
{
    if (world_cache.size()) world_cache.clear();
    if (speed == s)
        actionStop->trigger();
    else
    {
        speed = s;
        if (speed < 1) ticker.start(40.0);
        else if (speed == 2) ticker.start(5.0);
        else ticker.start(0);
    }
}

void MainWindow::Tick()
{
    if (speed > 2)
    {
        timer.start();
        world->hooks_enabled = false;
        for (int i = 0; i < speed * 2; i++)
        {
            world->Step();
            if (timer.elapsed() > 100) break;
        }
        world->hooks_enabled = true;
        world->Step();
    } else {
        stepper += speed;
        if (stepper >= last_stepper + 1)
        {
            stepper = last_stepper + 1;
            last_stepper = stepper;
            world->Step();
        }
    }
    qworld->SetDrawFraction(speed > 1 ? 1.0 : (stepper - floor(stepper)));
    qworld->Draw();
    world->UpdateQtHook();
}

void MainWindow::ff_pressed()
{
    SetSpeed(8.0);
}

void MainWindow::ff_released()
{
    world->UpdateQtHook();
    speed_trigger(speed_group.checkedAction());
}
void MainWindow::on_actionStep_triggered()
{
    ostringstream str;
    human_readable = false;
    str << *qworld->world;
    human_readable = true;
    world_cache.push_back(str.str());
    qworld->Step();    
    qworld->SetDrawFraction(1.0);
}

void MainWindow::on_actionStepBack_triggered()
{
    if (world_cache.size()) {
        istringstream s(world_cache.back());
        human_readable = false;
        int id = qworld->selected_occupant ? qworld->selected_occupant->id : -1;
        s >> *qworld->world;
        human_readable = true;
        world_cache.pop_back();
        world->UpdateQtHook();
        if (id > 0)
            qworld->SelectOccupant(qworld->world->LookupOccupantByID(id));
        qworld->SetDrawFraction(1.0);
        qworld->Draw();
    }
}

void MainWindow::on_actionIndividualStep_triggered()
{
    qworld->UpdateOccupant();
}

void MainWindow::on_actionPrevOccupant_triggered()
{
    qworld->SelectNextOccupant(false);
}

void MainWindow::on_actionNextOccupant_triggered()
{
    qworld->SelectNextOccupant(true);
}

void MainWindow::on_actionClearCreats_triggered()
{
    std::list<Occupant*>::iterator it = world->occupant_list.begin();
    while (it != world->occupant_list.end())
    {
        Creat* creat = dynamic_cast<Creat*>(*it++);
        if (creat) creat->Remove();
    }
    qworld->Draw();
}

void MainWindow::on_actionClearAll_triggered()
{
    while (world->occupant_list.size())
        world->occupant_list.front()->Remove();
    qworld->Draw();
}

void MainWindow::on_actionDeleteSelected_triggered()
{
    if (qworld->selected_occupant)
        qworld->selected_occupant->Remove();
    qworld->Draw();
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,\
    "Save World as", QDir::homePath(), tr("Floatworlds (*.fw)"));
    if (fileName.size() > 0)
    {
        ofstream f;
        f.open(fileName.toUtf8());
        f << *qworld->world << endl;
        f.close();
    }
}

void MainWindow::on_actionLoad_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,\
    "Load World", QDir::homePath(), tr("Floatworlds (*.fw)"));

    if (fileName.size() > 0)
    {
        ifstream f;
        f.open(fileName.toUtf8());
        f >> *qworld->world;
        f.close();
    }
    world->UpdateQtHook();
    qworld->Draw();
}

void MainWindow::on_actionZoomIn_triggered()
{
    qworld->SetZoom(qworld->CurrentZoom() + 1);
}

void MainWindow::on_actionZoomOut_triggered()
{
    int z = qworld->CurrentZoom() - 1;
    qworld->SetZoom(z < 2 ? 2 : z);
}

void MainWindow::DisplayInspector(Occupant *occ)
{
    if (occ->panel)
        occupantBox->setLayout(occ->panel);
    qworld->Draw();
}
