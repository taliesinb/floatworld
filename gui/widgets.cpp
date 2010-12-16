#include "widgets.hpp"

#include "../src/metaclass.hpp"
#include "../src/misc.hpp"
#include "../src/matrix.hpp"

using namespace std;

IntLabel::IntLabel() : Binding(NULL)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setStyleSheet( "background-color: white" );
    QFont font;
    font.setPointSize(10);
    setFont(font);
}

void IntLabel::Synchronize(bool inbound)
{
    if (inbound) setNum(*reinterpret_cast<int*>(ptr));
}

IntWidget::IntWidget(int _min, int _max) : Binding(SIGNAL(valueChanged(int)))
{
    setRange(_min, _max);
    QFont font;
    font.setPointSize(10);
    setFont(font);
}

void IntWidget::Synchronize(bool inbound)
{
    if (inbound) setValue(*reinterpret_cast<int*>(ptr));
    else *reinterpret_cast<int*>(ptr) = value();
}

FloatWidget::FloatWidget(float min, float max, float div) : Binding(SIGNAL(valueChanged(double)))
{
    setRange(min, max);
    setSingleStep(div);
    QFont font;
    font.setPointSize(10);
    setFont(font);
}

void FloatWidget::Synchronize(bool inbound)
{
    if (inbound) setValue(*reinterpret_cast<float*>(ptr));
    else *reinterpret_cast<float*>(ptr) = value();
}

BoolWidget::BoolWidget() : Binding(SIGNAL(toggled(bool)))
{
}

void BoolWidget::Synchronize(bool inbound)
{
    if (inbound) setChecked(*static_cast<bool*>(ptr));
    else *static_cast<bool*>(ptr) = isChecked();
}

EnumWidget::EnumWidget(const char *labels) : Binding(SIGNAL(activated(int)))
{
    QString str(labels);
    insertItems(0, str.split("\n"));
    QFont font;
    font.setPointSize(10);
    setFont(font);
}

void EnumWidget::Synchronize(bool inbound)
{
    if (inbound) setCurrentIndex(*static_cast<int*>(ptr));
    else *static_cast<int*>(ptr) = currentIndex();
}

QRgb RedBlueColorFunc(float value)
{
    int sgn = value > 0 ? 1 : -1;
    int val = 150 * log(1 + fabs(value));
    if (val > 255) val = 255;
    int hue = (256 + (64 + 64 * sgn)) % 256;
    int sat = val;
    int var = 255 - (val / 5.0);
    return QColor::fromHsv(hue, sat, var).rgb();
}

MatrixWidget::MatrixWidget(int size, bool flip)
    : MatrixView(size, flip, true), Binding(SIGNAL(ClickedCell(Pos)))
{
    color_func = &RedBlueColorFunc;
}

void MatrixWidget::OnSetPointer()
{
    matrix = static_cast<Matrix*>(ptr);
}

void MatrixWidget::Synchronize(bool inbound)
{
    if (inbound) repaint();
}

BindingsPanel::BindingsPanel(Class *mc, Object *obj)
    : mclass(mc), object(obj)
{

}

BindingsPanel::~BindingsPanel()
{
    being_removed();
}

void BindingsPanel::child_changed()
{
    for_iterate(it, widgets)
    {
         dynamic_cast<Binding*>(*it)->Synchronize(false);
    }
    value_changed();
}

void BindingsPanel::UpdateChildren()
{
    for_iterate(it, widgets)
    {
        QWidget* w = *it;
        w->blockSignals(true);
        dynamic_cast<Binding*>(w)->Synchronize(true);
        w->blockSignals(false);
    }
}

void BindingsPanel::ConstructChildren()
{
    if (Class* parent = Class::Lookup(mclass->pname))
    {
        Class* temp = mclass;
        mclass = parent;
        ConstructChildren();
        mclass = temp;
    }
    for (int i = 0; i < mclass->nqvars; i++)
    {
        QWidget* widget = (*mclass->factories[i])(object);
        const char* sig = dynamic_cast<Binding*>(widget)->changesignal;
        if (sig) QObject::connect(widget, sig, this, SLOT(child_changed()));
        addRow(mclass->labels[i], widget);
        widgets.push_back(widget);
    }
}



