#include <assert.h>
#include <typeinfo>
#include <string.h>
#include <sstream>

#include "metaclass.hpp"
#include "misc.hpp"

using namespace std;

const char* whitespace = "\t";

std::istream& operator>>(std::istream& is, const char* str)
{
    if (str == whitespace)
    {
        char ch;
        do is.get(ch); while (ch && isspace(ch));
        is.putback(ch);
        return is;
    }

    const char* s = str;
    char ch;

    do {
        is.get(ch);
        if (isspace(ch) && isspace(*s))
        {
            while (isspace(*s)) { s++; if (*s == 0) goto done; }
            while (isspace(ch)) is.get(ch);
        }
        if (ch != *s)
        {
            cout << "Expectation failed. Rest of stream:" << endl;
            char s[128];
            is.read(s, 128);
            cout << s;
            throw "error";
        }
        s++;
    } while (*s);
    done:
    return is;
};


std::ostream& operator<<(std::ostream& os, Class& c)
{
    os << c.Name() << endl;
    c.Write(os);
    return os;
}

std::istream& operator>>(std::istream& is, Class& c)
{
    const char* name = c.Name();
    is >> name;
    is >> whitespace;
    c.Read(is);
    return is;
}



int MetaClass::nmetaclasses = 0;
MetaClass* MetaClass::metaclasses[128];

const char* Class::Name()
{
    const char* name = typeid(*this).name();
    while (*name >= '0' and *name <= '9') name++;
    return name;
}

void Class::Reset()
{
}

MetaClass& Class::GetMetaClass()
{
    return *MetaClass::Lookup(Name());
}

void Class::Write(ostream& os)
{
    stringstream ostr;
    string line;

    GetMetaClass().Write(this, ostr);

    os << "{" << endl;
    while(std::getline(ostr, line))
    {
       os << "\t" << line << endl;
    }
    os << "}" << endl;
}

void Class::Read(istream& is)
{
    is >> "{" >> whitespace;

    GetMetaClass().Read(this, is);

    is >> whitespace >> "}";
}

MetaClass::MetaClass(const char* _name, const char* _pname, ClassMaker func)
    : name(_name),
      pname(_pname),
      maker(func),
      nvars(0)
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

void MetaClass::Read(Class* c, istream& is)
{
    MetaClass* parent = Lookup(pname);
    if (parent)
    {
        parent->Read(c, is);
        is >> whitespace >> "," >> whitespace;
    }
    for (int i = 0; i < nvars; i++)
    {
        if (i != 0) is >> whitespace >> "," >> whitespace;
        is >> whitespace >> "\"";
        is >> varname[i];
        is >> "\":" >> whitespace;
        (*readers[i])(c, is);
    }
}

void MetaClass::Write(Class* c, ostream& os)
{
    MetaClass* parent = Lookup(pname);
    if (parent)
    {
        parent->Write(c, os);
        os << ", \n";
    }
    for (int i = 0; i < nvars; i++)
    {
        if (i != 0) os << ", \n";
        os << '"' << varname[i] << "\": ";
        (*writers[i])(c, os);
    }
}

Registrator::Registrator(MetaClass& metaclass, const char* name, ClassReader read, ClassWriter write)
{
    metaclass.writers[metaclass.nvars] = write;
    metaclass.readers[metaclass.nvars] = read;
    metaclass.varname[metaclass.nvars] = name;
    metaclass.nvars++;
}


