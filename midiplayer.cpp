#include "midiplayer.h"

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
    setTempo(120);
    QObject::connect(&client,&MidiClient::eventReceived,[this] (SequencerEvent* ev) {
        snd_seq_tick_time_t time = queue->getStatus().getTickTime();
        emit positionChanged((time*60000.0)/(ppq*queue->getTempo().getNominalBPM()));
    });
}

void MidiPlayer::play() {
    client.startSequencerInput();
    seq->start();
}

void MidiPlayer::stop() {
    client.stopSequencerInput();
    seq->allNotesOff();
    seq->stop();
    //seq->allNotesOff();
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
    //tempo.setTempo(500000); //1bps
    tempo.setPPQ(96);
    tempo.setNominalBPM(bpm);
    qWarning() << "Tempo = " << bpm;
    queue->setTempo(tempo);
}

//This is in ms ... calculate ticks ..
void MidiPlayer::seek(quint64 pos) {
    //qWarning() << "Seeking to " << pos << "ms => " <<pos/60000.0*ppq*queue->getTempo().getNominalBPM() ;
    seq->setPosition(pos/60000.0*ppq*queue->getTempo().getNominalBPM());
}

void MidiPlayer::setSong(Song *song) {
    _song = song;

    setTempo(_song->bpm());
    queue->start();
    seq->stop();
    events.clear();
    for (const Sylabel* s : song->sylabels()) {
        if (s->type() == Sylabel::Type::LineBreak) continue;
        //NoteEvent ev(0,60+s->key(),100,s->beats()*ppq/4);
        qWarning() << "Getting ...";
        NoteEvent* ev = s->getEvent();
        qWarning() << "Creating event" << s->key() << "[" << s->beats()*ppq/4 << "] @" << s->beat()*ppq/4+s->song->gap()/60000.0*ppq*queue->getTempo().getNominalBPM();
        ev->scheduleTick(queue->getId(),s->beat()*ppq/4+s->song->gap()/60000.0*ppq*queue->getTempo().getNominalBPM(),false);
        qWarning() << "Subscribe";
        ev->setSubscribers();
        qWarning() << "Append";
        events.append(ev);
    }
    qWarning() << "DONE! Setting ..";
    seq->setEvents(events);
}
