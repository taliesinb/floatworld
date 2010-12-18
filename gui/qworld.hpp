#ifndef QWORLD_HPP
#define QWORLD_HPP

#include <QtGui/QWidget>
#include "src/metaclass.hpp"
#include "src/pos.hpp"

class World;
class Occupant;
class QScrollArea;
class QFormLayout;
class MatrixView;

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
    World* world;
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
    void CellClicked(Pos pos);
};

#endif // QWORLD_HPP
