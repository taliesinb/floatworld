#include "qthookdefs.hpp"
#include "../gui/qthooks.hpp"

Binding::Binding(const char *sig) : changesignal(sig)
{
}

Binding::~Binding()
{

}

Binding* Binding::SetPointer(void *p)
{
    ptr = p;
    OnSetPointer();
    return this;
}

void Binding::OnSetPointer()
{

}

void Binding::Synchronize(bool)
{

}

QWidget* Binding::AsWidget()
{
    return dynamic_cast<QWidget*>(this);
}

Binding* IntegerHook(int min, int max)
{
    return new IntWidget(min, max);
}

Binding* FloatHook(float min, float max, float step)
{
    return new FloatWidget(min, max, step);
}

Binding* BoolHook()
{
    return new BoolWidget();
}

Binding* EnumHook(const char* str)
{
    return new EnumWidget(str);
}

Binding* MatrixHook(int size, bool flip)
{
    return new MatrixWidget(size, flip);
}

Binding* IntegerLabel()
{
    return new IntLabel;
}
