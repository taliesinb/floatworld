#include "widgets.hpp"

#include "../src/metaclass.hpp"
#include "../src/misc.hpp"
#include "../src/matrix.hpp"

#include <QPainter>
#include <QMouseEvent>

using namespace std;

QRgb RedBlueColorFunc(float value);

MatrixView::MatrixView(int size, bool flip, bool grid)
{
    //setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
    color_func = &RedBlueColorFunc;
    draw_flipped = flip;
    matrix = NULL;
    border = 3;
    scale = size;
    draw_grid = grid;
}

QSize MatrixView::sizeHint() const
{
    return draw_flipped ?
            QSize(matrix->rows * scale + 2 * border + 1,
                  matrix->cols * scale + 2 * border + 1) :
            QSize(matrix->cols * scale + 2 * border + 1,
                  matrix->rows * scale + 2 * border + 1);
}

QSize MatrixView::minimumSizeHint() const
{
   return sizeHint();
}

void MatrixView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    int w = draw_flipped ? matrix->rows : matrix->cols;
    int h = draw_flipped ? matrix->cols : matrix->rows;

    int W = w * scale, H = h * scale;

    painter.setPen(Qt::black);
    painter.drawRect(QRect(0, 0, W + 2 * border, H + 2 * border));
    painter.setPen(Qt::white);
    painter.drawRect(QRect(1, 1, W + 2 * border - 2, H + 2 * border - 2));
    painter.fillRect(QRect(2, 2, W + 2 * border - 3, H + 2 * border - 3), Qt::white);

    QImage image(w, h, QImage::Format_RGB32);
    if (draw_flipped)
    {
        QRgb* line;
        for (int i = 0; i < h; i++)
        {
            line = reinterpret_cast<QRgb*>(image.scanLine(i));
            for (int j = 0; j < w; j++)
            {
                *line++ = (*color_func)(matrix->operator ()(j, i));
            }
        }
    } else {
        float* val = matrix->data;
        QRgb* line;
        for (int i = 0; i < h; i++)
        {
            line = reinterpret_cast<QRgb*>(image.scanLine(i));
            for (int j = 0; j < w; j++)
            {
                *line++ = (*color_func)(*val++);
            }
        }
    }
    painter.drawImage(QRect(border, border, W, H), image, QRect(0, 0, w, h));


    if (draw_grid) {
        painter.setPen(Qt::gray);
        for (int i = 0; i <= w; i++)
        {
            int x = border + i * scale;
            painter.drawLine(x, border, x, border + h * scale);
        }
        for (int i = 0; i <= h; i++)
        {
            int y = border + i * scale;
            painter.drawLine(border, y, border + w * scale, y);
        }
    }

    OverPaint(painter);

    if (highlighted.Inside(h, w))
    {
        QPen pen;
        pen.setWidth(1);
        pen.setColor(QColor(255,255,255,100));
        painter.setPen(pen);
        painter.drawRect(QRect(border + scale * highlighted.col - 2,
                               border + scale * highlighted.row - 2,
                               scale + 2, scale + 2));
    }
}

void MatrixView::resizeEvent(QResizeEvent *)
{
    WasResized();
}

void MatrixView::mousePressEvent(QMouseEvent *event)
{
    int x = (event->x() - border) / scale;
    int y = (event->y() - border) / scale;
    if (draw_flipped) swap(x, y);
    if (0 <= x && x < matrix->cols && 0 <= y && y < matrix->rows)
    {
        ClickedCell(Pos(y, x));
    }
}
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

MatrixWidget::MatrixWidget(int size, bool flip, const char* row_labels, const char* column_labels)
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



