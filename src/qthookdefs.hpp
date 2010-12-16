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
Binding* BindInteger(int min, int max);
Binding* BindFloat(float min, float max, float step);
Binding* BindBool();
Binding* BindEnum(const char* str);
Binding* BindMatrix(int size, bool flip);

#endif // QTHOOKDEFS_HPP
