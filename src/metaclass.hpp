#ifndef METACLASS_HPP
#define METACLASS_HPP

#include <iostream>
#include <list>

extern const char* whitespace;
std::istream& operator>>(std::istream& is, const char* str);

class MetaClass;

class Class
{
public:
    MetaClass& GetMetaClass();
    void Write(std::ostream& os);
    void Read(std::istream& is);
    virtual void Reset();
    const char* Name();
};

std::ostream& operator<<(std::ostream& os, Class& c);
std::istream& operator>>(std::istream& os, Class& c);

template<class T>
std::ostream& operator<<(std::ostream& os, std::list<T>& lst)
{
    os << "[";
    int sz = lst.size();
    for (int j = 0; j < sz; j++)
    {
        if (j) os << ", ";
        os << lst[j];
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

typedef Class* (*ClassMaker)();
typedef void (*ClassWriter)(Class*, std::ostream&);
typedef void (*ClassReader)(Class*, std::istream&);

class MetaClass
{
public:

    static int nmetaclasses;
    static MetaClass* metaclasses[128];
    static MetaClass* Lookup(const char* name);
    static Class* MakeNew(const char* name);
    static Class* Create(std::istream& is);
    
    const char* name;
    const char* pname;
    ClassMaker maker;

    int nvars;
    ClassWriter writers[32];
    ClassReader readers[32];
    const char* varname[32];

    MetaClass(const char* _name, const char* _pname, ClassMaker func);
    void Read(Class* occ, std::istream& is);
    void Write(Class* occ, std::ostream& os);
};

class Registrator
{
  public:
    Registrator(MetaClass& metaclass, const char* name, ClassReader read, ClassWriter write);
};   

#define RegisterClass(ThisClass, ParentClass)                       \
    Class* ThisClass##Factory() { return new ThisClass; }           \
    MetaClass ThisClass##MetaClass(#ThisClass, #ParentClass,        \
                                   &ThisClass##Factory);

#define RegisterAbstractClass(ThisClass, ParentClass)               \
    MetaClass ThisClass##MetaClass(#ThisClass, #ParentClass, NULL);

#define RegisterVar(ThisClass, Name)                                \
    void ThisClass##Name##Writer(Class* occ, std::ostream& os)      \
    { os << (dynamic_cast<ThisClass*>(occ))->Name; }                \
    void ThisClass##Name##Reader(Class* occ, std::istream& is)      \
    { is >> (dynamic_cast<ThisClass*>(occ))->Name; }                \
    Registrator ThisClass##Name##Registrator(ThisClass##MetaClass,  \
    #Name, &ThisClass##Name##Reader, &ThisClass##Name##Writer);

#define RegisterCustomVar(ThisClass, CodeSave, CodeLoad)            \
    void ThisClass##CustomSave(ThisClass* this, std::ostream& os)   \
    CodeSave                                                        \
    void ThisClass##CustomLoad(ThisClass* this, std::istream& is)   \
    CodeLoad                                                        \
    void ThisClass##CustomSave2(Class* occ, std::ostream& os)       \
    { ThisClass##CustomSave(dynamic_cast<ThisClass *>(this), os); } \
    void ThisClass##CustomLoad2(Class* occ, std::istream& is)       \
    { Class##CustomLoad(dynamic_cast<ThisClass *>(this), is); }     \
    Registrator ThisClass##CustomRegistrator(ThisClass##MetaClass,  \
    "Custom", &ThisClass##CustomLoad2, &ThisClass##CustomSave2);

#endif
