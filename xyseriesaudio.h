#ifndef XYSERIESAUDIO_H
#define XYSERIESAUDIO_H

#include <QIODevice>
#include <QList>
#include <QPointF>
#include "./ui_mainwindow.h"
#include "fftw3.h"


class XYSeriesAudio : public QIODevice {
    Q_OBJECT
public:
    // explicit XYSeriesAudio(Ui_MainWindow *ui, fftw_complex *in, fftw_complex *out, fftw_plan *p, QObject *parent = nullptr);
    explicit XYSeriesAudio(Ui_MainWindow *ui, QObject *parent = nullptr);

    void processFFT(const QVector<qreal> &input, QVector<qreal> &output, QVector<qreal> &freq);

    static const int sampleCount = 2000;
    static const int sampleRate = 8000;
    int sampleRate1 = 8000;
    int *smpRate = nullptr;

    protected:
        qint64 readData(char *data, qint64 maxSize) override;
        qint64 writeData(const char *data, qint64 maxSize) override;

    private:
        Ui_MainWindow *m_ui = nullptr;

    public:
        fftw_complex *m_fftIn = nullptr;
        fftw_complex *m_fftOut  = nullptr;
        fftw_plan m_fftP = nullptr;

        QList<QPointF> m_buffer;

        QVector<qreal> wave_x, wave_y;
        QVector<qreal> spectrum_y, specfreq_x;

};

#endif // XYSERIESAUDIO_H
