#ifndef SYLABEL_H
#define SYLABEL_H

#include <QString>
#include <QHash>

class Song;

class Sylabel
{
public:
    enum class Type {
        Regular, Freestyle, Golden, LineBreak, Bad
    };

    enum class Clef {
        G, F
    };

    enum class Note {
        C, D, E, F, G, A, B
    };

    Song * song;
    Sylabel(QString source, int players, Song * song);
    Type type() const { return _t; }
    QString text() const { return _text; }
    int beat() const { return _beat; }
    int beats() const { return _beats; } //This is 'show next line hint' for line break!
    double time() const;
    double duration() const;
    int key() const { return _pitch; }
    void setKey(int key) { _pitch = key; }
    bool isBad() const { return _t == Type::Bad; }
    int forPlayers() const { return _players; }
    bool isExtension() const { return _text == "~" || _text == "~ "; }
    bool isSharp() const;
    Sylabel::Note note() const;

    int getLine(Clef c) const;
    bool operator==(const Sylabel& other) const;
private:
    int _beat; //time (beta = quater note no)
    int _beats; //duration in quarter notes
    Type _t;
    QString _text;
    int _pitch, _players;
};

inline uint qHash(const Sylabel::Note &n) {
    return qHash(static_cast<std::underlying_type<Sylabel::Note>::type>(n));
}

#endif // SYLABEL_H
