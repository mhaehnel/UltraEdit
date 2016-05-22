#pragma once
#include <QDialog>
#include <QList>
#include <collection.h>

namespace Ui {
class CollectionEditor;
}

class CollectionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CollectionEditor(QList<Collection*> cols, QWidget *parent = 0);
    virtual ~CollectionEditor();

    QList<Collection*> collections() const;
    bool changed() const;

private slots:
    void on_create_clicked();
    void on_remove_clicked();

    void on_toolButton_clicked();

    void checkButton(const QString &);

    void on_collectionList_itemSelectionChanged();

private:
    void updateCollections();
    QList<Collection*> cols_;
    Ui::CollectionEditor* ui;
    bool _changed;
};
