#pragma once
#include <QObject>
#include "song.h"
#include <drumstick.h>
#include <midithread.h>

class MidiPlayer : public QObject
{
    Q_OBJECT
    drumstick::MidiClient client;
    drumstick::MidiPort* port;
    drumstick::MidiQueue* queue;
    Song* _song;
    MidiThread* seq;
    int ppq = 96;
    QList<drumstick::SequencerEvent> events;
public:
    explicit MidiPlayer(QObject *parent = 0);
    QStringList getPorts();

public slots:
    void connect(QString portName);
    void setTempo(double bpm);
    void setSong(Song* song);
    void seek(quint64 pos);
    void play();
    void reschedule();
    void stop();
    void setVolume(int vol);
};
