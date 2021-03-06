#include "midiplayer.h"

using namespace drumstick;

MidiPlayer::MidiPlayer(QObject *parent) :QObject(parent)
{
    client.open();
    client.setClientName("UltraEdit");
    port = client.createPort();
    port->setPortName("OutputPort");
    port->setCapability(SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_WRITE);
    port->setPortType(SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    queue = client.createQueue();
    seq = new MidiThread(&client,port->getPortId());
}

void MidiPlayer::play() {
    if (_song == nullptr) return;
    ProgramChangeEvent ev(0,0);
    ev.setSource(port->getPortId());
    ev.setSubscribers();
    ev.setDirect();
    client.outputDirect(&ev);
    client.startSequencerInput();
    queue->clear();
    seq->start();
}

void MidiPlayer::stop() {
    if (_song == nullptr) return;
    seq->allNotesOff();
    client.stopSequencerInput();
    seq->stop();
    queue->clear();
    events.clear();
}

void MidiPlayer::connect(QString portName) {
    //This does extra sanity checking. We might not need that.
    port->unsubscribeAll();
    if (!portName.isEmpty()) {
        for (PortInfo &p : client.getAvailableOutputs()) {
            if (QString("%1:%2").arg(p.getClientName()).arg(p.getPort()) == portName)
                port->subscribeTo(&p);
        }
    }
}

QStringList MidiPlayer::getPorts() {
    QStringList items;
    for (PortInfo &p : client.getAvailableOutputs())
        items << QString("%1:%2").arg(p.getClientName()).arg(p.getPort());
    return items;
}

void MidiPlayer::setTempo(double bpm) {
    //Setting tempo (this is fiddly :()
    QueueTempo& tempo = queue->getTempo();
    tempo.setPPQ(ppq);
    tempo.setNominalBPM(bpm);
    queue->setTempo(tempo);
}

//This is in ms ... calculate ticks ..
void MidiPlayer::seek(quint64 pos) {
    if (_song == nullptr) return;
    seq->setPosition(pos/60000.0*ppq*queue->getTempo().getNominalBPM());
}


void MidiPlayer::reschedule() {
    bool wasRunning = seq->isRunning();
    if (wasRunning) stop();
    events.clear();
    for (const Sylabel* s : _song->sylabels()) {
        if (s->isLineBreak()) continue;
        NoteEvent ev(*s->event()); //Copy event
        ev.scheduleTick(queue->getId(),s->beat()*ppq/4+s->song->gap()/60000.0*ppq*queue->getTempo().getNominalBPM(),false);
        ev.setSubscribers();
        events.append(ev);
    }
    seq->setEvents(events);
    setTempo(_song->bpm());

    if (wasRunning) play();
}


void MidiPlayer::setSong(Song *song) {
    _song = song;
    reschedule();
}

void MidiPlayer::setVolume(int vol) {
    ControllerEvent ev(0, MIDI_CTL_MSB_MAIN_VOLUME, vol);
    ev.setSource(port->getPortId());
    ev.setSubscribers();
    ev.setDirect();
    client.outputDirect(&ev);
}
