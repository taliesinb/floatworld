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

    for (int i = 0; i < 8; i++)
    {
        Circle* c = new Circle;
        c->Attach(*grid, grid->RandomCell());
        c->AssignID();
        c->radius = RandInt(8,20);
        c->threshold = 7;
        c->p_jump = 0.002;
        c->ratio = 0.1;
        for (int k = 0; k < 10; k++) c->Update();
    }

    grid->energy_decay_rate = 0.08;
    grid->enable_respawn = true;
    grid->mutation_prob = 0.1;
    grid->path_energy = 0;

    grid->AddCreats(30, true);

    for (int k = 0; k < 0; k++)
    {
        Occupant* block = new RewardBlock();
        block->Attach(*grid, grid->RandomCell());
        block->AssignID();
    }

    for (int k = 0; k < 0; k++)
    {
        Occupant* block = new ActiveTrap();
        block->Attach(*grid, grid->RandomCell());
        block->AssignID();
    }

    ticker.setInterval(1);
    connect(&ticker, SIGNAL(timeout()), this, SLOT(Tick()));

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
    actionFF->setAutoRepeat(false);

    renderSettingsBox->setLayout(gridWidget->qt_hook);

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
        gridWidget->SetDrawFraction(1.0);
        gridWidget->Draw();
    }
}

void MainWindow::SetSpeed(float s)
{
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
        grid->hooks_enabled = false;
        for (int i = 0; i < speed * 2; i++)
        {
            grid->Step();
            if (timer.elapsed() > 100) break;
        }
        grid->hooks_enabled = true;
        grid->Step();
    } else {
        stepper += speed;
        if (stepper >= last_stepper + 1)
        {
            stepper = last_stepper + 1;
            last_stepper = stepper;
            grid->Step();
        }
    }
    gridWidget->SetDrawFraction(speed > 1 ? 1.0 : (stepper - floor(stepper)));
    gridWidget->Draw();
    grid->UpdateQtHook();
}

void MainWindow::ff_pressed()
{
    SetSpeed(8.0);
}

void MainWindow::ff_released()
{
    grid->UpdateQtHook();
    speed_trigger(speed_group.checkedAction());
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
