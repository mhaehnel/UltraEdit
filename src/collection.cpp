#include "collection.h"
#include <song.h>

Collection::Collection(QString name, QString basePath, QString pathRule)
    : basePath_(basePath), pathRule_(pathRule), name_(name)
{
    QRegExp rex("\\$\\{([^\\}]*)\\}");
    int pos;
    while ((pos = rex.indexIn(pathRule_,pos)) != -1) {
        if (!replacementTags.contains(rex.cap(1)))
            replacementTags << rex.cap(1);
        pos += rex.matchedLength();
        qDebug() << rex.cap(1);
    }
    qDebug() << "Tags for pathRule" << pathRule_ << ":" << replacementTags;
}

QFileInfo Collection::path(const Song *s, QString extension) const {
    QString res = pathRule_;
    for (const QString& tag : replacementTags) {
        res.replace(QString("${%1}").arg(tag),s->tag(tag));
    }
    res.replace("@{EXT}",extension);
    return basePath_+'/'+res;
}
