#include "widgets.hpp"

#include <QPainter>

int sz = 80;

GridWidget::GridWidget(QWidget* parent)
        : QWidget(parent),
        grid(sz, sz),
        pixmap(sz, sz, QImage::Format_RGB32)
{
    renders = 0;
    scale = 4;
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
            if (Creat* creat = dynamic_cast<Creat*>(grid.OccupantAt(Pos(i,j))))
            {
                float intensity = 255;
                if (draw_type == draw_age)
                {
                    float stage = float(creat->age) / Creat::maxage;
                    if (stage < 0.3) color.setRgb(0,128,0);
                    else if (stage < 0.6) color.setRgb(128,128,0);
                    else if (stage < 0.95) color.setRgb(128,0,0);
                    else if (stage < 1.0) color.setRgb(250,250,250);
                } else {
                    if (draw_type == draw_energy) intensity = creat->energy * 6;
                    if (draw_type == draw_color)  color.setHsv((int(creat->marker * 20) + (100 * 255)) % 255, 220, 220);
                    else color.setRgb(intensity > 255 ? 255 : intensity, 0, 0);
                }

                *line1++ = color.rgb();
            } else
                *line1++ = qRgb(val,val,val);
        }
    }
    renders++;
}

