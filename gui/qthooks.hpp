#ifndef QTHOOKS_HPP
#define QTHOOKS_HPP

#include <iostream>
#include <list>

#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>

class Class;
class Object;

class Hook
{
public:
    void* ptr;
    const char* changesignal;

    Hook(const char* sig);
    void SetPointer(void* ptr);

    virtual void Synchronize(bool inbound);
};

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

class HookManager : public QFormLayout
{
    Q_OBJECT

public:

    Class& mclass;
    Object* object;
    std::list<QWidget*> widgets;

    HookManager(Class& mc, Object* obj);

public slots:
    void child_changed();

public:
    void ConstructChildren();
    void UpdateChildren();
};

#endif // QTHOOKS_HPP
