#include "widgets.hpp"
#include "qworld.hpp"
#include "../src/block.hpp"
#include "../src/world.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <QScrollArea>
#include <QGridLayout>
#include <QScrollBar>

using namespace std;

RegisterClass(QWorld, Object);
RegisterBinding(QWorld, draw_type, "color by", "action\nage\nenergy\nplumage\n# children\n# interacts");
RegisterBinding(QWorld, draw_creats, "draw creats");
RegisterBinding(QWorld, draw_energy, "draw energy");
RegisterBinding(QWorld, draw_blocks, "draw blocks");
RegisterBinding(QWorld, draw_block_colors, "color blocks");
RegisterBinding(QWorld, draw_hue_multiplier, "hue multiplier", 0.1, 10.0, 0.1);

QWorld::QWorld(QWidget* parent) :
        QWidget(parent),
        selected_occupant(NULL)
{
    world = new World();

    scroll_area = new QScrollArea;
    grid = new MatrixView(6, false, false);
    grid->matrix = &world->energy;

    hover_mode = true;

    draw_type = DrawAction;
    draw_hue_multiplier = 1.0;
    draw_creats = true;
    draw_blocks = true;
    draw_energy = true;
    draw_block_colors = true;

    scroll_area->setWidgetResizable(true);
    scroll_area->setAlignment(Qt::AlignCenter);
    scroll_area->setLineWidth(0);
    scroll_area->setFrameStyle(0);
    scroll_area->setWidget(grid);
    scroll_area->updateGeometry();

    //scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QSize size = grid->sizeHint();
    scroll_area->setGeometry(0, 0, size.width(), size.height());

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(scroll_area);
    setLayout(layout);

    SetupPanel(false);

    connect(this->panel, SIGNAL(value_changed()), this, SLOT(Draw()));
    connect(grid, SIGNAL(OverPaint(QPainter&)), this, SLOT(OnChildPaint(QPainter&)));
    connect(grid, SIGNAL(ClickedCell(Pos)), this, SLOT(SelectAtPos(Pos)));
    connect(grid, SIGNAL(HoverCell(Pos)), this, SLOT(HoverAtPos(Pos)));
    connect(grid, SIGNAL(WasResized()), this, SLOT(RecenterZoom()));
}

void QWorld::SetSize(int rows, int cols)
{
    if (cols < 100) grid->scale = 12;
    //world->SetSize(rows, cols);
    //setMaximumSize(sizeHint());
}

std::ostream& operator<<(std::ostream& s, QSize size)
{
    s << size.width() << ", " << size.height() << endl;
    return s;
}

QSize QWorld::sizeHint() const
{
    return grid->sizeHint() + QSize(25,25);// + QSize(50,100);
}

void QWorld::SelectAtPos(Pos pos)
{
    Occupant* occ = world->OccupantAt(pos);

    if (hover_mode == false && occ)
    {
        if (selected_occupant == occ) {
            UnselectOccupant();
            hover_mode = true;
        } else
            SelectOccupant(occ);
    } else if (occ)
    {
        hover_mode = false;
        SelectOccupant(occ);
    } else {
        hover_mode = true;
        CellClicked(pos);
    }

    Draw();
}

void QWorld::AfterStep()
{
    if (selected_occupant)
    {
        if (hover_mode) HoverAtPos(grid->recticule);
        else grid->recticule = selected_occupant->pos;
    }
}

void QWorld::DragToPos(Pos pos)
{
    if (selected_occupant && !world->OccupantAt(pos))
    {
        selected_occupant->Move(pos);
        selected_occupant->last_pos = pos;
        grid->recticule = pos;
        if (Creat* creat = dynamic_cast<Creat*>(selected_occupant))
        {
            creat->UpdateBrain();
            creat->UpdateQtHook();
        }
    }

    Draw();
}

void QWorld::HoverAtPos(Pos pos)
{
    if (grid->dragging) { DragToPos(pos); return; }
    if (!hover_mode) return;

    Occupant* occ = world->OccupantAt(pos);

    if (occ && occ->pos != grid->recticule)
    {
        UnselectOccupant();
        SelectOccupant(occ);
    } else if (!occ)
    {
        UnselectOccupant();
        grid->recticule.row = -1;
        grid->recticule.col = -1;
    }

    Draw();
}

void QWorld::UnselectOccupant()
{
    Occupant* occ = selected_occupant;
    selected_occupant = NULL;
    if (occ)
        occ->DeleteQtHook();
    Draw();
}

void QWorld::SelectedOccupantRemoved()
{
    selected_occupant = NULL;
    grid->recticule = Pos(-1,-1);
    hover_mode = true;
    Draw();
}

void QWorld::UpdateOccupant()
{
    if (selected_occupant)
    {
        SetDrawFraction(1.0);

        foreach(Occupant* occ, world->occupant_list)
            occ->last_pos = occ->pos;

        QMutableLinkedListIterator<Occupant*> i(world->occupant_list);
        while (i.hasNext())
        {
            Occupant* occ = i.next();
            if (occ == selected_occupant) occ->Update();
            else if (Creat* creat = dynamic_cast<Creat*>(occ))
            {
                if (!creat->alive) creat->Remove();
                else if (creat->energy < 0) creat->alive = false;
            }
            occ->UpdateQtHook();
        }

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
    if (scale * 2 < grid->scale && scroll_area->horizontalScrollBar()->maximum() == 0 &&
        scroll_area->verticalScrollBar()->maximum() == 0) return;

    tmp_x = getScrollBarFraction(scroll_area->horizontalScrollBar());
    tmp_y = getScrollBarFraction(scroll_area->verticalScrollBar());
    grid->scale = scale * 2;
    grid->setMaximumSize(sizeHint());

    updateGeometry();
}

void QWorld::RecenterZoom()
{
    setScrollBarFraction(scroll_area->horizontalScrollBar(), tmp_x);
    setScrollBarFraction(scroll_area->verticalScrollBar(), tmp_y);
}

int QWorld::CurrentZoom()
{
    return grid->scale / 2;
}

void QWorld::OnChildPaint(QPainter& painter)
{
    int scale = grid->scale;
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
    painter.translate(grid->border, grid->border);
    painter.scale(scale, scale);

    foreach(Occupant* occ, world->occupant_list)
    {
        if (!occ->solid) continue;

        Pos pos2 = occ->pos;
        Pos pos1 = occ->last_pos;
        if (draw_fraction != 1.0 && abs(pos1.row - pos2.row) > 20)
        {
            if (pos1.row < world->rows/2) pos1.row += world->rows;
            else pos1.row -= world->rows;
        }
        if (draw_fraction != 1.0 && abs(pos1.col - pos2.col) > 20)
        {
            if (pos1.col < world->cols/2) pos1.col += world->cols;
            else pos1.col -= world->cols;
        }

        float x = pos2.col * draw_fraction + pos1.col * (1 - draw_fraction);
        float y = pos2.row * draw_fraction + pos1.row * (1 - draw_fraction);

        Creat* creat = dynamic_cast<Creat*>(occ);
        Block* block = dynamic_cast<Block*>(occ);

        if (creat && draw_creats)
        {
            switch (draw_type) {
            case DrawAge: {
                    float stage = float(creat->age) / creat->max_age;
                    int hue = 255 * (0.4 * (1 - stage));
                    if (hue < 0) hue = 0;
                    color.setHsv(hue, 240, 240);
                    if (!creat->alive) color.setRgb(255,255,255);
                } break;
            case DrawEnergy: {
                    float stage = float(creat->energy) / world->action_cost[ActionReproduce];
                    int value = 240;
                    if (stage < 0) stage = 0;
                    if (stage < 0.4) { value *= (0.5 + stage / 0.8); stage = 0.4; }
                    int hue = 255 * (0.75 + stage * 0.4);
                    if (hue > 350) hue = 350;
                    color.setHsv(hue, 240, value);
                } break;
            case DrawColor: {
                    int hue = int(360 * 100 + (draw_hue_multiplier * creat->marker) * 360) % 360;
                    if (hue > 360) hue = 360;
                    if (hue < 0) hue = 0;
                    color.setHsv(hue, 220, 220);
                } break;
            case DrawAction: {
                    switch (creat->action)
                    {
                    case ActionNone: color.setRgb(100, 100, 100); break;
                    case ActionForward: color.setRgb(50, 50, 240); break;
                    case ActionLeft: color.setRgb(50, 120, 180); break;
                    case ActionRight: color.setRgb(120, 50, 200); break;
                    case ActionReproduce: color.setRgb(0, 255, 0); break;
                    }
                    if (creat->interacted) color.setRgb(255, 150, 0);
                    if (!creat->alive) color.setRgb(255,0,0);
                } break;
            case DrawNumberOfChildren: {
                    color.setHsv(creat->children * 60, 220, 200);
                } break;
            case DrawNumberOfInteractions: {
                    color.setHsv(creat->interaction_count * 60, 220, 200);
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
            else color.setRgb(250,250,250);

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

    if (update_rest) world->occupant_list.removeAll(creat);
    if (update_rest)
    {
        Step();
        world->occupant_list.push_back(creat);
    }
    (creat->*(world->action_lookup[creat->action]))();
    creat->UpdateInputs();
    creat->UpdateBrain();
    creat->UpdateQtHook();

    SetDrawFraction(1.0);
    Draw();
}

void QWorld::SelectOccupant(Occupant *occ)
{
    if (selected_occupant && occ != selected_occupant)
        UnselectOccupant();

    if (occ && occ != selected_occupant)
    {
        // TODO: make sure s_o hasn't been freed,
        // which might happen if user deletes an
        // occupant

        selected_occupant = occ;

        BindingsPanel* hm = occ->SetupPanel(true);
        connect(hm, SIGNAL(value_changed()), this,
                SLOT(Draw()));
        connect(hm, SIGNAL(being_removed()), this,
                SLOT(SelectedOccupantRemoved()));
        OccupantSelected(occ);

        grid->recticule = occ->pos;
    }
}

void QWorld::Step()
{
    world->Step();
    world->UpdateQtHook();
    AfterStep();
    Draw();
}

void QWorld::InvisibleStep()
{
    world->Step();
}

void QWorld::SetDrawFraction(float frac)
{
    draw_fraction = frac;
}

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


void QWorld::Draw()
{
    grid->color_func = draw_energy ? &WhiteBlueColorFunc : &BlackColorFunc;
    update();
}

void QWorld::SelectNextOccupant(bool forward)
{
    Pos p = world->Wrap(grid->recticule);
    int sz = world->rows * world->cols;
    int start = p.row * world->cols + p.col;
    int index = start;
    do {
        index += (forward ? 1 : -1);
        index += sz;
        index %= sz;
        Occupant* occ = world->occupant_grid[index];
        if (occ)
        {
            SelectOccupant(occ);
            break;
        }
    } while (index != start);
    Draw();
}
