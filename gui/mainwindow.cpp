#include "mainwindow.hpp"
#include <fstream>

using namespace std;

void MainWindow::viewtype_set(QAction* action)
{
    if (action == actionAge) gridWidget->draw_type = GridWidget::draw_age;
    if (action == actionEnergy) gridWidget->draw_type = GridWidget::draw_energy;
    if (action == actionPlain) gridWidget->draw_type = GridWidget::draw_plain;
    if (action == actionPlumage) gridWidget->draw_type = GridWidget::draw_color;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    adam(Creat::neurons, Creat::neurons),
    speed(0), stepper(0), speed_multiplier(4),
    action_group(new QActionGroup(this))
{
    setupUi(this);

    action_group->addAction(actionAge);
    action_group->addAction(actionEnergy);
    action_group->addAction(actionPlain);
    action_group->addAction(actionPlumage);

    connect( action_group, SIGNAL( selected( QAction* ) ), this, SLOT( viewtype_set( QAction* ) ) );

    // shortcut
    Grid& grid = gridWidget->grid;

    // setup creatures
    Creat::Setup();

    grid.interaction_type = Wastage;

    // setup adam:
    grid.initial_brain = &adam;
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

    grid.max_age = 120;

    for (int i = 0; i < 3; i++)
    {
        Circle* c = new Circle;
        c->Place(grid, grid.RandomCell());
        c->radius = 16;
        c->p_jump = 0.01;
        for (int k = 0; k < 10; k++) c->Update();
    }

    for (int i = 0; i < 3; i++)
    {
        Circle* c = new Circle;
        c->Place(grid, grid.RandomCell());
        c->radius = 23;
        c->energy = 0.25;
        c->p_jump = 0.005;
        for (int k = 0; k < 10; k++) c->Update();
    }

    for (int i = 0; i < 2; i++)
    {
        Circle* c = new Circle;
        c->Place(grid, grid.RandomCell());
        c->radius = 8;
        c->energy = 3.0;
        c->p_jump = 0.01;
        for (int k = 0; k < 10; k++) c->Update();
    }

    grid.energy_decay_rate = 0.08;
    grid.enable_respawn = true;
    grid.mutation_prob = 0.1;
    grid.path_energy = 0;

    grid.AddCreats(40, true);

    for (int k = 0; k < 50; k++)
    {
    Occupant* block = new SkinnerBlock();
    block->Place(grid, grid.RandomCell());
    }

    ofstream of;
    of.open("/Users/tali/test.txt");
    of << grid.creats[0] << endl;
    of.close();

    Creat creat;
    ifstream inf;
    inf.open("/Users/tali/test.txt");
    inf >> creat;
    inf.close();

    ofstream of2;
    of2.open("/Users/tali/test2.txt");
    of2 << creat << endl;
    of2.close();

    connect(&timer1, SIGNAL(timeout()), this, SLOT(takeStep()));
    timer1.start(5);
//    connect(&timer2, SIGNAL(timeout()), this, SLOT(reportFPS()));
//    timer2.start(5000);

    gridWidget->rerender();
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
    gridWidget->grid.Step();
    gridWidget->rerender();
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

void MainWindow::reportFPS()
{
    cout << gridWidget->renders << endl;
    gridWidget->renders = 0;
}

void MainWindow::takeStep()
{
    if (!speed) return;
    Grid& grid = gridWidget->grid;
    if (speed < 1)
    {
        float thresh = floor(stepper);
        stepper += speed;
        if (stepper > thresh + 0.9999)
        {
            stepper = 0;
            grid.Step();
        }
    } else grid.Run(int(round(speed)), 500);

    gridWidget->rerender();
    gridWidget->update();
}

