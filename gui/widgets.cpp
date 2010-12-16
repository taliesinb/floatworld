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

/* IGNORE FOR NOW
RegisterVar(QGrid, draw_type);
RegisterVar(QGrid, draw_creats);
RegisterVar(QGrid, draw_blocks);
RegisterVar(QGrid, draw_energy);
*/
RegisterClass(QWorld, None);
RegisterQtHook(QWorld, draw_type, "color by", BindEnum("Action\nAge\nEnergy\nPlumage"));
RegisterQtHook(QWorld, draw_creats, "draw creats", BindBool());
RegisterQtHook(QWorld, draw_energy, "draw energy", BindBool());
RegisterQtHook(QWorld, draw_blocks, "draw blocks", BindBool());
RegisterQtHook(QWorld, draw_block_colors, "color blocks", BindBool());

int sz = 120;
int border = 3;

QRgb BlackColorFunc(float value)
{
    return qRgb(0,0,0);
}

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

QWorld::QWorld(QWidget* parent) :
        QWidget(parent),
        selected_occupant(NULL)
{
    grid = new World();
    grid->SetSize(sz,sz);

    scroll_area = new QScrollArea;
    energy = new MatrixView(4, false, false);
    energy->matrix = &grid->energy;

    draw_type = DrawAction;
    draw_creats = true;
    draw_blocks = true;
    draw_energy = true;
    draw_block_colors = true;

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

    SetupQtHook(false);

    connect(this->panel, SIGNAL(value_changed()), this, SLOT(Draw()));
    connect(energy, SIGNAL(OverPaint(QPainter&)), this, SLOT(OnChildPaint(QPainter&)));
    connect(energy, SIGNAL(ClickedCell(Pos)), this, SLOT(SelectAtPos(Pos)));
    connect(energy, SIGNAL(WasResized()), this, SLOT(RecenterZoom()));
}

std::ostream& operator<<(std::ostream& s, QSize size)
{
    s << size.width() << ", " << size.height() << endl;
    return s;
}

QSize QWorld::sizeHint() const
{
    return energy->sizeHint() + QSize(40,40);// + QSize(50,100);
}

void QWorld::SelectAtPos(Pos pos)
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

void QWorld::UnselectOccupant()
{
    selected_occupant = NULL;
    Draw();
}

void QWorld::UpdateOccupant()
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

void QWorld::SetZoom(int scale)
{
    tmp_x = getScrollBarFraction(scroll_area->horizontalScrollBar());
    tmp_y = getScrollBarFraction(scroll_area->verticalScrollBar());
    energy->scale = scale * 2;
    energy->setMaximumSize(sizeHint());
    updateGeometry();
}

void QWorld::RecenterZoom()
{
    setScrollBarFraction(scroll_area->horizontalScrollBar(), tmp_x);
    setScrollBarFraction(scroll_area->verticalScrollBar(), tmp_y);
}

int QWorld::CurrentZoom()
{
    return energy->scale / 2;
}

void QWorld::OnChildPaint(QPainter& painter)
{
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

        if (!occ->solid) continue;

        Pos pos2 = occ->pos;
        Pos pos1 = occ->last_pos;
        if (draw_fraction != 1.0 && abs(pos1.row - pos2.row) > 20)
        {
            if (pos1.row < grid->rows/2) pos1.row += grid->rows;
            else pos1.row -= grid->rows;
        }
        if (draw_fraction != 1.0 && abs(pos1.col - pos2.col) > 20)
        {
            if (pos1.col < grid->cols/2) pos1.col += grid->cols;
            else pos1.col -= grid->cols;
        }

        float x = pos2.col * draw_fraction + pos1.col * (1 - draw_fraction);
        float y = pos2.row * draw_fraction + pos1.row * (1 - draw_fraction);

        Creat* creat = dynamic_cast<Creat*>(occ);
        Block* block = dynamic_cast<Block*>(occ);

        if (creat && draw_creats)
        {
            switch (draw_type) {
            case DrawAge: {
                    float stage = float(creat->age) / grid->max_age;
                    int hue = 255 * (0.3 * (1 - stage));
                    if (hue > 255) hue = 255;
                    if (hue < 0) hue = 0;
                    color.setHsv(hue, 240, 240);
                } break;
            case DrawEnergy: {
                    float stage = float(creat->energy) / 100;
                    int hue = 255 * (0.75 + stage * 0.4);
                    if (hue > 350) hue = 350;
                    color.setHsv(hue, 240, 240);
                } break;
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
                } break;
            }
            if (poly)
            {
                painter.save();
                painter.setPen(color);
                painter.setBrush(QBrush(color));
                painter.translate(x + 0.5, y + 0.5);
                int orient2 = creat->orient;
                int orient1 = creat->last_orient;
                if (orient1 == 3 && orient2 == 0) orient1 = -1;
                if (orient1 == 0 && orient2 == 3) orient1 = 4;
                painter.rotate((orient2 * draw_fraction + orient1 * (1 - draw_fraction)) * 90);
                painter.drawPolygon(tri);
                painter.restore();
            } else
                painter.fillRect(QRectF(x, y, 1.0 - 1./scale, 1.0 - 1./scale), color);

        } else if (block && draw_blocks)
        {
            if (draw_block_colors) color.setHsv(255 * block->draw_hue, 200, 255);
            else color.setRgb(100,100,100);

            if (poly) {
                painter.save();
                painter.setPen(blockpen);
                blockpen.setColor(color);
                float off = 1./scale;
                if (block->draw_filled)
                    painter.fillRect(QRectF(x + off, y + off, 1.0 - 2 * off, 1.0 - 2 * off), color);
                else
                    painter.drawRect(QRectF(x + off, y + off , 1.0 - 2 * off, 1.0 - 2 * off));
                painter.restore();
            } else
                painter.fillRect(QRectF(x, y, 1.0 - 1./scale, 1.0 - 1./scale), color);
        }
    }
    painter.restore();
}

void QWorld::keyReleaseEvent(QKeyEvent* event)
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

    creat->last_pos = creat->pos;
    creat->last_orient = creat->orient;

    if (update_rest) grid->occupant_list.remove(creat);
    if (update_rest)
    {
        Step();
        grid->occupant_list.push_back(creat);
    }
    (creat->*(grid->action_lookup[creat->action]))();
    creat->UpdateQtHook();

    SetDrawFraction(1.0);
    Draw();
}

void QWorld::SelectOccupant(Occupant *occ)
{
    if (selected_occupant && occ != selected_occupant)
        selected_occupant->DeleteQtHook();

    if (occ && occ != selected_occupant)
    {
        // TODO: make sure s_o hasn't been freed,
        // which might happen if user deletes an
        // occupant

        selected_occupant = occ;

        BindingsPanel* hm = occ->SetupQtHook(true);
        connect(hm, SIGNAL(value_changed()), this,
                SLOT(Draw()));
        connect(hm, SIGNAL(being_removed()), this,
                SLOT(UnselectOccupant()));
        OccupantSelected(occ);
    }
}

void QWorld::Step()
{
    grid->Step();
    Draw();
}

void QWorld::SetDrawFraction(float frac)
{
    draw_fraction = frac;
}

void QWorld::Draw()
{
    energy->color_func = draw_energy ? &WhiteBlueColorFunc : &BlackColorFunc;

    if (selected_occupant)
        energy->highlighted = selected_occupant->pos;
    repaint();
}

void QWorld::SelectNextOccupant(bool forward)
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

