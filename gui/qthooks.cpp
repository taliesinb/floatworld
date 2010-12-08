#include "qthooks.hpp"
#include "../src/metaclass.hpp"
#include "../src/misc.hpp"
#include "../src/qthookdefs.hpp"
#include "../src/matrix.hpp"

using namespace std;

HookManager::HookManager(Class &mc, Object *obj)
    : mclass(mc), object(obj)
{

}

HookManager::~HookManager()
{
}

IntWidget::IntWidget(int min, int max) : Hook(SIGNAL(valueChanged(int)))
{
    setRange(min, max);
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

MatrixWidget::MatrixWidget(int size) : Hook(SIGNAL(ClickedCell(Pos))), MatrixLabel(NULL)
{
    matrix = NULL;
    pixel_scale = size;
}

void MatrixWidget::OnSetPointer()
{
    matrix = static_cast<Matrix*>(ptr);
    AllocateImage(matrix->cols, matrix->rows);
}

void MatrixWidget::Synchronize(bool inbound)
{
    if (inbound) { Rerender(); repaint(); }
}

void MatrixWidget::Rerender()
{
    QColor color(0, 0, 0);

    for (int i = 0; i < matrix->cols; i++)
    {
        QRgb* line1 = reinterpret_cast<QRgb*>(pixel_data->scanLine(i));
        for (int j = 0; j < matrix->rows; j++)
        {
            float val = (*matrix)(i, j);
            int sgn = val > 0 ? 1 : -1;
            val = 150 * log(1 + fabs(val));
            if (val > 255) val = 255;
            int hue = 64 + 64 * sgn;
            int sat = min(100 + int(val), 255);
            int var = min(100 + int(val), 255);
            color.setHsv(64 + 64 * sgn, val, 255);
            *line1++ = color.rgb();
        }
    }
}

void HookManager::child_changed()
{
    for_iterate(it, widgets)
    {
         dynamic_cast<Hook*>(*it)->Synchronize(false);
    }
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
    for (int i = 0; i < mclass.nqvars; i++)
    {
        QWidget* widget = (*mclass.factories[i])(object);
        QObject::connect(widget, dynamic_cast<Hook*>(widget)->changesignal, this, SLOT(child_changed()));
        addRow(mclass.labels[i], widget);
        widgets.push_back(widget);
    }
    UpdateChildren();
}


