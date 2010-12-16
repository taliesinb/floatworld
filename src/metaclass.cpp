#include <assert.h>
#include <typeinfo>
#include <string.h>
#include <sstream>

#include <QObject>
#include <QWidget>

#include "../gui/widgets.hpp"
#include "metaclass.hpp"
#include "misc.hpp"

using namespace std;

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
        if (ch == 0)
        {
            cout << "Early termination with null symbol" << endl;
            cout << "Was expecting <" << str << ">" << endl;
            throw "error";
        }
        if (isspace(ch) && isspace(*s))
        {
            while (isspace(*s)) { s++; if (*s == 0) goto done; }
            while (isspace(ch)) is.get(ch);
        }
        if (ch != *s)
        {
            cout << "Expectation failed!" << endl;
            cout << "Was trying to match:" << endl;
            cout << "<" << str << ">" << endl;
            cout << "Got up to:" << endl;
            cout << "<" << s << ">" << endl;
            cout << "Last character read: <" << ch << ">" << endl;
            cout << "Rest of stream:" << endl;
            char s[128];
            is.read(s, 128);
            cout << s;
            throw "error";
        }
        s++;
    } while (*s);
    done:
    //cout << "matched <" << str << ">" << endl;
    return is;
};

std::string get_word(std::istream& is)
{
    char ch;
    string str;
    do {
        is.get(ch);
        if (isalnum(ch)) str += ch;
        else
        {
            is.putback(ch);
            break;
        }
    } while (ch);
    return str;
}

std::ostream& operator<<(std::ostream& os, Object& c)
{
    os << "{" << endl;
    os << "\t\"class\": \"" << c.Name() << "\"," << endl;
    c.Write(os);
    os << "}" << endl;
    return os;
}

std::istream& operator>>(std::istream& is, Object& c)
{
    const char* name = c.Name();
    is >> "{\t\"class\": \"" >> name >> "\",";
    c.Read(is);
    return is;
}

int Class::nmetaclasses = 0;
Class* Class::metaclasses[128];

Object::Object()
{
    panel = NULL;
}

const char* Object::Name()
{
    const char* name = typeid(*this).name();
    while (*name >= '0' and *name <= '9') name++;
    return name;
}

void Object::Reset()
{
}

BindingsPanel* Object::SetupQtHook(bool title)
{
    assert (panel == NULL);
    panel = new BindingsPanel(&GetClass(), this);
    if (title)
    {
        QLabel* label = new QLabel;
        label->setText(GetClass().name);
        QFont font;
        font.setPointSize(15);
        font.setBold(true);
        label->setFont(font);
        label->setAlignment(Qt::AlignHCenter);
        panel->addRow("type", label);
    }
    panel->ConstructChildren();
    panel->UpdateChildren();
    return panel;
}

void Object::UpdateQtHook()
{
    if (panel)
    {
        panel->UpdateChildren();
    }
}

void Object::DeleteQtHook()
{    
    if (panel) {
        QLayoutItem *child;
        while ((child = panel->takeAt(0)) != 0) {
                child->widget()->deleteLater();
                delete child;
        }
        delete panel;
    }
    panel = NULL;
}

Class& Object::GetClass()
{
    return *Class::Lookup(Name());
}

void Object::Write(ostream& os)
{
    stringstream ostr;
    string line;

    GetClass().Write(this, ostr);

    while(std::getline(ostr, line))
    {
       os << "\t" << line << endl;
    }
}

void Object::Read(istream& is)
{
    GetClass().Read(this, is);
}

Class::Class(const char* _name, const char* _pname, ObjectMaker func)
    : name(_name),
      pname(_pname),
      maker(func),
      nvars(0),
      nqvars(0)
{
    metaclasses[nmetaclasses] = this;
    nmetaclasses++;
}

Class* Class::Lookup(const char* name)
{
    for (int i = 0; i < nmetaclasses; i++)
    {
        if (strcmp(name, metaclasses[i]->name) == 0) return metaclasses[i];
    }
    return NULL;
}

Object* Class::MakeNew(const char* name)
{
    Class* metaclass = Lookup(name);
    assert(metaclass);

    return (*metaclass->maker)();
}

Object* Class::Create(std::istream& is)
{
//    cout << "Trying to read class" << endl;
    is >> "{\t\"class\": \"";
    string name = get_word(is);
    is >> "\"" >> whitespace >> "," >> whitespace;
//    cout << "Read the following classname: " << name << endl;
    Object* serial = MakeNew(name.c_str());
    serial->Read(is);
    is >> whitespace >> "}";
    return serial;
}

void Class::Read(Object* c, istream& is)
{
//    cout << "Starting to read class: " << name << endl;
    Class* parent = Lookup(pname);
    if (parent)
    {
        parent->Read(c, is);
        is >> whitespace >> "," >> whitespace;
    }
    c->Reset();
    for (int i = 0; i < nvars; i++)
    {
//        cout << "Reading \"" << varname[i] << "\", var " << i << " of " << c->Name() << endl;
        if (i != 0) is >> whitespace >> "," >> whitespace;
        is >> whitespace >> "\"";
        is >> varname[i];
        is >> "\":" >> whitespace;
        (*readers[i])(c, is);
    }
//    cout << "Done reading class: " << name << endl;
}

void Class::Write(Object* c, ostream& os)
{
    Class* parent = Lookup(pname);
    if (parent)
    {
        parent->Write(c, os);
        os << ", \n";
    }
    for (int i = 0; i < nvars; i++)
    {
        if (i != 0) os << ",\n";
        os << '"' << varname[i] << "\": ";
        (*writers[i])(c, os);
    }
}

Registrator::Registrator(Class& metaclass, const char* name, ObjectReader read, ObjectWriter write)
{
    metaclass.writers[metaclass.nvars] = write;
    metaclass.readers[metaclass.nvars] = read;
    metaclass.varname[metaclass.nvars] = name;
    metaclass.nvars++;
}

Registrator::Registrator(Class& metaclass, const char* label, HookFactory factory)
{
    int n = metaclass.nqvars++;
    metaclass.labels[n] = label;
    metaclass.factories[n] = factory;
}

