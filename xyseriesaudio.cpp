
#include "xyseriesaudio.h"
#include "qdebug.h"
#include "fftw3.h"

// #include <QXYSeries>

XYSeriesAudio::XYSeriesAudio(Ui_MainWindow *ui, QObject *parent) :
    QIODevice(parent),
    m_ui(ui)
{
}

qint64 XYSeriesAudio::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

qint64 XYSeriesAudio::writeData(const char *data, qint64 maxSize)
{
    static const int resolution = 8;

    if (m_buffer.isEmpty()) {
        m_buffer.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i)
            m_buffer.append(QPointF(i, 0));
    }

    int start = 0;
    const int availableSamples = int(maxSize) / resolution;
    // static QTime time(QTime::currentTime());
    // qDebug() << time.currentTime().toString("h:m:s:z") << " availableSamples:" << availableSamples;

    if (availableSamples < sampleCount) {
        start = sampleCount - availableSamples;
        for (int s = 0; s < start; ++s)
            m_buffer[s].setY(m_buffer.at(s + availableSamples).y());
    }

    for (int s = start; s < sampleCount; ++s, data += resolution)
        m_buffer[s].setY(qreal(uchar(*data) -128) / qreal(128));

    wave_x.resize(m_buffer.size());
    wave_y.resize(m_buffer.size());
    for (int i = 0; i < m_buffer.size(); i++) {
        wave_x[i] = m_buffer[i].x();
        wave_y[i] = m_buffer[i].y();
    }

    processFFT(wave_y, spectrum_y, specfreq_x);
    // qDebug() << "freq_last:" << spec_freq.last() << " " << *(spec_freq.end()-1);

    return (sampleCount - start) * resolution;
}

void XYSeriesAudio::processFFT(const QVector<qreal> &input, QVector<qreal> &output, QVector<qreal> &freq) {

    int N = input.size();

    for (int i = 0; i < N; ++i) {
        m_fftIn[i][0] = input[i]; // Real part
        m_fftIn[i][1] = 0; // Imaginary part
    }

    fftw_execute(m_fftP); // Execute FFT

    output.resize(N/2);
    freq.resize(N/2);
    for (int i = 0; i < N/2; ++i) {
        output[i] = sqrt(m_fftOut[i][0] * m_fftOut[i][0] + m_fftOut[i][1] * m_fftOut[i][1]); // Calculate magnitude
        freq[i] = (qreal)i * (*smpRate) / sampleCount;
    }

}
