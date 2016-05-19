#pragma once

#include <QObject>
#include <QLabel>
#include <QAudioDecoder>
#include <vector>

class AudioTrace : public QObject
{
Q_OBJECT

    struct SingleSample {
        float min;
        float max;
        float rms;
    };

    bool complete;

    using ChannelSample = std::vector<SingleSample>;

    int curSampleCount;
    ChannelSample curSample;

    std::vector<ChannelSample> samples;
    std::vector<float> rawFrames;

    QAudioFormat fmt;
    QAudioDecoder dec;

    const int samplesPerLine = 512;
private slots:
    void bufferAvailable();
signals:
    void finished();
    void progress(float percent);

public:
    AudioTrace(QString filename);
    void renderTrace(QLabel& lbl, quint64 pos) const;
    void renderSection(QPixmap& target, quint64 start, quint64 end) const;
};
