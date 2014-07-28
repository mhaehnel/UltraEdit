#ifndef SYLABEL_H
#define SYLABEL_H

#include <QString>

class Sylabel
{
public:
    enum class Type {
        Regular, Freestyle, Golden, LineBreak, Bad
    };

    enum class Clef {
        G, F
    };

    Sylabel(QString source, int players);
    Type type() const { return _t; }
    QString text() const { return _text; }
    int beat() const { return _beat; }
    int beats() const { return _beats; } //This is 'show next line hint' for line break!
    double time(double bpm) const { return _beat/bpm/4*60.0; }
    double duration(double bpm) const { return _beats/bpm/4*60.0; }
    int key() const { return _pitch; }
    bool isBad() const { return _t == Type::Bad; }
    int forPlayers() const { return _players; }
    bool isExtension() const { return _text == "~" || _text == "~ "; }
    bool isSharp() const;
    int getLine(Clef c) const;
    bool operator==(const Sylabel& other) const;
private:
    int _beat; //time (beta = quater note no)
    int _beats; //duration in quarter notes
    Type _t;
    QString _text;
    int _pitch, _players;
};

#endif // SYLABEL_H
