#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMessageBox>
#include <QAudioInput>
#include <QAudioSource>
#include <QBuffer>
#include <QDebug>
#include "xyseriesaudio.h"

// QT_FORWARD_DECLARE_CLASS(QAudioDevice)
// QT_FORWARD_DECLARE_CLASS(QAudioInput)
// QT_FORWARD_DECLARE_CLASS(QAudioSource)
// QT_FORWARD_DECLARE_CLASS(QMediaDevices)

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initAudioDevices();
    void updateAudioDevices();
    void initGraphformPlot();
    void initGraphFreqPlot();

    void testFreqPlotGraph();



private slots:
    void startRecording();
    void stopRecording();
    void resetRecording();
    void processAudioInput();
    void updateSampleRate(int index);

private:
    Ui::MainWindow *ui;

    QAudioDevice *m_inputDevice = nullptr;
    QAudioInput *m_audioInput = nullptr;
    QAudioSource *m_audioSource = nullptr;
    QTimer *m_timer = nullptr;
    XYSeriesAudio *m_device = nullptr;

    QCPBars *m_bars1 = nullptr;
    // QTime m_startTime, m_stopTime;
    // QTime recordTime;
    // QCPBars *m_bars2 = nullptr;

    qint64 elapsedTime = 0, recordElapsed = 0;
    qint32 sampleRate = 8000;
    QElapsedTimer elapsedTimer;

    bool isRecording = false;

};
#endif // MAINWINDOW_H
