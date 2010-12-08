#include "mainwindow.hpp"

#include <fstream>
#include <QFileDialog>
#include "qthooks.hpp"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    adam(Creat::neurons, Creat::neurons),
    speed(0), stepper(0), speed_multiplier(4),
    action_group(new QActionGroup(this))
{
    setupUi(this);

    grid = &gridWidget->grid;

    connect( action_group, SIGNAL( selected( QAction* ) ), this, SLOT( viewtype_set( QAction* ) ) );

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

    adam(breed,energy) = 1.0;
    adam(move,cons) = 1.1;
    adam(left,random) = 1.05;

    grid->max_age = 120;

    for (int i = 0; i < 3; i++)
    {
        Circle* c = new Circle;
        c->Attach(*grid, grid->RandomCell());
        c->AssignID();
        c->radius = 12;
        c->threshold = 5;
        c->p_jump = 0.01;
        for (int k = 0; k < 10; k++) c->Update();
    }
/*
    for (int i = 0; i < 2; i++)
    {
        Circle* c = new Circle;
        c->Attach(grid, grid->RandomCell());
        c->radius = 23;
        c->energy = 0.25;
        c->p_jump = 0.005;
        for (int k = 0; k < 10; k++) c->Update();
    }

    for (int i = 0; i < 2; i++)
    {
        Circle* c = new Circle;
        c->Attach(grid, grid->RandomCell());
        c->radius = 8;
        c->energy = 2.0;
        c->p_jump = 0.01;
        for (int k = 0; k < 10; k++) c->Update();
    }
*/
    grid->energy_decay_rate = 0.08;
    grid->enable_respawn = true;
    grid->mutation_prob = 0.1;
    grid->path_energy = 0;

    grid->AddCreats(30, true);

    for (int k = 0; k < 12; k++)
    {
        Occupant* block = new SkinnerBlock();
        block->Attach(*grid, grid->RandomCell());
        block->AssignID();
    }

    connect(gridWidget, SIGNAL(ClickedCell(Pos)), this,
            SLOT(cell_clicked(Pos)));

    connect(&timer1, SIGNAL(timeout()), this, SLOT(takeStep()));
    timer1.start(5);
//    connect(&timer2, SIGNAL(timeout()), this, SLOT(reportFPS()));
//    timer2.start(5000);

    gridWidget->Rerender();
    gridWidget->repaint();

    grid->SetupQtHook();
    gridBox->setLayout(grid->qt_hook);
}

void MainWindow::cell_clicked(Pos pos)
{
    cout << "Clicked at pos " << pos << endl;
    Occupant* occ = grid->OccupantAt(pos);
    if (occ)
    {
        cout << "Setting to occ with id " << occ->id << endl;

        if (occupantBox->layout())
        {
            QLayoutItem *child;
            while ((child = occupantBox->layout()->takeAt(0)) != 0) {
                    child->widget()->deleteLater();
                    delete child;
            }
            HookManager* hm = dynamic_cast<HookManager*>(occupantBox->layout());
            hm->object->DeleteQtHook();
        }

        occupantBox->setLayout(occ->SetupQtHook());

    } else {
        grid->energy(pos) += 5;
    }
    gridWidget->Rerender();
    gridWidget->repaint();
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
    grid->Step();
    gridWidget->Rerender();
    gridWidget->repaint();
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
        f >> gridWidget->grid;
        f.close();
    }
}


void MainWindow::reportFPS()
{
    cout << gridWidget->render_count << endl;
    gridWidget->render_count = 0;
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
    } else grid->Run(int(round(speed)), 500);

    gridWidget->Rerender();
    gridWidget->update();
}
