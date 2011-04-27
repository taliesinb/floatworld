#include "mainwindow.hpp"
#include "newworld.hpp"

#include <sstream>
#include <fstream>
#include <QFileDialog>
#include <QToolButton>
#include <QWidget>

MainWindow::MainWindow()
    : speed(0), stepper(0), last_stepper(0), block_draw(false),
    speed_group(this), selected_object(NULL)
{
    setupUi(this);

    world = qworld->world;

    ticker.setInterval(1);
    connect(&ticker, SIGNAL(timeout()), this, SLOT(Tick()));

    world->SetupQtHook(false);
    containerWorld->setLayout(world->panel);

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

    containerRendering->setLayout(qworld->panel);

    connect(qworld, SIGNAL(OccupantSelected(Occupant*)), this, SLOT(DisplayInspector(Occupant*)));

    // hunt out the fast forward button and hack some callbacks onto it
    QList<QWidget*> list = actionFF->associatedWidgets();
    foreach(QWidget* w, list)
    {
        QToolButton* button = dynamic_cast<QToolButton*>(w);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(ff_pressed()));
            connect(button, SIGNAL(released()), this, SLOT(ff_released()));
            button->setAutoRepeat(false);
        }
    }

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
    comboNewObjectClass->addItem("None");
    foreach(Class* c, objects)
    {
        const char* name = c->name;
        comboNewObjectClass->addItem(name);
    }
    comboNewObjectClass->setCurrentIndex(0);

    menuView->addAction(dockWorld->toggleViewAction());
    menuView->addAction(dockObjectInspector->toggleViewAction());
    menuView->addAction(dockObjectCreator->toggleViewAction());
    menuView->addAction(dockRendering->toggleViewAction());

    dockObjectCreator->close();
    dockRendering->close();

    connect(comboNewObjectClass, SIGNAL(activated(QString)), this, SLOT(ObjectSelected(QString)));
    connect(qworld, SIGNAL(CellClicked(Pos)), this, SLOT(CreateObjectAt(Pos)));
    update();
}

void MainWindow::showEvent(QShowEvent *)
{
    world->UpdateQtHook();
    qworld->UpdateQtHook();
    qworld->Draw();
    repaint();
    show();
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

    selected_object->SetupQtHook(false);
    containerObjectInspector->setLayout(selected_object->panel);
}

void MainWindow::CreateObjectAt(Pos pos)
{
    if (selected_object && !world->OccupantAt(pos))
    {
        std::stringstream s;
        s << *selected_object;
        Occupant *occ = dynamic_cast<Occupant*>(Class::Create(s));
        world->Attach(occ, pos, true);
        world->AssignID(occ);
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
    if (speed == s && s == 0) return; // already stopped
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
    if (world->num_creats == 0) Stop();
}

void MainWindow::ff_pressed()
{
    SetSpeed(16.0);
}

void MainWindow::ff_released()
{
    world->UpdateQtHook();
    speed_trigger(speed_group.checkedAction());
}

void MainWindow::Stop()
{
    actionStop->trigger();
}

void MainWindow::on_actionStep_triggered()
{
    Stop();

    std::ostringstream str;
    human_readable = false;
    str << *qworld->world;
    human_readable = true;
    QString string = str.str().c_str();
    world_cache.push_back(string);

    qworld->Step();    
    qworld->SetDrawFraction(1.0);
}

void MainWindow::on_actionStepBack_triggered()
{
    if (speed) return;

    if (world_cache.size()) {
        const char* str = world_cache.back().toAscii();
        std::istringstream s(str);
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
    Stop();
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
    QLinkedList<Occupant*>::iterator it = world->occupant_list.begin();
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
    world->energy.SetZero();
    qworld->Draw();
}

void MainWindow::on_actionDeleteSelected_triggered()
{
    if (qworld->selected_occupant)
        qworld->selected_occupant->Remove();
    qworld->Draw();
}

void MainWindow::on_actionNew_triggered()
{
    Stop();

    NewWorldDialog* dialog = new NewWorldDialog(this);
    const char* c = prototype.toAscii();
    std::istringstream s(c);
    s >> dialog;
    if (dialog->exec() == QDialog::Accepted)
    {
        this->deleteLater();
    }
}

void MainWindow::on_actionRestart_triggered()
{
    Stop();

    NewWorldDialog* dialog = new NewWorldDialog(this);
    const char* c = prototype.toAscii();
    std::istringstream s(c);
    s >> dialog;
    dialog->GetWorld()->rng.Seed(rand() % 65536);
    dialog->CreateWorld();
    this->deleteLater();
}

void MainWindow::on_actionSave_triggered()
{
    Stop();

    QString fileName = QFileDialog::getSaveFileName(this,\
    "Save World as", QDir::homePath(), tr("Floatworlds (*.fw)"));
    if (fileName.size() > 0)
    {
        std::ofstream f;
        f.open(fileName.toUtf8());
        f << *qworld->world << std::endl;
        f.close();
    }
}

void MainWindow::on_actionLoad_triggered()
{
    Stop();

    QString fileName = QFileDialog::getOpenFileName(this,\
    "Load World", QDir::homePath(), tr("Floatworlds (*.fw)"));

    if (fileName.size() > 0)
    {
        std::ifstream f;
        f.open(fileName.toUtf8());
        f >> *qworld->world;
        f.close();
    }
    world->UpdateQtHook();
    qworld->Draw();
}

void MainWindow::on_actionZoomIn_triggered()
{
    qworld->SetZoom(floor(qworld->CurrentZoom() * 1.2 + 1));
}

void MainWindow::on_actionZoomOut_triggered()
{
    int z = ceil((qworld->CurrentZoom() - 1) / 1.2);
    qworld->SetZoom(z < 2 ? 2 : z);
}

void MainWindow::on_buttonScatter_pressed()
{
    int num = numberScatter->value();
    for (int i = 0; i < num; i++)
    {
        CreateObjectAt(world->RandomCell());
    }
}

void MainWindow::DisplayInspector(Occupant *occ)
{
    if (occ->panel) containerObjectInspector->setLayout(occ->panel);
    qworld->Draw();
}
