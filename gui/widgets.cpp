#include "widgets.hpp"

#include <QPainter>
#include <QMouseEvent>

using namespace std;

int sz = 80;

GridWidget::GridWidget(QWidget* parent)
        : QWidget(parent),
        grid(),
        pixmap(sz, sz, QImage::Format_RGB32)
{
    grid.SetSize(sz,sz);
    renders = 0;
    scale = 6;
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    //setAttribute(Qt::WA_NoSystemBackground);
    this->setFixedSize(grid.rows * scale, grid.cols * scale);
}

// when painting, seems better to use scaled drawImage
//#define WOOP
void GridWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(QRect(0,0,scale*sz,scale*sz),pixmap,QRect(0,0,sz,sz));
    //painter.drawPixmap(0,0,200,200,pixmap);
}

void GridWidget::rerender()
{
    int draw_type = grid.draw_type;
    QColor color(255, 0, 0);
    for (int i = 0; i < grid.rows; i++)
    {
        QRgb* line1 = reinterpret_cast<QRgb*>(pixmap.scanLine(i));
        float* line2 = grid.energy[i];
        for (int j = 0; j < grid.cols; j++)
        {
            int val = *line2++ * 10;

            if (val > 0) val = 10 * log(1 + val);
            if (val > 255) val = 255;
            if (val > 0) color.setRgb(val, val, val);
            else color.setRgb(10 - val, 0, 0);

            Occupant* occ = grid.OccupantAt(Pos(i,j));

            Creat* creat = dynamic_cast<Creat*>(occ);
            if (creat)
            {
                switch (draw_type)
                {
                case DrawAge: {
                    float stage = float(creat->age) / grid.max_age;
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
                    color.setHsv(int(255 * 255 + creat->marker * 255) % 255, 220, 220);
                    break;
                case DrawAction:
                    switch (creat->action)
                    {
                    case ActionNone: color.setRgb(150, 50, 50); break;
                    case ActionForward: color.setRgb(80, 80, 80); break;
                    case ActionLeft: color.setRgb(180, 120, 50); break;
                    case ActionRight: color.setRgb(120, 50, 180); break;
                    case ActionReproduce: color.setRgb(0, 255, 0); break;
                    }
                }
            } else if (dynamic_cast<Block*>(occ))
            {
                color.setRgb(0, 200, 0);
            }
            if (!creat && grid.draw_creats_only) color.setRgb(0,0,0);
            if (occ && occ->qt_hook) color = color.lighter(200);
            *line1++ = color.rgb();
        }
    }
    renders++;
}

void GridWidget::mousePressEvent(QMouseEvent *event)
{
    int x = event->x() / scale;
    int y = event->y() / scale;
    Pos pos(y,x);
    if (pos.Inside(grid.rows, grid.cols))
    {
        ClickedCell(pos);
    }
}
