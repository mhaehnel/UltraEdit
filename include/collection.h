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
public:
    explicit Collection(QString name, QString basePath, QString pathRule);
    QString basePath() const { return basePath_; }
    QString name() const { return name_; }
    QString pathRule() const { return pathRule_; }
    QFileInfo path(const Song* s, QString extension) const;
};
