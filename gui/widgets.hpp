#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtGui/QWidget>

#include "src/pos.hpp"

class Matrix;

typedef QRgb (*ColorFunc)(float);

class MatrixView : public QWidget
{
    Q_OBJECT

public:
    ColorFunc color_func;
    Matrix* matrix;
    int scale;
    bool draw_grid;
    bool draw_flipped;
    Pos highlighted;

    MatrixView(int size, bool flip, bool grid);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:

    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);

signals:

    void OverPaint(QPainter&);
    void ClickedCell(Pos pos);

    friend class GridWidget;

};

class Grid;
class Occupant;
class QScrollArea;

class QGrid : public QWidget
{
    Q_OBJECT

private:
    QScrollArea* scroll_area;
    MatrixView* energy;
    float draw_fraction;

public:
    Grid* grid;
    QGrid(QWidget* parent);

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
    void DrawFraction(float frac);
    void SelectAtPos(Pos pos);
    void UnselectOccupant();
    void UpdateOccupant();
    void OnChildPaint(QPainter&);

signals:
    void OccupantSelected(Occupant*);
};


#endif // WIDGETS_HPP
