#ifndef COLLECTION_H
#define COLLECTION_H

#include <QString>

class Collection
{
    QString basePath_;
    QString pathRule_;
    QString name_;
public:
    explicit Collection(QString name, QString basePath, QString pathRule);
    QString basePath() const { return basePath_; }
    QString name() const { return name_; }
    QString pathRule() const { return pathRule_; }
};

#endif // COLLECTION_H
