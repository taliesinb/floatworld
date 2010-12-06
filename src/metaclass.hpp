#ifndef METACLASS_HPP
#define METACLASS_HPP

#include <iostream>
#include <list>

static const char* whitespace = "\t";

std::istream& operator>>(std::istream& is, const char* str);

class Class;

class Object
{
public:
    Class& GetMetaClass();
    void Write(std::ostream& os);
    void Read(std::istream& is);
    virtual void Reset();
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

typedef Object* (*ObjectMaker)();
typedef void (*ObjectWriter)(Object*, std::ostream&);
typedef void (*ObjectReader)(Object*, std::istream&);

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

    Class(const char* _name, const char* _pname, ObjectMaker func);
    void Read(Object* occ, std::istream& is);
    void Write(Object* occ, std::ostream& os);
};

class Registrator
{
  public:
    Registrator(Class& metaclass, const char* name, ObjectReader read, ObjectWriter write);
};   

#define RegisterClass(ThisClass, ParentClass)                           \
    Object* ThisClass##Factory() { return new ThisClass; }              \
    Class ThisClass##MetaClass(#ThisClass, #ParentClass,                \
                                   &ThisClass##Factory);

#define RegisterAbstractClass(ThisClass, ParentClass)                   \
    Class ThisClass##MetaClass(#ThisClass, #ParentClass, NULL);

#define RegisterVar(ThisClass, Name)                                    \
    void ThisClass##Writer##Name(Object* obj, std::ostream& os)         \
    { os << (dynamic_cast<ThisClass*>(obj))->Name; }                    \
    void ThisClass##Reader##Name(Object* obj, std::istream& is)         \
    { is >> (dynamic_cast<ThisClass*>(obj))->Name; }                    \
    Registrator ThisClass##Name##Registrator(ThisClass##MetaClass,      \
    #Name, &ThisClass##Reader##Name, &ThisClass##Writer##Name);

#define RegisterCustomVar(ThisClass, Field, SaveFunc, LoadFunc)         \
    void ThisClass##SaveHelper##Field(Object* obj, std::ostream& os)    \
    { SaveFunc(dynamic_cast<ThisClass*>(obj), os); }                    \
    void ThisClass##LoadHelper##Field(Object* obj, std::istream& is)    \
    { LoadFunc(dynamic_cast<ThisClass*>(obj), is); }                    \
    Registrator ThisClass##Field##Registrator(ThisClass##MetaClass,     \
    #Field, &ThisClass##LoadHelper##Field, &ThisClass##SaveHelper##Field);

#endif
