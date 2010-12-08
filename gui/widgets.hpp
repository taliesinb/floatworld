#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtGui/QWidget>
#include <QPixmap>
#include <QImage>

#include "../src/grid.hpp"

class MatrixLabel : public QWidget
{
    Q_OBJECT

public:
    QImage* pixel_data;
    float pixel_scale;
    int render_count; /* for peformance debugging */

    MatrixLabel(QWidget* parent);

    virtual void Rerender();
    void AllocateImage(int rows, int cols);

protected:

    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);

signals:

    void ClickedCell(Pos pos);

};

class GridWidget : public MatrixLabel
{
    Q_OBJECT

public:
    Grid grid;
    GridWidget(QWidget* parent);

    void Rerender();
};


#endif // WIDGETS_HPP
