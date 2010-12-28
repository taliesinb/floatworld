#ifndef BINDING_HPP
#define BINDING_HPP

class QWidget;
class Matrix;
class RNG;

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

    static Binding* New(int& ptr);
    static Binding* New(int& ptr, int min, int max);
    static Binding* New(float& ptr, float min, float max, float step);
    static Binding* New(bool& ptr);
    static Binding* New(int& ptr, const char* labels);
    static Binding* New(int &ptr, const char* labels, bool dummy);
    static Binding* New(RNG& rng);
    static Binding* New(Matrix& matrix, int size, bool flip, const char* rows=0, const char* cols=0);
};

#endif
