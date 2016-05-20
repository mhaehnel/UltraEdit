#include "audiotrace.h"
#include <QPainter>
#include <cmath>

AudioTrace::AudioTrace(QString filename) : QObject(), complete(false) {
    connect(&dec,&QAudioDecoder::bufferReady,this,&AudioTrace::bufferAvailable);
    connect(&dec,&QAudioDecoder::finished,[this] {
        if (dec.bufferAvailable()) bufferAvailable();
        complete = true;
        emit finished();
    });
    connect(&dec,static_cast<void(QAudioDecoder::*)(QAudioDecoder::Error)>(&QAudioDecoder::error),[] (QAudioDecoder::Error err) {
        if (err == QAudioDecoder::ResourceError) {
            qDebug() << "No audio stream";
        } else {
            qDebug() << "An error occured while decoding!" <<  err;
        }
    });
    connect(&dec,&QAudioDecoder::positionChanged,[this]  (qint64 position) {
        emit progress(position*100/dec.duration());
    });
    dec.setSourceFilename(filename);
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleRate(44100);
    fmt.setSampleSize(32);
    fmt.setChannelCount(1); //Is this enough? I think yes for analysis
    fmt.setCodec("audio/pcm");
    fmt.setSampleType(QAudioFormat::Float);
    dec.setAudioFormat(fmt);
    dec.start();
}

AudioTrace::AggregateSample::AggregateSample()
    : min_(std::numeric_limits<float>::max()),
      max_(std::numeric_limits<float>::min()),
      rms_(INFINITY), rmsCache(0), samples_(0)
{}

void AudioTrace::AggregateSample::addSample(float sample) {
    samples_++;
    if (sample < min_) min_ = sample;
    if (sample > max_) max_ = sample;
    rmsCache += static_cast<double>(sample)*sample;
}

float AudioTrace::AggregateSample::rms() const {
    return (samples_ == 0)?NAN:sqrt(rmsCache/samples_);
}

void AudioTrace::bufferAvailable() {
    static AggregateSample curSample;

    QAudioBuffer buf = dec.read();
    if (!buf.isValid()) return;
    auto sampleBuf = reinterpret_cast<const float*>(buf.constData());
    for (int i = 0; i < buf.frameCount(); i++) {
        auto sample = sampleBuf[i];
        curSample.addSample(sample);
        rawFrames.push_back(sample);
        if (curSample.samples() == samplesPerLine) {
            samples.push_back(curSample);
            curSample = AggregateSample();
        }
    }
}

//TODO: this only works for single channel!
void AudioTrace::renderTrace(QLabel& lbl, quint64 pos) const {
        if (!complete || !fmt.isValid()) return;
        pos *= fmt.sampleRate()/1000.0;
        pos /= samplesPerLine; //Adjust

        QPixmap pix(lbl.width(),lbl.height());
        QPainter ptr(&pix);
        ptr.fillRect(0,0,pix.width(),pix.height(),Qt::black);
        double middle = pix.height()/2;
        unsigned offsetStart = (static_cast<unsigned>(pix.width()/2) > pos)?0:pos-pix.width()/2;
        unsigned offsetEnd = std::min(samples.size(),static_cast<size_t>(offsetStart+pix.width()));

        for (auto i = offsetStart; i < offsetEnd; i++) {
            if  (i >= std::max(0ull,pos-10) && i <= pos+10) {
                ptr.setPen(Qt::blue);
                ptr.drawLine(QLineF(i-offsetStart,0,i-offsetStart,pix.height()));
                ptr.setPen(Qt::yellow);
            } else {
                ptr.setPen(Qt::lightGray);
            }
            ptr.drawLine(QLineF(i-offsetStart,middle-samples[i].max()*middle,i-offsetStart,middle-samples[i].min()*middle));
            if  (i >= std::max(0ull,pos-10) && i <= pos+10) {
                ptr.setPen(Qt::darkYellow);
            } else {
                ptr.setPen(Qt::darkGreen);
            }
            ptr.drawLine(QLineF(i-offsetStart,middle-samples[i].rms()*middle,i-offsetStart,middle+samples[i].rms()*middle));
        }
        ptr.setPen(Qt::red);
        ptr.drawLine(QLineF(pos-offsetStart,0,pos-offsetStart,pix.height()));
        ptr.end();

        lbl.setPixmap(pix);
}

void AudioTrace::renderSection(QPixmap &target, quint64 start, quint64 end) const {
    target.fill(Qt::transparent);
    if (!complete || !fmt.isValid()) return;

    quint64 fromFrame = fmt.framesForDuration(start*1000);
    int sample = (fmt.framesForDuration(end*1000)-fromFrame)/target.width();
    double middle = target.height()/2;

    QPainter ptr(&target);
    for (auto i = 0; i < target.width(); i++) {
        AggregateSample as;
        for (auto j = 0; j < sample; j++)
            as.addSample(rawFrames[fromFrame+i*sample+j]);
        ptr.setPen(Qt::lightGray);
        ptr.drawLine(QLineF(i,middle-as.max()*middle,i,middle-as.min()*middle));
        ptr.setPen(Qt::darkGreen);
        ptr.drawLine(QLineF(i,middle-as.rms()*middle,i,middle+as.rms()*middle));
    }
    ptr.end();
}
