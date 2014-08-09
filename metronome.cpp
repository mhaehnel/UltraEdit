#include "metronome.h"

using namespace drumstick;

Metronome::Metronome(QObject* parent) : QObject(parent)
{
    client.open();
    client.setClientName("UltraEdit Metronome");
    port = client.createPort();
    port->setPortName("MetronomePort");
    port->setCapability(SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_WRITE);
    port->setPortType(SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    queue = client.createQueue();
    port->setTimestamping(true); //?
    port->setTimestampQueue(queue->getId());
    QueueTimer qtimer;
    qtimer.setId(Timer::bestGlobalTimerId());
    queue->setTimer(qtimer);
    client.setRealTimeInput(false);
    seq = new MidiThread(&client,port->getPortId());
}

void Metronome::connect(QString portName) {
    port->unsubscribeAll();
    if (!portName.isEmpty()) {
        for (PortInfo &p : client.getAvailableOutputs()) {
            if (QString("%1:%2").arg(p.getClientName()).arg(p.getPort()) == portName)
                port->subscribeTo(&p);
        }
    }
}

void Metronome::setSong(Song *song) {
    setTempo(song->bpm());
    int beat = 4;
    qWarning() << "BPM:" << song->bpm();
    int duration = song->sylabels().last()->beat();
    duration += song->sylabels().last()->beats();
    while (events.size() > 0) delete events.takeFirst();
    for (int i = 0; i < duration/8; i++) {
        NoteEvent* ev = new NoteEvent(9,i%beat?33:34,100,10);
        ev->scheduleTick(queue->getId(),i*ppq*2+song->gap()/60000.0*ppq*queue->getTempo().getNominalBPM(),false);
        ev->setSubscribers();
        events.append(ev);
    }
    seq->setEvents(events);
}

void Metronome::setTempo(double bpm) {
    QueueTempo& tempo = queue->getTempo();
    tempo.setPPQ(ppq);
    tempo.setNominalBPM(bpm);
    queue->setTempo(tempo);
}

void Metronome::seek(quint64 pos) {
    seq->setPosition(pos/60000.0*ppq*queue->getTempo().getNominalBPM());
}

void Metronome::play() {
    client.startSequencerInput();
    ProgramChangeEvent ev(9,0);
    ev.setSource(port->getPortId());
    ev.setSubscribers();
    ev.setDirect();
    client.outputDirect(&ev);
    seq->start();
}

void Metronome::stop() {
    client.stopSequencerInput();
    seq->allNotesOff();
    seq->stop();
    queue->clear();
}
