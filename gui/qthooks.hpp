#ifndef QTHOOKS_HPP
#define QTHOOKS_HPP

#include <iostream>

class QWidget;
class QSpinBox;

QWidget* QSpinBoxFactory();
void QSpinBoxReader(QSpinBox* box, std::istream& is);
void QSpinBoxWriter(QSpinBox* box, std::ostream& os);

#endif // QTHOOKS_HPP
