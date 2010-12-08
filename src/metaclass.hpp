#ifndef METACLASS_HPP
#define METACLASS_HPP

#include <iostream>
#include <list>

static const char* whitespace = "\t";

std::istream& operator>>(std::istream& is, const char* str);

class Class;
class Object;
class HookManager;

class Object
{
public:
    HookManager* qt_hook;

    Object();
    Class& GetClass();
    void Write(std::ostream& os);
    void Read(std::istream& is);

    virtual void Reset();
    HookManager* SetupQtHook();
    void UpdateQtHook();
    void DeleteQtHook();
    const char* Name();
};

std::ostream& operator<<(std::ostream& os, Object& c);
std::istream& operator>>(std::istream& os, Object& c);

template<class T>
std::ostream& operator<<(std::ostream& os, std::list<T>& lst)
{
    os << "[";
    int count = 0;
    typename std::list<T>::iterator i = lst.begin();
    while (i != lst.end())
    {
        if (count++) os << ", ";
        os << *i++;
    }
    os << "]";
    return os;
}

template<class T>
std::istream& operator>>(std::istream& is, std::list<T>& lst)
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

/*

 we need a factory function to pass a reference to the actual
 class member. this factory function will take create a new
 widget with the right variables and then connect to a void
 pointer.

 registerqtchook(class, name, label, hookwidget)
 hook* factory(object* obj) =
 {
    hook* = new hookwidget;
    hook->SetPointer(static_cast<void*>(dynamic_cast<class*>(obj)->name)
    return hook;
 }

 RegisterQtHook actually takes a new widget as a
 'prototype'. It will copy it and then hook it to
 the corresponding value. But we want the corresponding
 classes to only be parameterized on type. So they need
 to take a pointer to the value they are hooked to.
 They also need to have refresh() method.

 What will registerqthook look like?

 registerqthook(class, name, label, hookwidget*)

 hookwidget will have a refresh method and an setpointer
 method. setpointer will have to be void*. It will do a
 reinterpretcast to the appropriate type.

 hookwidget will also know how to clone itself. but it'll
 have to clone itself from an ordinary pointer. so we need
 another cast to cast it to its own type.


*/

#include "qthookdefs.hpp"

#define RegisterQtHook(CLASS, NAME, LABEL, PROTOTYPE)       \
    QWidget* CLASS##NAME##Factory(Object* obj) {            \
    Hook* h = PROTOTYPE;                                \
    h->SetPointer(static_cast<void*>(&dynamic_cast<CLASS*>(obj)->NAME)); \
    return h->AsWidget(); }                                                \
    Registrator CLASS##NAME##HookRegistrator(CLASS##MetaClass, \
    LABEL, &CLASS##NAME##Factory);


#define RegisterClass(CLASS, PARENT)                           \
    Object* CLASS##Factory() { return new CLASS; }              \
    Class CLASS##MetaClass(#CLASS, #PARENT,                \
    &CLASS##Factory);

#define RegisterAbstractClass(CLASS, PARENT)                   \
    Class CLASS##MetaClass(#CLASS, #PARENT, NULL);

#define RegisterVar(CLASS, NAME)                                    \
    void CLASS##Writer##NAME(Object* obj, std::ostream& os)         \
    { os << (dynamic_cast<CLASS*>(obj))->NAME; }                    \
    void CLASS##Reader##NAME(Object* obj, std::istream& is)         \
    { is >> (dynamic_cast<CLASS*>(obj))->NAME; }                    \
    Registrator CLASS##NAME##Registrator(CLASS##MetaClass,      \
    #NAME, &CLASS##Reader##NAME, &CLASS##Writer##NAME);

#define RegisterCustomVar(CLASS, FIELD, SAVE, LOAD)         \
    void CLASS##SaveHelper##FIELD(Object* obj, std::ostream& os)    \
    { SAVE(dynamic_cast<CLASS*>(obj), os); }                    \
    void CLASS##LoadHelper##FIELD(Object* obj, std::istream& is)    \
    { LOAD(dynamic_cast<CLASS*>(obj), is); }                    \
    Registrator CLASS##FIELD##Registrator(CLASS##MetaClass,     \
    #FIELD, &CLASS##LoadHelper##FIELD, &CLASS##SaveHelper##FIELD);

#endif
