#include "widgets.hpp"
#include "../src/block.hpp"

#include <QPainter>
#include <QMouseEvent>

using namespace std;

int sz = 150;
int border = 3;

MatrixLabel::MatrixLabel(QWidget* parent)
    : QWidget(parent),
    pixel_scale(5),
    pixel_data(NULL),
    render_count(0),
    grid(false), highlighted(-1,-1)
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
    setFixedSize(width * pixel_scale + 2 * border+1, height * pixel_scale + 2 * border+1);
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
    painter.setPen(Qt::black);
    painter.drawRect(QRect(0, 0, w2 + 2 * border, h2 + 2 * border));
    painter.setPen(Qt::white);
    painter.drawRect(QRect(1,1,w2 + 2 * border-2, h2 + 2 * border-2));
    painter.fillRect(QRect(2,2,w2 + 2 * border-3, h2 + 2 * border-3), Qt::white);
    painter.drawImage(QRect(border, border, w2, h2),
                      *pixel_data, QRect(0, 0, w, h));
    if (grid) {
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
        pen.setWidth(2);
        pen.setColor(QColor(255,255,255,100));
        painter.setPen(pen);
        painter.drawRect(QRect(border + pixel_scale * highlighted.col - 2,
                           border + pixel_scale * highlighted.row - 2,
                           pixel_scale + 4, pixel_scale + 4));

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

GridWidget::GridWidget(QWidget* parent)
        : MatrixLabel(parent)
{
    pixel_scale = 4;
    grid.SetSize(sz,sz);
    AllocateImage(sz,sz);
}

void GridWidget::Rerender()
{
    int draw_type = grid.draw_type;
    QColor color(0, 0, 0);

    if (grid.rows != pixel_data->height() ||
        grid.cols != pixel_data->width())
    {
        AllocateImage(grid.rows, grid.cols);
        return;
    }

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
            } else if (Block* block = dynamic_cast<Block*>(occ))
            {
                color.setHsv(255 * block->draw_hue, 110, 255);
            }
            if (!creat && grid.draw_creats_only) color.setRgb(0,0,0);
            *line1++ = color.rgb();
        }
    }
    render_count++;
}
