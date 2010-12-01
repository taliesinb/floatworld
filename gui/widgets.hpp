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

    enum {
        draw_plain,
        draw_age,
        draw_energy,
        draw_color
    } draw_type;

    // TODO: destructor

    void rerender();


protected:
    void paintEvent(QPaintEvent *event);
};


#endif // WIDGETS_HPP
