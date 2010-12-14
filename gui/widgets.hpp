#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtGui/QWidget>

#include "src/pos.hpp"

class QImage;

class MatrixLabel : public QWidget
{
    Q_OBJECT

public:
    QImage* pixel_data;
    float pixel_scale;
    bool draw_grid;
    Pos highlighted;

    MatrixLabel(QWidget* parent);
    MatrixLabel();

    virtual void Rerender();
    void AllocateImage(int rows, int cols);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:

    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);

signals:

    void ClickedCell(Pos pos);

};

class Grid;
class Occupant;
class QScrollArea;

class GridWidget : public QWidget
{
    Q_OBJECT

private:
    QScrollArea* scroll_area;
    MatrixLabel* matrix_label;

public:
    Grid* grid;
    GridWidget(QWidget* parent);

    Occupant* selected_occupant;
    void SelectOccupant(Occupant* occ);
    void SelectNextOccupant(bool forward);
    QSize sizeHint() const;

    void SetZoom(int scale);
    int CurrentZoom();

    void keyReleaseEvent(QKeyEvent *);

public slots:
    void Step();
    void Draw();
    void Rerender();
    void SelectAtPos(Pos pos);
    void UnselectOccupant();
    void UpdateOccupant();

signals:
    void OccupantSelected(Occupant*);
};


#endif // WIDGETS_HPP
