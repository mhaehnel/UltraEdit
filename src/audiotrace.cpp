#include "audiotrace.h"
#include <QPainter>
#include <cmath>

AudioTrace::AudioTrace(QString filename) : QObject() {
    connect(&dec,&QAudioDecoder::bufferReady,this,&AudioTrace::bufferAvailable);
    connect(&dec,&QAudioDecoder::finished,[this] {
        if (dec.bufferAvailable()) bufferAvailable();
        emit finished();
        qDebug() << "Done Tracing" << samples.size() << "Lines -> " << 1.0*samples.size()*samplesPerLine/fmt.sampleRate() << "seconds";
    });
    connect(&dec,static_cast<void(QAudioDecoder::*)(QAudioDecoder::Error)>(&QAudioDecoder::error),[] {
        qDebug() << "An error occured while decoding!";
    });
    connect(&dec,&QAudioDecoder::positionChanged,[this]  (qint64 position) {
        emit progress(position*100/dec.duration());
    });
    connect(&dec,&QAudioDecoder::formatChanged,this,&AudioTrace::foundFormat);
    curSampleCount = 0;
    dec.setSourceFilename(filename);
    qDebug() << "Tracing" << filename;
    dec.start();
}

void AudioTrace::bufferAvailable() {
    while (dec.bufferAvailable()) {
        QAudioBuffer buf = dec.read();
        if (!fmt.isValid()) {
            fmt = buf.format();
            qDebug() << "Sample size: " << fmt.sampleSize();
            qDebug() << "Bytes/frame: " << fmt.bytesPerFrame();
            qDebug() << "Rate: " << fmt.sampleRate();
        } else {
            Q_ASSERT(fmt == buf.format());
        }
        Q_ASSERT(fmt.channelCount() == buf.sampleCount()/buf.frameCount());
        Q_ASSERT(fmt.sampleSize() == 32);
        const int32_t* sampleBuf = reinterpret_cast<const int32_t*>(buf.constData());
        for (int i = 0; i < buf.frameCount(); i++) {
            if (curSampleCount == 0) {
                curSample = ChannelSample(fmt.channelCount(),
                                          {std::numeric_limits<double>::max(),
                                           std::numeric_limits<double>::min(),
                                          0});
            }
            for (int c = 0; c < fmt.channelCount(); c++) {
                int32_t val = sampleBuf[i*fmt.channelCount()+c];
                double fval = (1.0*val)/((val < 0)?std::numeric_limits<int32_t>::max():std::numeric_limits<int32_t>::max());
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

}

//TODO: this only works for single channel!
void AudioTrace::renderTrace(QLabel& lbl, quint64 pos) {
        if (dec.state() != QAudioDecoder::StoppedState || !fmt.isValid()) return;
        pos *= fmt.sampleRate()/1000.0;
        pos /= samplesPerLine; //Adjust

        QPixmap pix(lbl.width(),lbl.height());
        QPainter ptr(&pix);
        ptr.fillRect(0,0,pix.width(),pix.height(),Qt::black);
        double middle = pix.height()/2;
        unsigned offsetStart = (static_cast<unsigned>(pix.width()/2) > pos)?0:pos-pix.width()/2;
        unsigned offsetEnd = std::min(samples.size(),static_cast<size_t>(offsetStart+pix.width()));
       // qDebug () << "Size:" << samples.size() << "from:" << offsetStart << "to" << offsetEnd;
        ptr.setPen(Qt::blue);
        for (auto i = std::max(0ull,pos-10); i < pos+10; i++) {
            ptr.drawLine(QLineF(i-offsetStart,0,i-offsetStart,pix.height()));
        };
        ptr.setPen(Qt::lightGray);
        for (auto i = offsetStart; i < offsetEnd; i++) {
            if (i == std::max(0ull,pos-10)) {
                ptr.setPen(Qt::yellow);
            }
            if (i == pos+10) {
                ptr.setPen(Qt::lightGray);
            }
            ptr.drawLine(QLineF(i-offsetStart,middle-samples[i][1].max*middle,i-offsetStart,middle-samples[i][1].min*middle));
        };
        ptr.setPen(Qt::darkGreen);
        for (auto i = offsetStart; i < offsetEnd; i++) {
            if (i == std::max(0ull,pos-10)) {
                ptr.setPen(Qt::darkYellow);
            }
            if (i == pos+10) {
                ptr.setPen(Qt::darkGreen);
            }
            ptr.drawLine(QLineF(i-offsetStart,middle-samples[i][1].rms*middle,i-offsetStart,middle+samples[i][1].rms*middle));
        };
        ptr.setPen(Qt::red);
        ptr.drawLine(QLineF(pos-offsetStart,0,pos-offsetStart,pix.height()));
        ptr.end();
        lbl.setPixmap(pix);
}

void AudioTrace::foundFormat(const QAudioFormat&) {
     qDebug() << "Format changed within file!"; //TODO
}
