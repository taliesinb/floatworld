#include "mainwindow.hpp"

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

    Creat::mprofile.proba = 0.2 / Creat::nonzeroweights;
    Creat::mprofile.probb = 0.4 / Creat::nonzeroweights;
    Creat::mprofile.scale = 0.05;
    Creat::mprofile.noise = 0.05;
    Creat::drawspecial = false;
    Creat::drawoutline = false;
    grid.interaction = NoInteraction;
    Creat::mprofile.colordrift = true;

    // setup adam:
    grid.adam = &adam;
    adam.SetZero();
    enum {
        energyF = 0, energyL, energyR, creatF, creatL, creatR,
        cons, energy, age,
        move = Creat::inputs + Creat::hidden,
        left,
        right,
        breed,
    };
    adam(breed,cons) = -5.0;
    adam(breed,energy) = 1.0;
    adam(move,cons) = 1.0;

    for (int i = 0; i < 10; i++)
    {
        Circle* c = new Circle;
        c->Place(grid, grid.RandomCell());
        c->radius = 15;
        c->p_jump = 0.01;
    }

    grid.AddCreats(20, true);

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
    } else grid.Run(int(round(speed)));

    gridWidget->rerender();
    gridWidget->update();
}

