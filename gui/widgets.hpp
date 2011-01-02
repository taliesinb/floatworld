#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtGui/QWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QStringList>

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
    int border;
    bool draw_grid;
    bool draw_flipped;
    Pos highlighted;
    Pos last_hover;

    MatrixView(int size, bool flip, bool grid);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:

    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

signals:

    void WasResized();
    void OverPaint(QPainter&);
    void ClickedCell(Pos pos);
    void HoverCell(Pos pos);

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

class EnumLabel : public QLabel, public Binding
{
public:
    QStringList labels;
    EnumLabel(const char* labels);
    virtual void Synchronize(bool inbound);
};

class RNGWidget : public QLabel, public Binding
{
    Q_OBJECT;

public:
    RNGWidget();
    virtual void Synchronize(bool inbound);
    virtual void mousePressEvent(QMouseEvent *ev);

signals:
    void Randomized();
};

class MatrixWidget : public MatrixView, public Binding
{
    Q_OBJECT

    QStringList row_labels;
    QStringList col_labels;
    bool flipped;
    int rows, cols;

public:
    MatrixWidget(int pixel_size, bool flip, QString row_labels, QString column_labels);
    virtual void OnSetPointer();
    virtual void Synchronize(bool inbound);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void ShowTooltip(Pos p);

};

class BindingsPanel : public QWidget
{
    Q_OBJECT

public:

    Class* mclass;
    Object* object;
    QFormLayout* layout;
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
    void CreateTitle();
};



#endif // WIDGETS_HPP
