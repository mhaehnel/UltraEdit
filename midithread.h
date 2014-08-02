#ifndef MIDITHREAD_H
#define MIDITHREAD_H

#include <drumstick/playthread.h>
#include <QMutex>

class MidiThread : public drumstick::SequencerOutputThread
{
    Q_OBJECT
public:
    MidiThread(drumstick::MidiClient* seq, int portid);
    virtual ~MidiThread();
    virtual bool hasNext();
    virtual drumstick::SequencerEvent* nextEvent();
    virtual unsigned int getInitialPosition() { return _position; }
    virtual unsigned int getEchoResolution() { return _echoRes; }
    void setEvents(QList<drumstick::SequencerEvent>& events);
    void resetPosition();
    void setPosition(unsigned int pos);
    void sendController(int chan, int control, int value);
    void allNotesOff();

private:
    QMutex iteratorLock;
    QList<drumstick::SequencerEvent> events;
    QMutableListIterator<drumstick::SequencerEvent>* iterator;
    drumstick::SequencerEvent* last_event;
    unsigned int _position;
    unsigned int _echoRes;

signals:

public slots:

};

#endif // MIDITHREAD_H
