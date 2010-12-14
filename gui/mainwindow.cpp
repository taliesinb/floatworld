#include "mainwindow.hpp"

#include <fstream>
#include <QFileDialog>
#include <QToolButton>
#include <QWidget>
#include "qthooks.hpp"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    adam(Creat::hidden + Creat::outputs, Creat::neurons),
    speed(0), stepper(0), last_stepper(0), block_draw(false),
    speed_group(this)
{
    setupUi(this);

    grid = gridWidget->grid;

    // setup creatures
    Creat::Setup();

    grid->interaction_type = Penalty;

    // setup adam:
    grid->initial_brain = &adam;
    adam.SetZero();
    enum {
        energyF = 0, energyL, energyR, creatF, creatL, creatR,
        cons, energy, age, random,
        move = Creat::inputs + Creat::hidden,
        left,
        right,
        breed,
    };
    int offset = Creat::inputs;
    adam(breed - offset, energy) = 1.0;
    adam(move - offset, cons) = 1.1;
    adam(left - offset, random) = 1.05;

    grid->max_age = 120;

    for (int i = 0; i < 15; i++)
    {
        Circle* c = new GaussianCircle;
        c->Attach(*grid, grid->RandomCell());
        c->AssignID();
        c->radius = 14;
        c->threshold = 7;
        c->p_jump = 0.01;
        for (int k = 0; k < 10; k++) c->Update();
    }

    grid->energy_decay_rate = 0.08;
    grid->enable_respawn = true;
    grid->mutation_prob = 0.1;
    grid->path_energy = 0;

    grid->AddCreats(30, true);

    for (int k = 0; k < 0; k++)
    {
        Occupant* block = new SkinnerBlock();
        block->Attach(*grid, grid->RandomCell());
        block->AssignID();
    }

    for (int k = 0; k < 0; k++)
    {
        Occupant* block = new ActiveTrap();
        block->Attach(*grid, grid->RandomCell());
        block->AssignID();
    }

    connect(&timer, SIGNAL(timeout()), this, SLOT(takeStep()));
    connect(&fast_timer, SIGNAL(timeout()), this, SLOT(calculateStep()));
    timer.start(40); // 25 fps

    gridWidget->Draw();

    grid->SetupQtHook(false);
    gridBox->setLayout(grid->qt_hook);

    speed_group.addAction(actionPlaySlowest);
    speed_group.addAction(actionPlaySlow);
    speed_group.addAction(actionPlayNormal);
    speed_group.addAction(actionPlayFast);
    speed_group.addAction(actionPlayFastest);
    speed_group.addAction(actionStop);
    actionStop->setChecked(true);
    speed_group.setExclusive(true);
    connect(&speed_group, SIGNAL(triggered(QAction*)), this, SLOT(speed_trigger(QAction*)));

    connect(grid->qt_hook, SIGNAL(value_changed()), gridWidget, SLOT(Draw()));
    connect(gridWidget, SIGNAL(OccupantSelected(Occupant*)), this, SLOT(DisplayInspector(Occupant*)));

    gridWidget->setMaximumSize(gridWidget->sizeHint());
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
    gridWidget->Draw();
    repaint();
}

void MainWindow::SetSpeed(float s)
{
    if (speed == s)
        actionStop->trigger();
    else
    {
        speed = s;
        if (s >= 1)
            fast_timer.start(1);
        else if (fast_timer.isActive())
            fast_timer.stop();
    }
    gridWidget->Draw();
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
        fast_timer.stop();
    }
}

void MainWindow::ff_pressed()
{
    //block_draw = true;
    SetSpeed(16.0);
}

void MainWindow::ff_released()
{
    //block_draw = false;
    grid->UpdateQtHook();
    speed_trigger(speed_group.checkedAction());
}

void MainWindow::calculateStep()
{
    grid->hooks_enabled = false;
    grid->Step();
    if (stepper++ > last_stepper + speed)
        fast_timer.stop();
    grid->hooks_enabled = true;
}

void MainWindow::takeStep()
{
    if (!speed) return;

    if (stepper >= ceil(last_stepper + speed))
    {
        last_stepper = stepper;
        stepper += speed;
        gridWidget->SetDrawFraction(speed < 1 ? 0.0 : 1.0);
        grid->Step();
        if (!block_draw) { // for fast forwarding
            gridWidget->Draw();
            grid->UpdateQtHook();
        }
    } else if (speed < 1)
    {
        gridWidget->SetDrawFraction(stepper - floor(stepper));
        gridWidget->Draw();
    }

    if (speed < 1)
        stepper += speed;
    else if (!fast_timer.isActive())
        fast_timer.start(1);
}


void MainWindow::on_actionStep_triggered()
{
    gridWidget->Step();
}

void MainWindow::on_actionIndividualStep_triggered()
{
    gridWidget->UpdateOccupant();
}

void MainWindow::on_actionPrevOccupant_triggered()
{
    gridWidget->SelectNextOccupant(false);
}

void MainWindow::on_actionNextOccupant_triggered()
{
    gridWidget->SelectNextOccupant(true);
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,\
    "Save World as", QDir::homePath(), tr("Floatworlds (*.fw)"));

    if (fileName.size() > 0)
    {
        ofstream f;
        f.open(fileName.toUtf8());
        f << *gridWidget->grid << endl;
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
        f >> *gridWidget->grid;
        f.close();
    }
    grid->UpdateQtHook();
    gridWidget->Draw();
}

void MainWindow::on_actionZoomIn_triggered()
{
    gridWidget->SetZoom(gridWidget->CurrentZoom() + 1);
}

void MainWindow::on_actionZoomOut_triggered()
{
    int z = gridWidget->CurrentZoom() - 1;
    gridWidget->SetZoom(z < 2 ? 2 : z);
}

void MainWindow::DisplayInspector(Occupant *occ)
{
    if (occ->qt_hook)
        occupantBox->setLayout(occ->qt_hook);
    gridWidget->Draw();
}
