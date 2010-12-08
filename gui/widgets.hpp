#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtGui/QWidget>
#include <QPixmap>
#include <QImage>

#include "../src/grid.hpp"
#include "../src/block.hpp"

class GridWidget : public QWidget
{
    Q_OBJECT

public:
    int renders;
    float scale;
    Grid grid;
    QImage pixmap;
    GridWidget(QWidget* parent);
    Occupant* occ;

    // TODO: destructor

    void rerender();

    void mousePressEvent(QMouseEvent* event);

signals:

    void ClickedCell(Pos pos);

protected:

    void paintEvent(QPaintEvent *event);
};


#endif // WIDGETS_HPP
