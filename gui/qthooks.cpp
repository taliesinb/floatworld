#include "qthooks.hpp"
#include "../src/metaclass.hpp"
#include "../src/misc.hpp"

using namespace std;

Hook::Hook(const char *sig) : changesignal(sig)
{
}

void Hook::SetPointer(void *p)
{
    ptr = p;
}

void Hook::Synchronize(bool)
{

}

HookManager::HookManager(Class &mc, Object *obj)
    : mclass(mc), object(obj)
{

}

IntWidget::IntWidget(int min, int max) : Hook(SIGNAL(valueChanged(int)))
{
    setRange(min, max);
}

void IntWidget::Synchronize(bool inbound)
{
    if (inbound) setValue(*reinterpret_cast<int*>(ptr));
    else *reinterpret_cast<int*>(ptr) = value();
}

FloatWidget::FloatWidget(float min, float max, float div) : Hook(SIGNAL(valueChanged(double)))
{
    setRange(min, max);
    setSingleStep(div);
}

void FloatWidget::Synchronize(bool inbound)
{
    if (inbound) setValue(*reinterpret_cast<float*>(ptr));
    else *reinterpret_cast<float*>(ptr) = value();
}

void HookManager::child_changed()
{
    for_iterate(it, widgets)
    {
         dynamic_cast<Hook*>(*it)->Synchronize(false);
    }
}

void HookManager::UpdateChildren()
{
    for_iterate(it, widgets)
    {
        QWidget* w = *it;
        w->blockSignals(true);
        dynamic_cast<Hook*>(w)->Synchronize(true);
        w->blockSignals(false);
    }
}

void HookManager::ConstructChildren()
{
    for (int i = 0; i < mclass.nqvars; i++)
    {
        QWidget* widget = (*mclass.factories[i])(object);
        QObject::connect(widget, dynamic_cast<Hook*>(widget)->changesignal, this, SLOT(child_changed()));
        addRow(mclass.labels[i], widget);
        widgets.push_back(widget);
    }
    UpdateChildren();
}

BoolWidget::BoolWidget() : Hook(SIGNAL(toggled(bool)))
{
}

void BoolWidget::Synchronize(bool inbound)
{
    if (inbound) setChecked(*static_cast<bool*>(ptr));
    else *static_cast<bool*>(ptr) = isChecked();
}

EnumWidget::EnumWidget(const char *labels) : Hook(SIGNAL(activated(int)))
{
    QString str(labels);
    insertItems(0, str.split("\n"));
    QFont font;
    font.setPointSize(10);
    setFont(font);
}

void EnumWidget::Synchronize(bool inbound)
{
    if (inbound) setCurrentIndex(*static_cast<int*>(ptr));
    else *static_cast<int*>(ptr) = currentIndex();
}
