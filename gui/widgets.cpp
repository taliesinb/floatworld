#include "widgets.hpp"
#include "qthooks.hpp"
#include "../src/qthookdefs.hpp"
#include "../src/block.hpp"
#include "../src/grid.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <QScrollArea>
#include <QGridLayout>

using namespace std;

int sz = 120;
int border = 3;

MatrixLabel::MatrixLabel(QWidget* parent)
    : QWidget(parent),
    pixel_data(NULL),
    pixel_scale(3),
    draw_grid(false), highlighted(-1,-1)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
}

MatrixLabel::MatrixLabel()
        : QWidget(),
        pixel_data(NULL),
        pixel_scale(3),
        draw_grid(false), highlighted(-1,-1)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
}

void MatrixLabel::AllocateImage(int width, int height)
{
    if (pixel_data) delete pixel_data;
    pixel_data = new QImage(width, height, QImage::Format_RGB32);
    pixel_data->fill(0);
    setMaximumSize(sizeHint());
    Rerender();
}

QSize MatrixLabel::sizeHint() const
{
    return QSize(pixel_data->width() * pixel_scale + 2*border + 1,
                 pixel_data->height() * pixel_scale + 2*border + 1);
}

QSize MatrixLabel::minimumSizeHint() const
{
   return sizeHint();
}

void MatrixLabel::Rerender()
{

}

void MatrixLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    int w = pixel_data->width(), h = pixel_data->height();
    int w2 = w * pixel_scale, h2 = h * pixel_scale;
    painter.setPen(Qt::black);
    painter.drawRect(QRect(0, 0, w2 + 2 * border, h2 + 2 * border));
    painter.setPen(Qt::white);
    painter.drawRect(QRect(1,1,w2 + 2 * border-2, h2 + 2 * border-2));
    painter.fillRect(QRect(2,2,w2 + 2 * border-3, h2 + 2 * border-3), Qt::white);
    painter.drawImage(QRect(border, border, w2, h2),
                      *pixel_data, QRect(0, 0, w, h));

    if (draw_grid) {
        painter.setPen(Qt::gray);
        for (int i = 0; i <= w; i++)
        {
            int x = border + i * pixel_scale;
            painter.drawLine(x, border, x, border + h * pixel_scale);
        }
        for (int i = 0; i <= h; i++)
        {
            int y = border + i * pixel_scale;
            painter.drawLine(border, y, border + w * pixel_scale, y);
        }
    }
    if (highlighted.Inside(h, w))
    {
        QPen pen;
        pen.setWidth(1);
        pen.setColor(QColor(255,255,255,100));
        painter.setPen(pen);
        painter.drawEllipse(QPoint(
                border + pixel_scale * (highlighted.col + 0.5),
                border + pixel_scale * (highlighted.row + 0.5)),
                int(3*pixel_scale), int(3*pixel_scale));
//        painter.drawRect(QRect(border + pixel_scale * highlighted.col - 2,
//                           border + pixel_scale * highlighted.row - 2,
//                           pixel_scale + 4, pixel_scale + 4));

    }
}

void MatrixLabel::mousePressEvent(QMouseEvent *event)
{
    int x = (event->x() - border) / pixel_scale;
    int y = (event->y() - border) / pixel_scale;
    if (pixel_data->valid(x, y))
    {
        ClickedCell(Pos(y, x));
    }
}

GridWidget::GridWidget(QWidget* parent) :
        QWidget(parent), selected_occupant(NULL)
{
    grid = new Grid();
    grid->SetSize(sz,sz);

    scroll_area = new QScrollArea;
    matrix_label = new MatrixLabel;

    matrix_label->pixel_scale = 6;
    matrix_label->AllocateImage(sz, sz);
    scroll_area->setWidgetResizable(true);
    scroll_area->setLineWidth(0);
    scroll_area->setFrameStyle(0);
    scroll_area->setWidget(matrix_label);
    scroll_area->updateGeometry();
    // TODO: make scrollbars always appear with
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QSize size = matrix_label->sizeHint();
    scroll_area->setGeometry(0, 0, size.width(), size.height());

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(scroll_area);
    setLayout(layout);

    connect(matrix_label, SIGNAL(ClickedCell(Pos)), this, SLOT(SelectAtPos(Pos)));
}

std::ostream& operator<<(std::ostream& s, QSize size)
{
    s << size.width() << ", " << size.height() << endl;
    return s;
}

QSize GridWidget::sizeHint() const
{
    return matrix_label->sizeHint() + QSize(40,40);// + QSize(50,100);
}

void GridWidget::Rerender()
{
    int draw_type = grid->draw_type;
    QColor color(0, 0, 0);

    if (grid->rows != matrix_label->pixel_data->height() ||
        grid->cols != matrix_label->pixel_data->width())
    {
        matrix_label->AllocateImage(grid->rows, grid->cols);
        return;
    }

    for (int i = 0; i < grid->rows; i++)
    {
        QRgb* line1 = reinterpret_cast<QRgb*>(matrix_label->pixel_data->scanLine(i));
        float* line2 = grid->energy[i];
        for (int j = 0; j < grid->cols; j++)
        {
            int val = *line2++ * 10;

            if (val > 0) val = 10 * log(1 + val);
            if (val > 255) val = 255;
            if (val < -200) val = -200;
            if (val > 0) color.setRgb(val, val, val);
            else color.setRgb(10 - val, 0, 0);

            Occupant* occ = grid->OccupantAt(Pos(i,j));

            Creat* creat = dynamic_cast<Creat*>(occ);
            if (creat)
            {
                switch (draw_type)
                {
                case DrawAge: {
                    float stage = float(creat->age) / grid->max_age;
                    if (stage < 0.3) color.setRgb(0,128,0);
                    else if (stage < 0.6) color.setRgb(128,128,0);
                    else if (stage < 0.95) color.setRgb(128,0,0);
                    else if (stage < 1.0) color.setRgb(250,250,250);
                } break;
                case DrawEnergy:
                    val = creat->energy * 6;
                    if (val > 255) val = 255;
                    color.setRgb(val, val, val);
                    break;
                case DrawColor:
                {
                    int hue = int(255 * 255 + creat->marker * 255) % 255;
                    if (hue > 255) hue = 255;
                    if (hue < 0) hue = 0;
                    color.setHsv(hue, 220, 220);
                } break;
                case DrawAction:
                {
                    switch (creat->action)
                    {
                    case ActionNone: color.setRgb(150, 50, 50); break;
                    case ActionForward: color.setRgb(150, 150, 150); break;
                    case ActionLeft: color.setRgb(180, 120, 50); break;
                    case ActionRight: color.setRgb(120, 50, 180); break;
                    case ActionReproduce: color.setRgb(0, 255, 0); break;
                    }
                    if (creat->interacted) color.setRgb(255, 0, 0);
                }}
            } else if (Block* block = dynamic_cast<Block*>(occ))
            {
                color.setHsv(255 * block->draw_hue, 110, 255);
            }
            if (!creat && grid->draw_creats_only) color.setRgb(0,0,0);
            *line1++ = color.rgb();
        }
    }
}

void GridWidget::SelectAtPos(Pos pos)
{
    Occupant* occ = grid->OccupantAt(pos);

    if (occ)
    {
        SelectOccupant(occ);
    } else
    {
        float min_d = 10000;
        Occupant* occ = NULL;
        for_iterate(it, grid->occupant_list)
        {
            Occupant* o = *it;
            float d = (o->pos - pos).Mag();
            if (d < min_d)
            {
                min_d = d;
                occ = o;
            }
        }
        if (min_d < 3)
            SelectOccupant(occ);
        else
            grid->energy(pos) += 5;
    }
    Draw();
}

void GridWidget::UnselectOccupant()
{
    selected_occupant = NULL;
    Draw();
}

void GridWidget::UpdateOccupant()
{
    if (selected_occupant)
    {
        selected_occupant->Update();
        Draw();
    }
}

void GridWidget::SelectOccupant(Occupant *occ)
{
    if (selected_occupant && occ != selected_occupant)
        selected_occupant->DeleteQtHook();

    if (occ && occ != selected_occupant)
    {
        // TODO: make sure s_o hasn't been freed,
        // which might happen if user deletes an
        // occupant

        selected_occupant = occ;

        HookManager* hm = occ->SetupQtHook();
        connect(hm, SIGNAL(value_changed()), this,
                SLOT(Draw()));
        connect(hm, SIGNAL(being_removed()), this,
                SLOT(UnselectOccupant()));
        OccupantSelected(occ);
    }
}

void GridWidget::Step()
{
    grid->Step();
    Draw();
}

void GridWidget::Draw()
{
    if (selected_occupant)
        matrix_label->highlighted = selected_occupant->pos;
    Rerender();
    repaint();
}

void GridWidget::SelectNextOccupant(bool forward)
{
    Pos p = matrix_label->highlighted.Wrap(grid->rows, grid->cols);
    int sz = grid->rows * grid->cols;
    int start = p.row * grid->cols + p.col;
    int index = start + (forward ? 1 : -1);
    do {
        Occupant* occ = grid->occupant_grid[index];
        if (occ)
        {
            SelectOccupant(occ);
            return;
        }
        index += (forward ? 1 : -1);
        index += sz;
        index %= sz;
    } while (index != start);
    Draw();
}

