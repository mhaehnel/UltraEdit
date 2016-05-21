#pragma once

#include <QObject>
#include <QLabel>
#include <QAudioDecoder>
#include <vector>

class AudioTrace : public QObject
{
Q_OBJECT

    class AggregateSample {
        float min_,max_, rms_;
        double rmsCache;
        int samples_;
    public:
        AggregateSample();
        void addSample(float sample);
        float min() const { return min_; }
        float max() const { return max_; }
        float rms() const;
        int samples() const { return samples_; }
    };

    bool complete;

    std::vector<AggregateSample> samples;
    std::vector<float> rawFrames;

    QAudioFormat fmt;
    QAudioDecoder dec;
    quint64 bpm_;
    qint64 offset_;

    const int samplesPerLine = 512;
private slots:
    void bufferAvailable();
public slots:
    void updateBpm(float bpm);
    void updateGap(qint64 gap);
signals:
    void finished();
    void progress(float percent);

public:
    AudioTrace(QString filename, float bpm = 0, qint64 offset = 0);
    void renderTrace(QLabel& lbl, quint64 pos, bool withBeats = false) const;
    void renderSection(QPixmap& target, quint64 start, quint64 end) const;
};
