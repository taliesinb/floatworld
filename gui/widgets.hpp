#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtGui/QWidget>

#include "src/pos.hpp"
#include "src/metaclass.hpp"

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

    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);

signals:

    void WasResized();
    void OverPaint(QPainter&);
    void ClickedCell(Pos pos);

    friend class GridWidget;

};

class World;
class Occupant;
class QScrollArea;
class QFormLayout;

class QWorld : public QWidget, public Object
{
    Q_OBJECT    

private:
    QScrollArea* scroll_area;
    MatrixView* energy;
    float draw_fraction;
    float tmp_x, tmp_y;

public:
    enum {
        DrawAction,
        DrawAge,
        DrawEnergy,
        DrawColor
    };
    int draw_type;
    bool draw_creats;
    bool draw_blocks;
    bool draw_energy;
    bool draw_block_colors;

public:
    World* grid;
    QWorld(QWidget* parent = NULL);

    Occupant* selected_occupant;
    void SelectOccupant(Occupant* occ);
    void SelectNextOccupant(bool forward);
    QSize sizeHint() const;

    void ConstructSettingsPanel(QFormLayout* layout);

    void SetZoom(int scale);
    int CurrentZoom();

    void keyReleaseEvent(QKeyEvent *);

public slots:
    void Step();
    void Draw();
    void SetDrawFraction(float frac);
    void SelectAtPos(Pos pos);
    void UnselectOccupant();
    void UpdateOccupant();
    void OnChildPaint(QPainter&);
    void RecenterZoom();

signals:
    void OccupantSelected(Occupant*);
};


#endif // WIDGETS_HPP
