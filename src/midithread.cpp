#include "midithread.h"

using namespace drumstick;

MidiThread::MidiThread(MidiClient* seq, int portid) :
    SequencerOutputThread(seq,portid),
    iterator(nullptr), last_event(nullptr), _position(0),
    _echoRes(96/12) //This is based on ppq = 96!
{}

MidiThread::~MidiThread() {
    QMutexLocker lock(&iteratorLock);
    if (isRunning()) stop();
    if (iterator != nullptr) delete iterator;
    if (last_event != nullptr) delete last_event;
}

void MidiThread::setEvents(QList<SequencerEvent*>& events) {
    QMutexLocker lock(&iteratorLock);
    this->events = events;
    if (iterator != nullptr) delete iterator;
    iterator = new QMutableListIterator<SequencerEvent*>(events);
    _position = 0;
}

void MidiThread::resetPosition() {
    QMutexLocker lock(&iteratorLock);
    if (iterator != nullptr) {
        iterator->toFront();
        _position = 0;
    }
}

void MidiThread::setPosition(unsigned int pos) {
    bool wasRunning = isRunning();
    if (wasRunning) stop();
    allNotesOff();
    _position = pos;
    QMutexLocker lock(&iteratorLock);
    iterator->toFront();
    while (iterator->hasNext() && iterator->next()->getTick() < pos) {};
    if (iterator->hasPrevious()) iterator->previous();
    if (wasRunning) start();
}

bool MidiThread::hasNext() {
    QMutexLocker lock(&iteratorLock);
    return iterator->hasNext();
}

void MidiThread::sendController(int chan, int control, int value) {
    ControllerEvent ev(chan, control, value);
    ev.setSource(m_PortId);
    ev.setSubscribers();
    ev.setDirect();
    sendSongEvent(&ev);
}

void MidiThread::allNotesOff() {
    sendController(0, MIDI_CTL_ALL_NOTES_OFF, 0);
    sendController(0, MIDI_CTL_ALL_SOUNDS_OFF, 0);
}

SequencerEvent* MidiThread::nextEvent() {
    if (last_event != nullptr)
        delete last_event; //TODO: Really?
    QMutexLocker lock(&iteratorLock);
    last_event = iterator->next()->clone();
    return last_event;

//    switch (last_event->getSequencerType()) {
//         case SND_SEQ_EVENT_NOTE:
//         case SND_SEQ_EVENT_NOTEON:
//         case SND_SEQ_EVENT_NOTEOFF:
//         case SND_SEQ_EVENT_KEYPRESS: {
//             KeyEvent* kev = static_cast<KeyEvent*>(last_event);
//             if (kev->getChannel() != MIDI_GM_DRUM_CHANNEL)
//                 kev->setKey(kev->getKey() + m_pitchShift);
//         }
//         break;
//         case SND_SEQ_EVENT_CONTROLLER: {
//             ControllerEvent *cev = static_cast<ControllerEvent*>(last_event);
//             if (cev->getParam() == MIDI_CTL_MSB_MAIN_VOLUME) {
//                 int chan = cev->getChannel();
//                 int value = cev->getValue();
//                 m_volume[chan] = value; TODO!
//                 value = floor(value * m_volumeFactor / 100.0);
//                 if (value < 0) value = 0;
//                 if (value > 127) value = 127;
//                 cev->setValue(value);
//             }
//         }
//         break;
//     }

}
