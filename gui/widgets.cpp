#include "widgets.hpp"
#include "../src/block.hpp"

#include <QPainter>
#include <QMouseEvent>

using namespace std;

int sz = 80;
int border = 3;

MatrixLabel::MatrixLabel(QWidget* parent)
    : QWidget(parent),
    pixel_scale(5),
    pixel_data(NULL),
    render_count(0)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void MatrixLabel::AllocateImage(int rows, int cols)
{
    if (pixel_data) delete pixel_data;
    pixel_data = new QImage(cols, rows, QImage::Format_RGB32);
    pixel_data->fill(0);
    setFixedSize(cols * pixel_scale + 2 * border, rows * pixel_scale + 2 * border);
    Rerender();
}

void MatrixLabel::Rerender()
{

}

void MatrixLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    int w = pixel_data->width(), h = pixel_data->height();
    int w2 = w * pixel_scale, h2 = h * pixel_scale;
    painter.fillRect(QRect(0, 0, w2 + 2 * border, h2 + 2 * border), Qt::gray);
    painter.drawImage(QRect(border, border, w2, h2),
                      *pixel_data, QRect(0, 0, w, h));
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

GridWidget::GridWidget(QWidget* parent)
        : MatrixLabel(parent)
{
    grid.SetSize(sz,sz);
    AllocateImage(sz,sz);
}

void GridWidget::Rerender()
{
    int draw_type = grid.draw_type;
    QColor color(0, 0, 0);

    for (int i = 0; i < grid.rows; i++)
    {
        QRgb* line1 = reinterpret_cast<QRgb*>(pixel_data->scanLine(i));
        float* line2 = grid.energy[i];
        for (int j = 0; j < grid.cols; j++)
        {
            int val = *line2++ * 10;

            if (val > 0) val = 10 * log(1 + val);
            if (val > 255) val = 255;
            if (val < -200) val = -200;
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
                    case ActionForward: color.setRgb(80, 80, 80); break;
                    case ActionLeft: color.setRgb(180, 120, 50); break;
                    case ActionRight: color.setRgb(120, 50, 180); break;
                    case ActionReproduce: color.setRgb(0, 255, 0); break;
                    }
                }}
            } else if (dynamic_cast<Block*>(occ))
            {
                color.setRgb(0, 200, 0);
            }

            if (!creat && grid.draw_creats_only) color.setRgb(0,0,0);
            if (occ && occ->qt_hook) color = color.lighter(200);
            *line1++ = color.rgb();
        }
    }
    render_count++;
}
