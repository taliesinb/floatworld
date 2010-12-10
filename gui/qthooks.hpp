#ifndef QTHOOKS_HPP
#define QTHOOKS_HPP

#include "widgets.hpp"

#include <iostream>
#include <list>

#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>

class Class;
class Object;
class Matrix;

class IntWidget : public QSpinBox, public Hook
{
public:
    IntWidget(int min, int max);

    virtual void Synchronize(bool inbound);
};

class FloatWidget : public QDoubleSpinBox, public Hook
{
public:
    FloatWidget(float min, float max, float div);

    virtual void Synchronize(bool inbound);
};

class BoolWidget : public QCheckBox, public Hook
{
public:
    BoolWidget();
    virtual void Synchronize(bool inbound);
};

class EnumWidget : public QComboBox, public Hook
{
public:
    EnumWidget(const char* labels);
    virtual void Synchronize(bool inbound);
};

class MatrixWidget : public MatrixLabel, public Hook
{
    Matrix* matrix;
    bool flipped;
    int rows, cols;

public:
    MatrixWidget(int pixel, bool flip);
    virtual void OnSetPointer();
    virtual void Rerender();
    virtual void Synchronize(bool inbound);
};

class HookManager : public QFormLayout
{
    Q_OBJECT

public:

    Class* mclass;
    Object* object;
    std::list<QWidget*> widgets;

    HookManager(Class* mc, Object* obj);
    virtual ~HookManager();

public slots:
    void child_changed();

signals:
    void value_changed();
    void being_removed();

public:
    void ConstructChildren();
    void UpdateChildren();
};

#endif // QTHOOKS_HPP
