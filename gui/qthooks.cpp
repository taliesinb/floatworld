#include "qthooks.hpp"
#include "../src/metaclass.hpp"
#include "../src/misc.hpp"

#include <QSpinBox>
#include <QCheckBox>
#include <sstream>

using namespace std;

HookObject::HookObject(Class &mc, Object *obj)
    : mclass(mc), object(obj)
{

}

void HookObject::child_changed()
{
    int i = 0;
    for_iterate(w, widgets)
    {
//        cout << "reading out widget " << i << endl;
        stringstream s;
        (*(mclass.qwriters[i]))(*w, s);
        (*(mclass.readers[mclass.qvarindex[i]]))(object, s);
        i++;
    }
}

void HookObject::UpdateChildren()
{
    int i = 0;
    for_iterate(w, widgets)
    {
//        cout << "writing in widget " << i << endl;
        stringstream s;
        (*(mclass.writers[mclass.qvarindex[i]]))(object, s);

        QWidget* widget = *w;
        widget->blockSignals(true);
        (*mclass.qreaders[i])(widget, s);
        widget->blockSignals(false);
        i++;
    }
}

void HookObject::ConstructChildren()
{
    for(int i = 0; i < mclass.nqvars; i++)
    {
        QWidget* widget = (*mclass.qfactories[i])(this);
        addRow(mclass.qlabels[i], widget);
        widgets.push_back(widget);
    }
    UpdateChildren();
}


QWidget* QSpinBoxFactory(HookObject* h)
{
    QSpinBox* box = new QSpinBox;
    box->setRange(-10000,10000);
    QObject::connect(box, SIGNAL(valueChanged(int)),
                     h, SLOT(child_changed()));
    return box;
}

void QSpinBoxWriter(QSpinBox* box, std::ostream& s)
{
    s << box->value();
}


void QSpinBoxReader(QSpinBox* box, std::istream& s)
{
    int val;
    s >> val;
    box->setValue(val);
}

QWidget* QDoubleSpinBoxFactory(HookObject* h)
{
    QDoubleSpinBox* box = new QDoubleSpinBox;
    QObject::connect(box, SIGNAL(valueChanged(double)),
                     h, SLOT(child_changed()));
    return box;
}

void QDoubleSpinBoxWriter(QDoubleSpinBox* box, std::ostream& s)
{
    s << box->value();
}


void QDoubleSpinBoxReader(QDoubleSpinBox* box, std::istream& s)
{
    double val;
    s >> val;
    box->setValue(val);
}

QWidget* ProbabilityBoxFactory(HookObject* h)
{
    QDoubleSpinBox* box = new QDoubleSpinBox;
    box->setRange(0.0, 1.0);
    box->setSingleStep(0.001);
    box->setDecimals(3);
    QObject::connect(box, SIGNAL(valueChanged(double)),
                     h, SLOT(child_changed()));
    return box;
}

void ProbabilityBoxWriter(QDoubleSpinBox* box, std::ostream& s)
{
    s << box->value();
}


void ProbabilityBoxReader(QDoubleSpinBox* box, std::istream& s)
{
    double val;
    s >> val;
    box->setValue(val);
}

QWidget* QCheckBoxFactory(HookObject* h)
{
    QCheckBox* box = new QCheckBox;
    QObject::connect(box, SIGNAL(stateChanged(int)),
                     h, SLOT(child_changed()));
    return box;

}

void QCheckBoxReader(QCheckBox* box, std::istream& is)
{
    bool b;
    is >> b;
    box->setChecked(b);
}

void QCheckBoxWriter(QCheckBox* box, std::ostream& os)
{
    os << box->isChecked();
}

