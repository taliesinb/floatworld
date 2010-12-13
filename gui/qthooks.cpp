#include "qthooks.hpp"
#include "../src/metaclass.hpp"
#include "../src/misc.hpp"
#include "../src/qthookdefs.hpp"
#include "../src/matrix.hpp"

using namespace std;

IntLabel::IntLabel() : Hook(NULL)
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

IntWidget::IntWidget(int _min, int _max) : Hook(SIGNAL(valueChanged(int)))
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

FloatWidget::FloatWidget(float min, float max, float div) : Hook(SIGNAL(valueChanged(double)))
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

BoolWidget::BoolWidget() : Hook(SIGNAL(toggled(bool)))
{
}

void BoolWidget::Synchronize(bool inbound)
{
    if (inbound) setChecked(*static_cast<bool*>(ptr));
    else *static_cast<bool*>(ptr) = isChecked();
}

EnumWidget::EnumWidget(const char *labels) : Hook(SIGNAL(activated(int)))
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

MatrixWidget::MatrixWidget(int size, bool flip)
    : MatrixLabel(), Hook(SIGNAL(ClickedCell(Pos)))
{
    flipped = flip;
    matrix = NULL;
    pixel_scale = size;
    rows = cols = 0;
    draw_grid = true;
}

void MatrixWidget::OnSetPointer()
{
    matrix = static_cast<Matrix*>(ptr);
    rows = matrix->rows;
    cols = matrix->cols;
    if (flipped) swap(rows, cols);
    AllocateImage(cols, rows);
}

void MatrixWidget::Synchronize(bool inbound)
{
    if (inbound) { Rerender(); repaint(); }
}

void MatrixWidget::Rerender()
{
    QColor color(0, 0, 0);
    for (int i = 0; i < rows; i++)
    {
        QRgb* line1 = reinterpret_cast<QRgb*>(pixel_data->scanLine(i));
        for (int j = 0; j < cols; j++)
        {
            float val = flipped ? (*matrix)(j, i) : (*matrix)(i, j);
            int sgn = val > 0 ? 1 : -1;
            val = 150 * log(1 + fabs(val));
            if (val > 255) val = 255;
            int hue = 256 + (64 + 64 * sgn) % 256;
            int sat = val;
            int var = 255 - (val / 5);
            color.setHsv(hue, sat, var);
            *line1++ = color.rgb();
        }
    }
}

HookManager::HookManager(Class *mc, Object *obj)
    : mclass(mc), object(obj)
{

}

HookManager::~HookManager()
{
    being_removed();
}

void HookManager::child_changed()
{
    for_iterate(it, widgets)
    {
         dynamic_cast<Hook*>(*it)->Synchronize(false);
    }
    value_changed();
}

void HookManager::UpdateChildren()
{
    for_iterate(it, widgets)
    {
        QWidget* w = *it;
        w->blockSignals(true);
        dynamic_cast<Hook*>(w)->Synchronize(true);
        w->blockSignals(false);
    }
}

void HookManager::ConstructChildren()
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
        const char* sig = dynamic_cast<Hook*>(widget)->changesignal;
        if (sig) QObject::connect(widget, sig, this, SLOT(child_changed()));
        addRow(mclass->labels[i], widget);
        widgets.push_back(widget);
    }
}


