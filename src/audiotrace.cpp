#include "audiotrace.h"
#include <QPainter>
#include <cmath>

AudioTrace::AudioTrace(QString filename) : QObject(), complete(false) {
    connect(&dec,&QAudioDecoder::bufferReady,this,&AudioTrace::bufferAvailable);
    connect(&dec,&QAudioDecoder::finished,[this] {
        if (dec.bufferAvailable()) bufferAvailable();
        complete = true;
        emit finished();
        qDebug() << "Done Tracing" << samples.size() << "Lines -> " << 1.0*samples.size()*samplesPerLine/fmt.sampleRate() << "seconds";
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
    curSampleCount = 0;
    dec.setSourceFilename(filename);
    qDebug() << "Tracing" << filename;
    dec.start();
}

void AudioTrace::bufferAvailable() {
    QAudioBuffer buf = dec.read();
    if (!buf.isValid()) return;
    if (!fmt.isValid()) {
        fmt = buf.format();
        qDebug() << "Sample size: " << fmt.sampleSize();
        qDebug() << "Bytes/frame: " << fmt.bytesPerFrame();
        qDebug() << "Rate: " << fmt.sampleRate();
        qDebug() << "Format:" << fmt.sampleType();
    } else {
        Q_ASSERT(fmt == buf.format());
    }
    Q_ASSERT(fmt.channelCount() == buf.sampleCount()/buf.frameCount());
    Q_ASSERT(fmt.sampleSize() == 32);
    auto sampleBuf = buf.constData();
    for (int i = 0; i < buf.frameCount(); i++) {
        if (curSampleCount == 0) {
            curSample = ChannelSample(fmt.channelCount(),
                                      {std::numeric_limits<float>::max(),
                                       std::numeric_limits<float>::min(),
                                      0});
        }
        for (int c = 0; c < fmt.channelCount(); c++) {
            float fval = 0;
            switch (fmt.sampleType()) {
                case QAudioFormat::Float:
                    fval = reinterpret_cast<const float*>(sampleBuf)[i*fmt.channelCount()+c];
                    break;
                case QAudioFormat::SignedInt:
                {
                    int32_t val = reinterpret_cast<const int32_t*>(sampleBuf)[i*fmt.channelCount()+c];
                    fval = (1.0*val)/((val < 0)?std::numeric_limits<int32_t>::max():std::numeric_limits<int32_t>::max());
                    break;
                }
                case QAudioFormat::UnSignedInt:
                {
                    uint32_t val = reinterpret_cast<const uint32_t*>(sampleBuf)[i*fmt.channelCount()+c];
                    fval = 2.0*(val/std::numeric_limits<uint32_t>::max())+1.0;
                    break;
                }
                case QAudioFormat::Unknown:
                    qDebug() << "Unknown Format! Can't decode.";
            }

            if (fval < curSample[c].min) curSample[c].min = fval;
            if (fval > curSample[c].max) curSample[c].max = fval;
            curSample[c].rms += fval*fval;
        }
        curSampleCount++;
        if (curSampleCount == samplesPerLine) {
            for (SingleSample& s : curSample) {
                s.rms = sqrt(s.rms/samplesPerLine);
            }

            samples.push_back(curSample);
            curSampleCount = 0;
        }
    }
}

//TODO: this only works for single channel!
void AudioTrace::renderTrace(QLabel& lbl, quint64 pos) {
        if (!complete || !fmt.isValid()) return;
        pos *= fmt.sampleRate()/1000.0;
        pos /= samplesPerLine; //Adjust

        QPixmap pix(lbl.width(),lbl.height());
        QPainter ptr(&pix);
        ptr.fillRect(0,0,pix.width(),pix.height(),Qt::black);
        double middle = pix.height()/2;
        unsigned offsetStart = (static_cast<unsigned>(pix.width()/2) > pos)?0:pos-pix.width()/2;
        unsigned offsetEnd = std::min(samples.size(),static_cast<size_t>(offsetStart+pix.width()));
        ptr.setPen(Qt::blue);
        for (auto i = std::max(0ull,pos-10); i < pos+10; i++) {
            ptr.drawLine(QLineF(i-offsetStart,0,i-offsetStart,pix.height()));
        };
        ptr.setPen(Qt::lightGray);
        for (auto i = offsetStart; i < offsetEnd; i++) {
            if (i == std::max(0ull,pos-10)) ptr.setPen(Qt::yellow);
            if (i == pos+10) ptr.setPen(Qt::lightGray);
            ptr.drawLine(QLineF(i-offsetStart,middle-samples[i][1].max*middle,i-offsetStart,middle-samples[i][1].min*middle));
        };
        ptr.setPen(Qt::darkGreen);
        for (auto i = offsetStart; i < offsetEnd; i++) {
            if (i == std::max(0ull,pos-10)) ptr.setPen(Qt::darkYellow);
            if (i == pos+10) ptr.setPen(Qt::darkGreen);
            ptr.drawLine(QLineF(i-offsetStart,middle-samples[i][1].rms*middle,i-offsetStart,middle+samples[i][1].rms*middle));
        };
        ptr.setPen(Qt::red);
        ptr.drawLine(QLineF(pos-offsetStart,0,pos-offsetStart,pix.height()));
        ptr.end();
        lbl.setPixmap(pix);
}
