#ifndef QTHOOKDEFS_HPP
#define QTHOOKDEFS_HPP

class QWidget;
class QFormLayout;

class Binding
{
public:
    void* ptr;
    const char* changesignal;

    Binding(const char* sig);
    virtual ~Binding();

    Binding* SetPointer(void* ptr);
    QWidget* AsWidget();
    virtual void OnSetPointer();
    virtual void Synchronize(bool inbound);
};

Binding* IntegerLabel();
Binding* IntegerHook(int min, int max);
Binding* FloatHook(float min, float max, float step);
Binding* BoolHook();
Binding* EnumHook(const char* str);
Binding* MatrixHook(int size, bool flip);

#endif // QTHOOKDEFS_HPP
