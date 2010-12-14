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
#include <QScrollBar>

using namespace std;

int sz = 120;
int border = 3;

QRgb WhiteBlueColorFunc(float value)
{
    int val = value * 10;
    if (val > 0) val = 10 * log(1 + val);
    if (val > 255) val = 255;
    if (val < -200) val = -200;
    if (val > 0) return qRgb(val, val, val);
    else return qRgb(10 - val, 0, 0);
}

MatrixView::MatrixView(int size, bool flip, bool grid)
{
    //setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
    color_func = &WhiteBlueColorFunc;
    draw_flipped = flip;
    matrix = NULL;
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

QGrid::QGrid(QWidget* parent) :
        QWidget(parent),
        selected_occupant(NULL)
{
    grid = new Grid();
    grid->SetSize(sz,sz);

    scroll_area = new QScrollArea;
    energy = new MatrixView(4, false, false);
    energy->matrix = &grid->energy;

    scroll_area->setWidgetResizable(true);
    scroll_area->setLineWidth(0);
    scroll_area->setFrameStyle(0);
    scroll_area->setWidget(energy);
    scroll_area->updateGeometry();

    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QSize size = energy->sizeHint();
    scroll_area->setGeometry(0, 0, size.width(), size.height());

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(scroll_area);
    setLayout(layout);

    connect(energy, SIGNAL(OverPaint(QPainter&)), this, SLOT(OnChildPaint(QPainter&)));
    connect(energy, SIGNAL(ClickedCell(Pos)), this, SLOT(SelectAtPos(Pos)));
    connect(energy, SIGNAL(WasResized()), this, SLOT(RecenterZoom()));
}

std::ostream& operator<<(std::ostream& s, QSize size)
{
    s << size.width() << ", " << size.height() << endl;
    return s;
}

QSize QGrid::sizeHint() const
{
    return energy->sizeHint() + QSize(40,40);// + QSize(50,100);
}


void QGrid::SelectAtPos(Pos pos)
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

void QGrid::UnselectOccupant()
{
    selected_occupant = NULL;
    Draw();
}

void QGrid::UpdateOccupant()
{
    if (selected_occupant)
    {
        selected_occupant->Update();
        Draw();
    }
}

float getScrollBarFraction(QScrollBar* s)
{
    float step = s->pageStep();
    float range = s->maximum() - s->minimum() + step;
    if (range < 1) return 0.5;
    return (s->value() + step/2.0 - s->minimum()) / range;
}

void setScrollBarFraction(QScrollBar* s, float frac)
{
    float step = s->pageStep();
    float range = s->maximum() - s->minimum() + step;
    s->setValue(frac * range - step/2.0 + s->minimum());
}

void QGrid::SetZoom(int scale)
{
    tmp_x = getScrollBarFraction(scroll_area->horizontalScrollBar());
    tmp_y = getScrollBarFraction(scroll_area->verticalScrollBar());
    energy->scale = scale * 2;
    energy->setMaximumSize(sizeHint());
    updateGeometry();
}

void QGrid::RecenterZoom()
{
    setScrollBarFraction(scroll_area->horizontalScrollBar(), tmp_x);
    setScrollBarFraction(scroll_area->verticalScrollBar(), tmp_y);
}

int QGrid::CurrentZoom()
{
    return energy->scale / 2;
}

void QGrid::OnChildPaint(QPainter& painter)
{
    int draw_type = grid->draw_type;
    int scale = energy->scale;
    bool poly = scale > 6;

    QColor color;
    QPolygonF tri;
    float pi = 3.1415;
    float z = -2.0;
    tri << QPointF(sin(0), cos(0))/z;
    tri << QPointF(0.8 * sin(2 * pi / 3), cos(2 * pi / 3))/z;
    tri << QPointF(0.8 * sin(4 * pi / 3), cos(4 * pi / 3))/z;

    QPen blockpen;
    blockpen.setWidthF(1.2);
    blockpen.setCosmetic(true);

    painter.save();
    painter.translate(border, border);
    painter.scale(scale, scale);

    for_iterate(it, grid->occupant_list)
    {
        Occupant* occ = *it;
        Creat* creat = dynamic_cast<Creat*>(occ);
        if (creat)
        {
            switch (draw_type) {
            case DrawAge: {
                    float stage = float(creat->age) / grid->max_age;
                    if (stage < 0.3) color.setRgb(0,128,0);
                    else if (stage < 0.6) color.setRgb(128,128,0);
                    else if (stage < 0.95) color.setRgb(128,0,0);
                    else if (stage < 1.0) color.setRgb(250,250,250);
                } break;
            case DrawEnergy: {
                    int val = creat->energy * 6;
                    if (val > 255) val = 255;
                    color.setRgb(val, val, val);
                    break;
                }
            case DrawColor: {
                    int hue = int(255 * 255 + creat->marker * 255) % 255;
                    if (hue > 255) hue = 255;
                    if (hue < 0) hue = 0;
                    color.setHsv(hue, 220, 220);
                } break;
            case DrawAction: {
                    switch (creat->action)
                    {
                    case ActionNone: color.setRgb(150, 50, 50); break;
                    case ActionForward: color.setRgb(150, 150, 150); break;
                    case ActionLeft: color.setRgb(180, 120, 50); break;
                    case ActionRight: color.setRgb(120, 50, 180); break;
                    case ActionReproduce: color.setRgb(0, 255, 0); break;
                    }
                    if (creat->interacted) color.setRgb(255, 0, 0);
                    break;
                }
            }
            if (poly)
            {
                painter.save();
                painter.setPen(color);
                painter.setBrush(QBrush(color));
                int orient2 = creat->orient;
                int orient1 = creat->last_orient;
                if (orient1 == 3 && orient2 == 0) orient1 = -1;
                if (orient1 == 0 && orient2 == 3) orient1 = 4;
                Pos pos2 = occ->pos;
                Pos pos1 = occ->last_pos;
                if ((pos2 - pos1).Mag() > 1) pos1 = pos2 - Pos(orient2);
                painter.translate((pos2.col * draw_fraction + pos1.col * (1 - draw_fraction)) + 0.5,
                                  (pos2.row * draw_fraction + pos1.row * (1 - draw_fraction)) + 0.5);
                painter.rotate((orient2 * draw_fraction + orient1 * (1 - draw_fraction)) * 90);
                painter.drawPolygon(tri);
                painter.restore();
            }
        } else if (Block* block = dynamic_cast<Block*>(occ))
        {
            color.setHsv(255 * block->draw_hue, 110, 255);
            if (poly) {
                painter.save();
                painter.setPen(blockpen);
                blockpen.setColor(color);
                if (block->draw_filled) painter.setBrush(color.darker());
                painter.drawEllipse(QPointF(block->pos.col + 0.5, block->pos.row + 0.5), 0.45, 0.45);
                painter.restore();
            }
        }
        if (!poly && occ->solid)
            painter.fillRect(QRectF(occ->pos.col, occ->pos.row, 1.0 - 1./scale, 1.0 - 1./scale), color);
    }
    painter.restore();
}

void QGrid::keyReleaseEvent(QKeyEvent* event)
{
    Creat* creat = dynamic_cast<Creat*>(selected_occupant);
    if (!creat) return;

    bool update_rest = !(event->modifiers() & Qt::ShiftModifier);


    int key = event->key();
    if      (key == Qt::Key_A) creat->action = ActionLeft;
    else if (key == Qt::Key_D) creat->action = ActionRight;
    else if (key == Qt::Key_W) creat->action = ActionForward;
    else if (key == Qt::Key_X) creat->action = ActionReproduce;
    else return;

    if (update_rest) grid->occupant_list.remove(creat);

    (creat->*(grid->action_lookup[creat->action]))();
    creat->UpdateQtHook();

    if (update_rest)
    {
        Step();
        grid->occupant_list.push_back(creat);
    }
    Draw();
}

void QGrid::SelectOccupant(Occupant *occ)
{
    if (selected_occupant && occ != selected_occupant)
        selected_occupant->DeleteQtHook();

    if (occ && occ != selected_occupant)
    {
        // TODO: make sure s_o hasn't been freed,
        // which might happen if user deletes an
        // occupant

        selected_occupant = occ;

        HookManager* hm = occ->SetupQtHook(true);
        connect(hm, SIGNAL(value_changed()), this,
                SLOT(Draw()));
        connect(hm, SIGNAL(being_removed()), this,
                SLOT(UnselectOccupant()));
        OccupantSelected(occ);
    }
}

void QGrid::Step()
{
    grid->Step();
    Draw();
}

void QGrid::Draw()
{
    if (selected_occupant)
        energy->highlighted = selected_occupant->pos;
    draw_fraction = 1.0;
    repaint();
}

void QGrid::DrawFraction(float frac)
{
    draw_fraction = frac;
    repaint();
}

void QGrid::SelectNextOccupant(bool forward)
{
    Pos p = energy->highlighted.Wrap(grid->rows, grid->cols);
    int sz = grid->rows * grid->cols;
    int start = p.row * grid->cols + p.col;
    int index = start;
    do {
        index += (forward ? 1 : -1);
        index += sz;
        index %= sz;
        Occupant* occ = grid->occupant_grid[index];
        if (occ)
        {
            SelectOccupant(occ);
            break;
        }
    } while (index != start);
    Draw();
}

