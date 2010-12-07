#ifndef METACLASS_HPP
#define METACLASS_HPP

#include <iostream>
#include <list>
#include "../gui/qthooks.hpp"
#include <QFormLayout>

static const char* whitespace = "\t";

std::istream& operator>>(std::istream& is, const char* str);

class Class;
class Object;
class HookObject;

class Object
{
private:
    HookObject* qt_hook;

public:
    Object();
    Class& GetClass();
    void Write(std::ostream& os);
    void Read(std::istream& is);

    virtual void Reset();
    void SetupQtHook();
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
typedef void (*QWriter)(QWidget*, std::ostream&);
typedef void (*QReader)(QWidget*, std::istream&);
typedef QWidget* (*QFactory)();

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
    QWriter qwriters[32];
    QReader qreaders[32];
    QFactory qfactories[32];
    int qvarindex[32];
    const char* qlabels[32];

    Class(const char* _name, const char* _pname, ObjectMaker func);
    void Read(Object* occ, std::istream& is);
    void Write(Object* occ, std::ostream& os);
    void ConstructWidgets(Object* obj);

};

class Registrator
{
  public:
    Registrator(Class& metaclass, const char* name, ObjectReader read, ObjectWriter write);
    Registrator(Class& metaclass, const char* name, const char* label, QReader read, QWriter write, QFactory factory);
};   

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

#define RegisterQtHook(CLASS, NAME, LABEL, WIDGET)\
    Registrator CLASS##Name##QRegistrator(CLASS##MetaClass, \
    #LABEL, #NAME, &WIDGET##Reader, &WIDGET##Writer,\
    &WIDGET##Factory)

#define RegisterCustomVar(CLASS, FIELD, SAVE, LOAD)         \
    void CLASS##SaveHelper##FIELD(Object* obj, std::ostream& os)    \
    { SAVE(dynamic_cast<CLASS*>(obj), os); }                    \
    void CLASS##LoadHelper##FIELD(Object* obj, std::istream& is)    \
    { LOAD(dynamic_cast<CLASS*>(obj), is); }                    \
    Registrator CLASS##FIELD##Registrator(CLASS##MetaClass,     \
    #FIELD, &CLASS##LoadHelper##FIELD, &CLASS##SaveHelper##FIELD);

#endif
