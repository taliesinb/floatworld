#ifndef QTHOOKS_HPP
#define QTHOOKS_HPP

#include "widgets.hpp"

#include <iostream>
#include <list>

#include "../src/qthookdefs.hpp"

#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>

class Class;
class Object;
class Matrix;

class IntLabel : public QLabel, public Binding
{
public:
    IntLabel();

    virtual void Synchronize(bool inbound);
};

class IntWidget : public QSpinBox, public Binding
{
public:
    IntWidget(int min, int max);

    virtual void Synchronize(bool inbound);
};

class FloatWidget : public QDoubleSpinBox, public Binding
{
public:
    FloatWidget(float min, float max, float div);

    virtual void Synchronize(bool inbound);
};

class BoolWidget : public QCheckBox, public Binding
{
public:
    BoolWidget();
    virtual void Synchronize(bool inbound);
};

class EnumWidget : public QComboBox, public Binding
{
public:
    EnumWidget(const char* labels);
    virtual void Synchronize(bool inbound);
};

class MatrixWidget : public MatrixView, public Binding
{
    bool flipped;
    int rows, cols;

public:
    MatrixWidget(int pixel, bool flip);
    virtual void OnSetPointer();
    virtual void Synchronize(bool inbound);
};

class BindingsPanel : public QFormLayout
{
    Q_OBJECT

public:

    Class* mclass;
    Object* object;
    std::list<QWidget*> widgets;

    BindingsPanel(Class* mc, Object* obj);
    virtual ~BindingsPanel();

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
