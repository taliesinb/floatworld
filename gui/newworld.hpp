#ifndef NEWWORLD_HPP
#define NEWWORLD_HPP

#include <QDialog>
#include <QString>
#include <QMap>
#include <QListWidgetItem>

#include "../src/world.hpp"

namespace Ui {
    class NewWorldDialog;
}

class ObjectListItem : public QListWidgetItem
{
public:
    int number;
    Object* prototype;
    Class* mclass;

    ObjectListItem();
    ObjectListItem(Class* mc);
    ~ObjectListItem();

    void UpdateLabel();
    void SetNumber(int num);
};

std::ostream& operator<<(std::ostream&, ObjectListItem*&);
std::istream& operator>>(std::istream&, ObjectListItem*&);

class NewWorldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewWorldDialog(QWidget *parent = 0);
    ~NewWorldDialog();

    Object* selected_object;

    bool is_start;
    QLinkedList<ObjectListItem*> AllItems();
    ObjectListItem* CurrentItem();
    World* GetWorld();

    void CreateDefaultObjects();
    void LoadFromStream(std::istream&);
    void SafeToStream(std::ostream&);

private:
    Ui::NewWorldDialog *ui;

public slots:
    void WorldSizeChanged();
    void AddObject();
    void RemoveObject();
    void SelectObject(QListWidgetItem*, QListWidgetItem* old=NULL);
    void DeselectObject();
    void SetObjectNumber(int num);
    void CopyObject();

    void CreateWorld();
    void SaveTemplate();
    void LoadTemplate();

    friend std::ostream& operator<<(std::ostream&, NewWorldDialog*);
    friend std::istream& operator>>(std::istream&, NewWorldDialog*);
};

std::ostream& operator<<(std::ostream&, NewWorldDialog*);
std::istream& operator>>(std::istream&, NewWorldDialog*);


#endif // NEWWORLD_HPP
