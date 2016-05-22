#pragma once

#include <QString>
#include <QDir>
#include <QStringList>

class Song;

class Collection
{
    QString basePath_;
    QString pathRule_;
    QString name_;
    QStringList replacementTags;
    std::vector<Song*> songs_;
public:
    explicit Collection(QString name, QString basePath, QString pathRule);
    virtual ~Collection();
    QString basePath() const { return basePath_; }
    QString name() const { return name_; }
    QString pathRule() const { return pathRule_; }
    QFileInfo path(const Song* s, QString extension) const;
    void removeSong(const Song* s);
    void addSong(Song* s);
    int size() const { return songs_.size(); }
    const std::vector<Song*>& songs() const { return songs_; }
};
