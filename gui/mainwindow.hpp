#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtGui/QMainWindow>
#include <QtGui/QGraphicsView>
#include <QTimer>

#include "../src/misc.hpp"
#include "../src/pos.hpp"
#include "../src/matrix.hpp"
#include "../src/grid.hpp"
#include "../src/shape.hpp"
#include "../src/block.hpp"

#include "ui_mainwindow.h"
#include "widgets.hpp"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    Matrix adam;
    QTimer timer;
    QTimer fast_timer;
    float speed;
    float stepper;
    float last_stepper;
    Grid* grid;
    MainWindow(QWidget *parent = 0);

    void SetSpeed(float speed);

private slots:

    void on_actionPlay1_4_triggered();
    void on_actionPlay1_2_triggered();
    void on_actionPlay1_triggered();
    void on_actionPlay2_triggered();
    void on_actionPlay4_triggered();
    void on_actionStop_triggered();
    void on_actionStep_triggered();
    void on_actionIndividualStep_triggered();
    void on_actionNextOccupant_triggered();
    void on_actionPrevOccupant_triggered();

    void on_actionSave_triggered();
    void on_actionLoad_triggered();

    void takeStep();
    void calculateStep();

    void DisplayInspector(Occupant* occ);
};

#endif // MAINWINDOW_HPP
