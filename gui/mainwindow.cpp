#include "mainwindow.hpp"

#include <fstream>
#include <QFileDialog>
#include "qthooks.hpp"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    adam(Creat::hidden + Creat::outputs, Creat::neurons),
    speed(0), stepper(0), speed_multiplier(4),
    selected_occupant(NULL),
    action_group(new QActionGroup(this))
{
    setupUi(this);

    grid = &gridWidget->grid;

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

    connect(grid->qt_hook, SIGNAL(value_changed()),
            this, SLOT(redraw()));
}

void MainWindow::cell_clicked(Pos pos)
{
    Occupant* occ = grid->OccupantAt(pos);

    if (occ)
    {
        SelectOccupant(occ);
    } else
    {
        float min_d = 10000;
        Occupant* occ = NULL;
        for_iterate(it, grid->occupant_list)
        {
            Occupant* o = *it;
            float d = (o->pos - pos).Mag();
            if (d < min_d)
            {
                min_d = d;
                occ = o;
            }
        }
        if (min_d < 3)
            SelectOccupant(occ);
        else
            grid->energy(pos) += 5;
    }
    redraw();
}

void MainWindow::unselect_occupant()
{
    selected_occupant = NULL;
}

void MainWindow::SelectOccupant(Occupant *occ)
{
    if (selected_occupant && occ != selected_occupant)
        selected_occupant->DeleteQtHook();

    if (occ && occ != selected_occupant)
    {
        // TODO: make sure s_o hasn't been freed,
        // which might happen if user deletes an
        // occupant

        selected_occupant = occ;

        HookManager* hm = occ->SetupQtHook();
        occupantBox->setLayout(hm);
        connect(hm, SIGNAL(value_changed()), this,
                SLOT(redraw()));
        connect(hm, SIGNAL(being_removed()), this,
                SLOT(unselect_occupant()));
    }
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
    redraw();
}

void MainWindow::on_actionIndividualStep_triggered()
{
    if (selected_occupant) {
        selected_occupant ->Update();
        redraw();
    }
}

void MainWindow::on_actionPrevOccupant_triggered()
{
    SelectNextOccupant(false);
    redraw();
}

void MainWindow::on_actionNextOccupant_triggered()
{
    SelectNextOccupant(true);
    redraw();
}

void MainWindow::SelectNextOccupant(bool forward)
{
    if (selected_occupant) {
        Pos p = selected_occupant->pos;
        int sz = grid->rows * grid->cols;
        int start = p.row * grid->cols + p.col;
        int index = start + (forward ? 1 : -1);
        do {
            Occupant* occ = grid->occupant_grid[index];
            if (occ)
            {
                SelectOccupant(occ);
                return;
            }
            index += (forward ? 1 : -1);
            index += sz;
            index %= sz;
        } while (index != start);

    } else {

        if (grid->occupant_list.size())
        {
            SelectOccupant(grid->occupant_list.front());
        }
    }
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
    grid->UpdateQtHook();
    redraw();
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
    redraw();
}

void MainWindow::redraw()
{
    if (selected_occupant)
        gridWidget->highlighted = selected_occupant->pos;
    gridWidget->Rerender();
    gridWidget->repaint();
}
