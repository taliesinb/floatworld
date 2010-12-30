#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtGui/QMainWindow>
#include <QtGui/QGraphicsView>
#include <QTimer>
#include <QElapsedTimer>
#include <QMap>
#include <QString>

#include "../src/world.hpp"
#include "../src/shape.hpp"
#include "../src/block.hpp"

#include "ui_mainwindow.h"
#include "widgets.hpp"

class World;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    Matrix adam;
    QElapsedTimer timer;
    QTimer ticker;
    QString prototype;
    float speed;
    float stepper;
    float last_stepper;
    bool block_draw;
    World* world;
    QActionGroup speed_group;
    QList<QString> world_cache;
    Occupant* selected_object;
    QMap<QString, Occupant*> prototypes;
    RNG rng;

    MainWindow();
    void SetSpeed(float speed);

    void showEvent(QShowEvent *);

private slots:

    void speed_trigger(QAction*);
    void on_actionStep_triggered();
    void on_actionStepBack_triggered();
    void on_actionIndividualStep_triggered();
    void on_actionNextOccupant_triggered();
    void on_actionPrevOccupant_triggered();
    void on_actionClearCreats_triggered();
    void on_actionClearAll_triggered();
    void on_actionDeleteSelected_triggered();

    void ff_pressed();
    void ff_released();
    void Stop(); // SetSpeed(0)

    void on_actionNew_triggered();
    void on_actionRestart_triggered();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_scatterButton_pressed();

    void ObjectSelected(QString);
    void CreateObjectAt(Pos pos);

    void Tick();

    void DisplayInspector(Occupant* occ);
};

#endif // MAINWINDOW_HPP
