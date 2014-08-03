#ifndef SYLABEL_H
#define SYLABEL_H

#include <QObject>
#include <QString>
#include <QHash>
#include <drumstick.h>

class Song;

class Sylabel : public QObject
{
    Q_OBJECT
public:
    enum class Type {
        Regular, Freestyle, Golden, SimpleLineBreak, LineBreak, Bad
    };

    enum class Clef {
        G, F
    };

    enum class Note {
        C, D, E, F, G, A, B
    };

    Sylabel(QString source, int players, Song * song);
    Sylabel(const Sylabel&) = delete;

    virtual ~Sylabel();

    Sylabel& operator=(const Sylabel&) = delete;
    bool operator==(const Sylabel& other) const;

    inline int beat() const { return _beat; }
    drumstick::NoteEvent* event() const { return _event; }
    inline int forPlayers() const { return _players; } //This is unimplemented atm
    inline bool isBad() const { return _t == Type::Bad; }
    inline bool isExtension() const { return _text.trimmed() == "~"; }
    inline QString text() const { return _text; }
    inline Type type() const { return _t; }

    int beats() const;
    double duration() const;
    bool isLineBreak() const { return _t == Type::SimpleLineBreak || _t == Type::LineBreak; }
    bool isSharp() const;
    unsigned char key() const;
    int line(Clef c) const; //note line
    double time() const;

    void setText(QString text);
    void transpose(char lvl);
    Sylabel::Note note() const;


    Song * song;
signals:
    void updated();

private:
    static int ppq;
    drumstick::NoteEvent* _event;
    int _beat; //time (beta = quater note no)
    Type _t;
    QString _text;
    int _players; //TODO
};

inline uint qHash(const Sylabel::Note &n) {
    return qHash(static_cast<std::underlying_type<Sylabel::Note>::type>(n));
}

#endif // SYLABEL_H
