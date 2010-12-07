#include "qthooks.hpp"
#include <QSpinBox>

QWidget* QSpinBoxFactory()
{
    return new QSpinBox;
}

void QSpinBoxWriter(QSpinBox* box, std::ostream& s)
{
    s << box->value();
}


void QSpinBoxReader(QSpinBox* box, std::istream& s)
{
    int val;
    s >> val;
    box->setValue(val);
}

