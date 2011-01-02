#ifndef METACLASS_HPP
#define METACLASS_HPP

#include "misc.hpp"
#include "binding.hpp"
#include "rng.hpp"

#include <iostream>
#include <QLinkedList>
#include <QString>

static const char* whitespace = "\t";

std::istream& operator>>(std::istream& is, const char* str);

std::istream& operator>>(std::istream& is, QString& s);
std::ostream& operator<<(std::ostream& os, QString& s);

extern bool human_readable;

class Class;
class Object;
class BindingsPanel;

class Object
{
public: // not to be used by others!
    RNG rng;

public:
    BindingsPanel* panel;

    Object();
    ~Object();

    Class& GetClass();
    void Write(std::ostream& os);
    void Read(std::istream& is);

    BindingsPanel* SetupPanel(bool title);
    void UpdatePanel();
    void DeletePanel();

    virtual void Reset();
    virtual void HookWasChanged();

    const char* Name();
};

std::ostream& operator<<(std::ostream& os, Object& c);
std::istream& operator>>(std::istream& os, Object& c);

template<class T>
std::ostream& operator<<(std::ostream& os, QLinkedList<T>& lst)
{
    typename QLinkedList<T>::iterator i = lst.begin();
    if (human_readable)
    {
        os << "[";
        int count = 0;
        while (i != lst.end())
        {
            if (count++) os << ", ";
            os << *i++;
        }
        os << "]";
    } else {
        os << lst.size() << " ";
        while (i != lst.end())
        {
            os << *i++ << " ";
        }
    }
    return os;
}

template<class T>
std::istream& operator>>(std::istream& is, QLinkedList<T>& lst)
{
    if (human_readable)
    {
        is >> "[" >> whitespace;
        char ch;
        while (is.get(ch) && ch)
        {
            if (ch == ']') break;
            else is.putback(ch);

            T t;
            is >> whitespace >> t >> whitespace;
            lst.push_back(t);

            is.get(ch);
            if (ch != ',') is.putback(ch);
            else is >> whitespace;
        }
    } else {
        int sz;
        is >> sz;
        for (int i = 0; i < sz; i++)
        {
            T t;
            is >> t;
            lst.push_back(t);
        }
    }
    return is;
}

class QWidget;
typedef Object* (*ObjectMaker)();
typedef void (*ObjectWriter)(Object*, std::ostream&);
typedef void (*ObjectReader)(Object*, std::istream&);
typedef QWidget* (*HookFactory)(Object*);

class Class
{
public:

    static int nmetaclasses;
    static Class* metaclasses[128];
    static Class* Lookup(const char* name);
    static Object* MakeNew(const char* name);
    static Object* Create(std::istream& is);
    
    bool abstract;
    const char* name;
    const char* pname;
    ObjectMaker maker;

    int nvars;
    ObjectWriter writers[32];
    ObjectReader readers[32];
    const char* varname[32];

    int nqvars;
    HookFactory factories[32];
    const char* labels[32];

    Class(const char* _name, const char* _pname, ObjectMaker func);
    void Read(Object* occ, std::istream& is);
    void Write(Object* occ, std::ostream& os);
    void ConstructWidgets(Object* obj);

};

class Registrator
{
  public:
    Registrator(Class& metaclass, const char* name, ObjectReader read, ObjectWriter write);
    Registrator(Class& metaclass, const char* label, HookFactory factory);
};   

#define RegisterBinding(CLASS, NAME, LABEL, ...)                                \
    QWidget* CLASS##NAME##Factory(Object* obj) {                                \
    Binding* h = Binding::New(dynamic_cast<CLASS*>(obj)->NAME, ##__VA_ARGS__);  \
    return h->AsWidget(); }                                                     \
    Registrator CLASS##NAME##BindingRegistrator(CLASS##MetaClass,               \
    LABEL, &CLASS##NAME##Factory);

#define RegisterArrayBinding(CLASS, NAME, VALUE, LABEL, ...)                    \
    QWidget* CLASS##NAME##VALUE##Factory(Object* obj) {                         \
    Binding* h = Binding::New(dynamic_cast<CLASS*>(obj)->NAME[VALUE], ##__VA_ARGS__);  \
    return h->AsWidget(); }                                                     \
    Registrator CLASS##NAME##VALUE##BindingRegistrator(CLASS##MetaClass,        \
    LABEL, &CLASS##NAME##VALUE##Factory);

#define RegisterClass(CLASS, PARENT)                                \
    Object* CLASS##Factory() { return new CLASS; }                  \
    Class CLASS##MetaClass(#CLASS, #PARENT,                         \
    &CLASS##Factory);

#define RegisterAbstractClass(CLASS, PARENT)                        \
    Class CLASS##MetaClass(#CLASS, #PARENT, NULL);

#define RegisterVar(CLASS, NAME)                                    \
    void CLASS##Writer##NAME(Object* obj, std::ostream& os)         \
    { os << (dynamic_cast<CLASS*>(obj))->NAME; }                    \
    void CLASS##Reader##NAME(Object* obj, std::istream& is)         \
    { is >> (dynamic_cast<CLASS*>(obj))->NAME; }                    \
    Registrator CLASS##NAME##Registrator(CLASS##MetaClass,          \
    #NAME, &CLASS##Reader##NAME, &CLASS##Writer##NAME);

#define RegisterCustomVar(CLASS, FIELD, SAVE, LOAD)                 \
    void CLASS##SaveHelper##FIELD(Object* obj, std::ostream& os)    \
    { SAVE(dynamic_cast<CLASS*>(obj), os); }                        \
    void CLASS##LoadHelper##FIELD(Object* obj, std::istream& is)    \
    { LOAD(dynamic_cast<CLASS*>(obj), is); }                        \
    Registrator CLASS##FIELD##Registrator(CLASS##MetaClass,         \
    #FIELD, &CLASS##LoadHelper##FIELD, &CLASS##SaveHelper##FIELD);

#endif
