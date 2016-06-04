#pragma once
#include "songframe.h"
#include <QWidget>
#include <QMediaPlayer>
#include <actions/changefile.h>

namespace Ui {
class SongInfo;
}

class SongInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SongInfo(QWidget *parent = 0);
    virtual ~SongInfo();
    QVideoWidget* videoWidget();

public slots:
    void setSelection(QList<SongFrame*> *selected);
    void selectionUpdated();
    void selectionChanged();
    void updateVideoInfo(QSize resolution, QString codec);

private slots:
    void on_title_textChanged(const QString &arg1);
    void on_artist_textChanged(const QString &arg1);
    void updateFile();
    void highlightText(int from, int to);

signals:
    void seek(quint64 pos);
    void play();
    void pause();
    void popOut();
    void reloadMedia();
private:
    Ui::SongInfo* ui;
    Song* song_;
    QMetaObject::Connection conSylText,conSylLine,conSyl,conUpdate;
    QList<SongFrame*> *selection;
    void handleUpdate(QString title, QString filter, const QFileInfo& currentFile, Song::ChangeFile::FileType type);
};
