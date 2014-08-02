#ifndef MIDIPLAYER_H
#define MIDIPLAYER_H

#include <QObject>
#include "song.h"
#include <drumstick.h>
#include <midithread.h>

namespace drumstick {
    class QSmf;
    class QWrk;
    class QOve;
    class MidiClient;
    class MidiPort;
    class MidiQueue;
    class SequencerEvent;
}

using namespace drumstick;

class MidiPlayer : public QObject
{
    Q_OBJECT
    MidiClient client;
    MidiPort* port;
    MidiQueue* queue;
    Song* _song;
    MidiThread* seq;
    int ppq = 96;
    QList<SequencerEvent> events;
public:
    explicit MidiPlayer(QObject *parent = 0);
    QStringList getPorts();
signals:
    void positionChanged(quint64 pos);

public slots:
    void connect(QString portName);
    void setTempo(double bpm);
    void setSong(Song* song);
    void seek(quint64 pos);
    void play();
    void stop();

};

#endif // MIDIPLAYER_H
