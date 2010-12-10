#ifndef QTHOOKDEFS_HPP
#define QTHOOKDEFS_HPP

class QWidget;

class Hook
{
public:
    void* ptr;
    const char* changesignal;

    Hook(const char* sig);
    virtual ~Hook();
    void SetPointer(void* ptr);
    QWidget* AsWidget();

    virtual void OnSetPointer();
    virtual void Synchronize(bool inbound);
};

Hook* IntegerHook(int min, int max);
Hook* FloatHook(float min, float max, float step);
Hook* BoolHook();
Hook* EnumHook(const char* str);
Hook* MatrixHook(int size, bool flip);

#endif // QTHOOKDEFS_HPP
