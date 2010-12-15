#include "qthookdefs.hpp"
#include "../gui/qthooks.hpp"

Hook::Hook(const char *sig) : changesignal(sig)
{
}

Hook::~Hook()
{

}

Hook* Hook::SetPointer(void *p)
{
    ptr = p;
    OnSetPointer();
    return this;
}

void Hook::OnSetPointer()
{

}

void Hook::Synchronize(bool)
{

}

QWidget* Hook::AsWidget()
{
    return dynamic_cast<QWidget*>(this);
}

Hook* IntegerHook(int min, int max)
{
    return new IntWidget(min, max);
}

Hook* FloatHook(float min, float max, float step)
{
    return new FloatWidget(min, max, step);
}

Hook* BoolHook()
{
    return new BoolWidget();
}

Hook* EnumHook(const char* str)
{
    return new EnumWidget(str);
}

Hook* MatrixHook(int size, bool flip)
{
    return new MatrixWidget(size, flip);
}

Hook* IntegerLabel()
{
    return new IntLabel;
}
