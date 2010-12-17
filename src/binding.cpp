#include "binding.hpp"
#include "../gui/widgets.hpp"

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

Binding* Binding::New(int& ptr, int min, int max)
{
    return (new IntWidget(min, max))->SetPointer(&ptr);
}

Binding* Binding::New(float& ptr, float min, float max, float step)
{
    return (new FloatWidget(min, max, step))->SetPointer(&ptr);
}

Binding* Binding::New(bool& ptr)
{
    return (new BoolWidget())->SetPointer(&ptr);
}

Binding* Binding::New(int& ptr, const char* str)
{
    return (new EnumWidget(str))->SetPointer(&ptr);
}

Binding* Binding::New(Matrix& ptr, int size, bool flip)
{
    return (new MatrixWidget(size, flip, NULL, NULL))->SetPointer(&ptr);
}

Binding* Binding::New(int& ptr)
{
    return (new IntLabel())->SetPointer(&ptr);
}
