#ifndef METACLASS_HPP
#define METACLASS_HPP

#include <iostream>

static const char* ntabs[] = {"", "\t", "\t\t", "\t\t\t", "\t\t\t\t", "\t\t\t\t\t"};

class MetaClass;

class Class
{
public:
    virtual void dummy();
    MetaClass* GetMetaClass();
    void Write(std::ostream& os, int indent=0);
    void Read(std::istream& is);
    const char* Name();
};

typedef Class* (*ClassMaker)();
typedef void (*ClassWriter)(Class*, std::ostream&, int indent);
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

    MetaClass(const char* _name, const char* _pname, ClassMaker func);
    void Read(Class* occ, std::istream& is);
    void Write(Class* occ, std::ostream& os, int indent=0);
};

class Registrator
{
  public:
    Registrator(MetaClass& metaclass, const char* name, ClassReader read, ClassWriter write);
};   

#define RegisterClass(ThisClass, ParentClass)                        \
Class* META_##ThisClass##New() { return new ThisClass; }                  \
        MetaClass META_##ThisClass##MetaClass(#ThisClass, #ParentClass, &META_##ThisClass##New);

#define RegisterAbstractClass(ThisClass, ParentClass)               \
  MetaClass META_##ThisClass##MetaClass(#ThisClass, #ParentClass, NULL);

#define RegisterVar(ThisClass, Name)                                             \
  void META_##ThisClass##Name##Writer(Class* occ, std::ostream& os, int indent) { os << ntabs[indent] << '"' << #Name << "\": " << (dynamic_cast<ThisClass*>(occ))->Name; } \
          void META_##ThisClass##Name##Reader(Class* occ, std::istream& is) { is >> (dynamic_cast<ThisClass*>(occ))->Name; } \
          Registrator META_##ThisClass##Name##Registrator(META_##ThisClass##MetaClass, #Name, &META_##ThisClass##Name##Reader, &META_##ThisClass##Name##Writer);

#define RegisterCustomVar(ThisClass, CodeSave, CodeLoad)                            \
  void META_##ThisClass##CustomSave(ThisClass* this, std::ostream& os)                 \
    CodeSave                                                            \
    void META_##ThisClass##CustomLoad(ThisClass* this, std::istream& is)               \
    CodeLoad                                                            \
    void META_##ThisClass##CustomSave2(Class* occ, std::ostream& os)               \
    { ThisClass##CustomSave(dynamic_cast<ThisClass *>(this), os); }               \
    void META_##ThisClass##CustomLoad2(Class* occ, std::istream& is)        \
    { Class##CustomLoad(dynamic_cast<ThisClass *>(this), is); }             \
    Registrator META_##ThisClass##CustomRegistrator(META_##ThisClass##MetaClass, "Custom", &META_##ThisClass##CustomLoad2, &META_##ThisClass##CustomSave2);

#endif
