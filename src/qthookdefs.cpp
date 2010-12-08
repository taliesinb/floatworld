#include "qthookdefs.hpp"
#include "../gui/qthooks.hpp"

Hook::Hook(const char *sig) : changesignal(sig)
{
}

void Hook::SetPointer(void *p)
{
    ptr = p;
    OnSetPointer();
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

Hook* MatrixHook(int size)
{
    return new MatrixWidget(size);
}