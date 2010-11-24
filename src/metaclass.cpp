#include <assert.h>
#include <typeinfo>
#include <string.h>

#include "metaclass.hpp"

using namespace std;

int MetaClass::nmetaclasses = 0;
MetaClass* MetaClass::metaclasses[128];

const char* Class::Name()
{
    const char* name = typeid(*this).name();
    while (*name >= '0' and *name <= '9') name++;
    return name;
}

void Class::dummy()
{
}

MetaClass* Class::GetMetaClass()
{
    return MetaClass::Lookup(Name());
}

void Class::Write(ostream& os, int indent)
{
    cout << ntabs[indent] << "{\n";
    GetMetaClass()->Write(this, os, indent+1);
    cout << ntabs[indent] << "}\n";
}

void Class::Read(istream& is)
{
    std::string str;
    is >> str;
    assert(str == Name());
    GetMetaClass()->Read(this, is);
}

MetaClass::MetaClass(const char* _name, const char* _pname, ClassMaker func)
    : nvars(0),
      name(_name),
      pname(_pname),
      maker(func)
{
    metaclasses[nmetaclasses] = this;
    nmetaclasses++;
}

MetaClass* MetaClass::Lookup(const char* name)
{
    for (int i = 0; i < nmetaclasses; i++)
    {
        if (strcmp(name, metaclasses[i]->name) == 0) return metaclasses[i];
    }
    return NULL;
}


Class* MetaClass::MakeNew(const char* name)
{
    MetaClass* metaclass = Lookup(name);
    assert(metaclass);

    return (*metaclass->maker)();
}

Class* MetaClass::Create(std::istream& is)
{
    char name[128];
    is >> name;
    Class* serial = MakeNew(name);
    serial->Read(is);
    return serial;
}

void MetaClass::Read(Class* occ, istream& is)
{
    MetaClass* parent = Lookup(pname);
    if (parent) parent->Read(occ, is);
    for (int i = 0; i < nvars; i++)
    {
        (*readers[i])(occ, is);
    }
}

void MetaClass::Write(Class* occ, ostream& os, int indent)
{
    MetaClass* parent = Lookup(pname);
    if (parent)
    {
        parent->Write(occ, os, indent);
        os << ", \n";
    }
    for (int i = 0; i < nvars; i++)
    {
        if (i != 0) os << ", \n";
        (*writers[i])(occ, os, indent);
    }
}

Registrator::Registrator(MetaClass& metaclass, const char* name, ClassReader read, ClassWriter write)
{
    metaclass.writers[metaclass.nvars] = write;
    metaclass.readers[metaclass.nvars] = read;
    metaclass.nvars++;
}
