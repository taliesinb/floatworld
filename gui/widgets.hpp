#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtGui/QWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>

#include "src/binding.hpp"
#include "src/metaclass.hpp"
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

    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);

signals:

    void WasResized();
    void OverPaint(QPainter&);
    void ClickedCell(Pos pos);

    friend class GridWidget;

};

class Class;
class Object;
class Matrix;

class IntLabel : public QLabel, public Binding
{
public:
    IntLabel();

    virtual void Synchronize(bool inbound);
};

class IntWidget : public QSpinBox, public Binding
{
public:
    IntWidget(int min, int max);

    virtual void Synchronize(bool inbound);
};

class FloatWidget : public QDoubleSpinBox, public Binding
{
public:
    FloatWidget(float min, float max, float div);

    virtual void Synchronize(bool inbound);
};

class BoolWidget : public QCheckBox, public Binding
{
public:
    BoolWidget();
    virtual void Synchronize(bool inbound);
};

class EnumWidget : public QComboBox, public Binding
{
public:
    EnumWidget(const char* labels);
    virtual void Synchronize(bool inbound);
};

class MatrixWidget : public MatrixView, public Binding
{
    bool flipped;
    int rows, cols;

public:
    MatrixWidget(int pixel, bool flip);
    virtual void OnSetPointer();
    virtual void Synchronize(bool inbound);
};

class BindingsPanel : public QFormLayout
{
    Q_OBJECT

public:

    Class* mclass;
    Object* object;
    std::list<QWidget*> widgets;

    BindingsPanel(Class* mc, Object* obj);
    virtual ~BindingsPanel();

public slots:
    void child_changed();

signals:
    void value_changed();
    void being_removed();

public:
    void ConstructChildren();
    void UpdateChildren();
};



#endif // WIDGETS_HPP
