#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QSettings>
#include <QString>
#include "song.h"
#include "validatorsettings.h"

class Validator
{
    enum class Type {
        MP3, TXT, COV, BG, VID, ALL
    };

public:
    enum class Mode {
        ReadOnly, Fix
    };

    Validator(QSettings& settings,Mode mode, QString basePath);
    bool validate(Song* song, Type t = Type::ALL);\
    QStringList possiblePaths(Song* song, Type t);
    bool good(); //False if failed to compile validator string!
private:
    bool _good;
    QString pattern;
    QString songPattern(Type t, Song* song);
    //return true if t-path of song matches path
    bool validatePath(Type t, Song* song, QString path);
    QString reducePath(QString path);
    Mode fixMode;
    QString basePath;
    QString txtPattern, mp3Pattern, vidPattern, covPattern, bgPattern;
    QList<QString> upper, exact, lower, title, start;

    friend class ValidatorSettings;
};

#endif // VALIDATOR_H
