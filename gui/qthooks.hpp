#ifndef QTHOOKS_HPP
#define QTHOOKS_HPP

#include <iostream>
#include <list>

#include <QFormLayout>

class QSpinBox;
class Class;
class Object;

class HookObject : public QFormLayout
{
    Q_OBJECT

public:

    Class& mclass;
    Object* object;
    std::list<QWidget*> widgets;

    HookObject(Class& mc, Object* obj);

public slots:
    void child_changed();

public:
    void ConstructChildren();
    void UpdateChildren();
};

#endif // QTHOOKS_HPP
