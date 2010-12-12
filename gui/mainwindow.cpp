#include "mainwindow.hpp"

#include <fstream>
#include <QFileDialog>
#include "qthooks.hpp"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    adam(Creat::hidden + Creat::outputs, Creat::neurons),
    speed(0), stepper(0), speed_multiplier(4)
{
    setupUi(this);

    grid = gridWidget->grid;

    // setup creatures
    Creat::Setup();

    grid->interaction_type = Wastage;

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

    for (int i = 0; i < 12; i++)
    {
        Circle* c = new Circle;
        c->Attach(*grid, grid->RandomCell());
        c->AssignID();
        c->radius = 12;
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

    for (int k = 0; k < 100; k++)
    {
        Occupant* block = new MoveableBlock();
        block->Attach(*grid, grid->RandomCell());
        block->AssignID();
    }

    connect(&timer, SIGNAL(timeout()), this, SLOT(takeStep()));
    timer.start(5);

    gridWidget->Draw();

    grid->SetupQtHook();
    gridBox->setLayout(grid->qt_hook);

    connect(grid->qt_hook, SIGNAL(value_changed()), gridWidget, SLOT(Draw()));
    connect(gridWidget, SIGNAL(OccupantSelected(Occupant*)), this, SLOT(DisplayInspector(Occupant*)));

    gridWidget->setMaximumSize(gridWidget->sizeHint()); // 30 30 is bad, it oscillates!
    resize(5000,5000); // force a resize to the maximum size
}


void MainWindow::on_actionPlay_triggered()
{
    speed = 1;
    this->actionPlay->setProperty("visible", false);
    this->actionStop->setProperty("visible", true);
}

void MainWindow::on_actionStop_triggered()
{
    speed = 0;
    this->actionPlay->setProperty("visible", true);
    this->actionStop->setProperty("visible", false);
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

void MainWindow::on_actionFast_triggered()
{
    speed *= speed_multiplier;
}

void MainWindow::on_actionSlow_triggered()
{
    speed /= speed_multiplier;
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,\
    "Save World as", QDir::homePath(), tr("Floatworlds (*.fw)"));

    if (fileName.size() > 0)
    {
        ofstream f;
        f.open(fileName.toUtf8());
        f << gridWidget->grid << endl;
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
        f >> *(gridWidget->grid);
        f.close();
    }
    grid->UpdateQtHook();
    gridWidget->Draw();
}

void MainWindow::takeStep()
{
    if (!speed) return;
    if (speed < 1)
    {
        float thresh = floor(stepper);
        stepper += speed;
        if (stepper > thresh + 0.9999)
        {
            stepper = 0;
            grid->Step();
        }
    } else grid->Run(int(round(speed)), 100);
    gridWidget->Draw();
}

void MainWindow::DisplayInspector(Occupant *occ)
{
    if (occ->qt_hook)
        occupantBox->setLayout(occ->qt_hook);
    gridWidget->Draw();
}
