#include "qthooks.hpp"
#include "../src/metaclass.hpp"
#include "../src/misc.hpp"

#include <QSpinBox>
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
        cout << "reading out widget " << i << endl;
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
        cout << "writing in widget " << i << endl;
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

