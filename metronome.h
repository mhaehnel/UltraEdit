#ifndef METRONOME_H
#define METRONOME_H

#include <QObject>
#include "song.h"
#include <drumstick.h>
#include "midithread.h"

class Metronome : public QObject
{
    Q_OBJECT
    drumstick::MidiClient client;
    drumstick::MidiPort* port;
    drumstick::MidiQueue* queue;
    MidiThread* seq;
    QList<drumstick::SequencerEvent*> events;
    static const int ppq = 96;
public:
    explicit Metronome(QObject* parent = nullptr);

public slots:
    void setSong(Song* song);
    void connect(QString portName);
    void setTempo(double bpm);
    void seek(quint64 pos);
    void play();
    void stop();
};

#endif // METRONOME_H
