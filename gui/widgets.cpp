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
    draw_type = draw_plain;
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
    QColor color(255, 0, 0);
    for (int i = 0; i < grid.rows; i++)
    {
        QRgb* line1 = reinterpret_cast<QRgb*>(pixmap.scanLine(i));
        float* line2 = grid.energy[i];
        for (int j = 0; j < grid.cols; j++)
        {
            int val = *line2++ * 5;
            color.setRgb(val,val,val);

            Occupant* occ = grid.OccupantAt(Pos(i,j));

            if (dynamic_cast<Block*>(occ))
            {
                color.setRgb(0, 200, 0);
            } else
            if (Creat* creat = dynamic_cast<Creat*>(occ))
            {
                float intensity = 255;
                if (draw_type == draw_age)
                {
                    float stage = float(creat->age) / grid.max_age;
                    if (stage < 0.3) color.setRgb(0,128,0);
                    else if (stage < 0.6) color.setRgb(128,128,0);
                    else if (stage < 0.95) color.setRgb(128,0,0);
                    else if (stage < 1.0) color.setRgb(250,250,250);
                } else {
                    if (draw_type == draw_energy) intensity = creat->energy * 6;
                    if (draw_type == draw_color)  color.setHsv(int(255 * 255 + creat->marker * 255) % 255, 220, 220);
                    else color.setRgb(intensity > 255 ? 255 : intensity, 0, 0);
                }
            }
            if (occ && occ->qt_hook) color = color.lighter(150);
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
