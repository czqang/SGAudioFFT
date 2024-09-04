#include "mainwindow.h"
#include "./ui_mainwindow.h"


QCustomPlot *widget;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_timer(new QTimer(this))
{
    ui->setupUi(this);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    this->setPalette(palette);

    initGraphformPlot();
    initGraphFreqPlot();
    updateAudioDevices();
    initAudioDevices();

    connect(m_timer, &QTimer::timeout, this, &MainWindow::processAudioInput);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startRecording);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopRecording);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::resetRecording);
    connect(ui->sampleRateComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateSampleRate);

    // testFreqPlotGraph();

}

MainWindow::~MainWindow()
{
    delete ui;
    fftw_destroy_plan(m_device->m_fftP);
    fftw_free(m_device->m_fftIn);
    fftw_free(m_device->m_fftOut);
}

void MainWindow::initAudioDevices()
{
    m_device = new XYSeriesAudio(ui, this);
    m_device->smpRate = &sampleRate;
    m_device->m_fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * XYSeriesAudio::sampleCount);
    m_device->m_fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * XYSeriesAudio::sampleCount);
    m_device->m_fftP = fftw_plan_dft_1d(XYSeriesAudio::sampleCount, m_device->m_fftIn, m_device->m_fftOut, FFTW_FORWARD, FFTW_ESTIMATE);

    QAudioFormat formatAudio;
    // formatAudio.setSampleRate(m_device->sampleRate);
    formatAudio.setSampleRate(sampleRate);
    formatAudio.setChannelCount(1);
    formatAudio.setSampleFormat(QAudioFormat::UInt8);

    const QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    m_audioInput = new QAudioInput(inputDevice, this);
    m_audioSource = new QAudioSource(inputDevice, formatAudio);
    m_audioSource->setBufferSize(200);

}

void MainWindow::initGraphformPlot() {

    ui->waveformPlot->clearGraphs();
    ui->waveformPlot->addGraph();
    // ui->waveformPlot->xAxis->setLabel("Sample Number");
    // ui->waveformPlot->yAxis->setLabel("Amplitude");
    ui->waveformPlot->axisRect()->setupFullAxesBox();
    ui->waveformPlot->xAxis->setRange(0, XYSeriesAudio::sampleCount);
    ui->waveformPlot->yAxis->setRange(-1.2, 1.2);
    QPen graphPen;
    graphPen.setColor(QColor(176, 31, 36));
    ui->waveformPlot->graph()->setPen(graphPen);

    ui->spectrumPlot->clearGraphs();
    ui->spectrumPlot->addGraph();
    ui->spectrumPlot->axisRect()->setupFullAxesBox();
    ui->spectrumPlot->xAxis->setRange(0, sampleRate/2);
    ui->spectrumPlot->yAxis->setRange(-0.1, 20);

    // QPixmap originalPixmap = QPixmap("../../Resource/SugonNetLogoB.jpeg");
    QPixmap originalPix1 = QPixmap(":/logo/xiongmao.svg");
    QPixmap originalPix2 = QPixmap(":/logo/haitun.svg");
    QPixmap originalPix3 = QPixmap(":/logo/wugui.svg");
    // ui->logoLabel->setPixmap(QPixmap("../../SugonNetLogo.jpeg"),ui->logoLabel->size(), Qt::KeepAspectRatio);
    ui->label1->setPixmap(originalPix1.scaled(ui->label1->size(), Qt::KeepAspectRatioByExpanding));
    ui->label2->setPixmap(originalPix2.scaled(ui->label2->size(), Qt::KeepAspectRatioByExpanding));
    ui->label3->setPixmap(originalPix3.scaled(ui->label3->size(), Qt::KeepAspectRatioByExpanding));

    QString initTime = "00:00:00";
    ui->timerLcdNum->setStyleSheet("QLCDNumber { color: black; background: transparent}");
    ui->timerLcdNum_2->setStyleSheet("QLCDNumber { color: black; background: transparent}");
    ui->timerLcdNum->display(initTime);
    ui->timerLcdNum_2->display(".000");

}

void MainWindow::initGraphFreqPlot()
{
    m_bars1 = new QCPBars(ui->frequencyPlot->xAxis, ui->frequencyPlot->yAxis);

    ui->frequencyPlot->clearGraphs();
    ui->frequencyPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    ui->frequencyPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    ui->frequencyPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->xAxis->setTickLabelColor(Qt::white);
    ui->frequencyPlot->yAxis->setTickLabelColor(Qt::white);
    ui->frequencyPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->frequencyPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->frequencyPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    ui->frequencyPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    // ui->frequencyPlot->xAxis->grid()->setSubGridVisible(true);
    ui->frequencyPlot->yAxis->grid()->setSubGridVisible(true);
    ui->frequencyPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    ui->frequencyPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);

    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    ui->frequencyPlot->setBackground(plotGradient);
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    ui->frequencyPlot->axisRect()->setBackground(axisRectGradient);

    ui->frequencyPlot->rescaleAxes();
    ui->frequencyPlot->xAxis->setRange(0, 4);
    ui->frequencyPlot->yAxis->setRange(0, 15);

}

void MainWindow::startRecording() {

    if (isRecording) return;
    m_timer->start(10);
    elapsedTimer.start();
    m_device->open(QIODevice::WriteOnly);
    m_audioSource->start(m_device);
    isRecording = true;
    qDebug() << "startButton has clicked. Recording? " << isRecording << "\n";

}

void MainWindow::stopRecording() {

    if (!isRecording) return;
    m_timer->stop();
    recordElapsed += elapsedTimer.elapsed();
    elapsedTimer.invalidate();
    m_audioSource->stop();
    m_device->close();
    isRecording = false;
    ui->timerLcdNum->setStyleSheet("QLCDNumber { color: blue; background: transparent}");
    ui->timerLcdNum_2->setStyleSheet("QLCDNumber { color: blue; background: transparent}");
    qDebug() << "stopButton has clicked. Recording? " << isRecording << "\n";

}

void MainWindow::resetRecording()
{
    m_timer->stop();
    m_device->close();
    m_device->reset();
    m_audioSource->stop();
    isRecording = false;
    elapsedTime = 0;
    elapsedTimer.invalidate();
    recordElapsed = 0;
    QString initTime = "00:00:00";
    ui->timerLcdNum->display(initTime);
    ui->timerLcdNum->setStyleSheet("QLCDNumber { color: black; background: transparent}");
    ui->timerLcdNum_2->display(".000");
    ui->timerLcdNum_2->setStyleSheet("QLCDNumber { color: black; background: transparent}");
    ui->amplProgressBar->setValue(0);

    m_device->m_buffer.clear();
    m_device->wave_x.clear();
    m_device->wave_y.clear();

    ui->waveformPlot->graph()->setData(m_device->wave_x, m_device->wave_y);
    ui->waveformPlot->replot();

    m_device->specfreq_x.clear();
    m_device->spectrum_y.clear();
    ui->spectrumPlot->graph()->setData(m_device->specfreq_x, m_device->spectrum_y);
    ui->spectrumPlot->replot();

    QVector<qreal> x, y;
    m_bars1->setData(x, y);
    ui->frequencyPlot->replot();

    qDebug() << "stopButton has clicked. Recording? " << isRecording << "\n";
}

void MainWindow::processAudioInput() {

    elapsedTime = elapsedTimer.elapsed() + recordElapsed;
    int milliseconds = elapsedTime % 1000;
    int seconds = (elapsedTime / 1000) % 60;
    int minutes = (elapsedTime / (1000 * 60)) % 60;
    int hour = (elapsedTime / (1000 * 60 * 60)) % 24;
    QString timeString = QString("%1:%2:%3")
                             .arg(hour, 2, 10, QChar('0'))
                             .arg(minutes, 2, 10, QChar('0'))
                             .arg(seconds, 2, 10, QChar('0'));
    // ui->timerLcdNum->setStyleSheet("QLCDNumber { color: red; }");
    ui->timerLcdNum->setStyleSheet("color:rgb(176, 31, 36); background: transparent");
    ui->timerLcdNum->display(timeString);
    QString msString = QString(".%1").arg(milliseconds, 3, 10, QChar('0'));
    ui->timerLcdNum_2->display(msString);
    ui->timerLcdNum_2->setStyleSheet("color:rgb(176, 31, 36); background: transparent");

    qreal average = 0.0;
    for (int i = 0; i < m_device->wave_y.size(); i++)
        average += qAbs(m_device->wave_y[i]);
    average /= m_device->wave_y.size();
    ui->amplProgressBar->setValue(average*500);

    ui->waveformPlot->graph()->setData(m_device->wave_x, m_device->wave_y);
    ui->waveformPlot->replot();

    ui->spectrumPlot->graph()->setData(m_device->specfreq_x, m_device->spectrum_y);
    ui->spectrumPlot->replot();

    QVector<qreal> freqHist_x(20), freqHist_y(20);
    int N = m_device->spectrum_y.size()/freqHist_x.size();
    // qDebug() << "N:" << N << "y.size:" << m_device->spectrum_y.size() << "x.size:" << freqHist_x.size();
    for (int i=0; i<freqHist_x.size(); ++i)
    {
        freqHist_x[i] = 0.2*i+0.1;

        qreal sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += m_device->spectrum_y[i*50 + j];
        }
        freqHist_y[i] = sum / N;
    }

    // QCPBars *m_bars1 = new QCPBars(ui->frequencyPlot->xAxis, ui->frequencyPlot->yAxis);
    m_bars1->setWidth(3/(double)freqHist_x.size());
    m_bars1->setData(freqHist_x, freqHist_y);
    m_bars1->setPen(Qt::NoPen);
    m_bars1->setBrush(QColor(10, 140, 70, 160));

    // QCPBars *bars2 = new QCPBars(ui->frequencyPlot->xAxis, ui->frequencyPlot->yAxis);
    // bars2->setWidth(3/(double)freqHist_x.size());
    // bars2->setData(freqHist_x, freqHist_x);
    // bars2->setPen(Qt::NoPen);
    // bars2->setBrush(QColor(10, 100, 50, 70));
    // bars2->moveAbove(bars1);

    ui->frequencyPlot->replot();

}

void MainWindow::updateAudioDevices() {  
    const QList<QAudioDevice>& devices = QMediaDevices::audioInputs();
    if (devices.isEmpty()) {
        QMessageBox::warning(nullptr, "audio",
                             "There is no audio input device available.");
    }
    else {
        for (const QAudioDevice &device : devices) {  
            ui->deviceComboBox->addItem(device.description(), QVariant::fromValue(device));  
        }
        // currentDevice = devices.first();  
        ui->deviceComboBox->setCurrentIndex(0); 
    }

    ui->sampleRateComboBox->addItem("8000 Hz", 8000);
    ui->sampleRateComboBox->addItem("32768 Hz", 32768);
    ui->sampleRateComboBox->addItem("44100 Hz", 44100);
    ui->sampleRateComboBox->addItem("48000 Hz", 48000);
    ui->sampleRateComboBox->setCurrentIndex(0);

}

void MainWindow::updateSampleRate(int index) {

    if (isRecording)
    {
        switch (sampleRate) {
        case 8000:
            ui->sampleRateComboBox->setCurrentIndex(0);
            break;
        case 32768:
            ui->sampleRateComboBox->setCurrentIndex(1);
            break;
        case 44100:
            ui->sampleRateComboBox->setCurrentIndex(2);
            break;
        case 48000:
            ui->sampleRateComboBox->setCurrentIndex(3);
            break;
        default:
            ui->sampleRateComboBox->setCurrentIndex(0);
            break;
        }
        return;
    }
    resetRecording();
    sampleRate = ui->sampleRateComboBox->itemData(index).toInt();
    // ui->sampleRateComboBox->setCurrentIndex(0);
    initGraphformPlot();
    initAudioDevices();
}

void MainWindow::testFreqPlotGraph() {

    QVector<double> x1(20), y1(20);
    QVector<double> x2(100), y2(100);
    QVector<double> x3(20), y3(20);
    QVector<double> x4(20), y4(20);
    for (int i=0; i<x1.size(); ++i)
    {
        x1[i] = i/(double)(x1.size()-1)*10;
        y1[i] = qCos(x1[i]*0.8+qSin(x1[i]*0.16+1.0))*qSin(x1[i]*0.54)+1.4;
    }
    for (int i=0; i<x2.size(); ++i)
    {
        x2[i] = i/(double)(x2.size()-1)*10;
        y2[i] = qCos(x2[i]*0.85+qSin(x2[i]*0.165+1.1))*qSin(x2[i]*0.50)+1.7;
    }
    for (int i=0; i<x3.size(); ++i)
    {
        x3[i] = i/(double)(x3.size()-1)*10;
        y3[i] = 0.05+3*(0.5+qCos(x3[i]*x3[i]*0.2+2)*0.5)/(double)(x3[i]+0.7)+rand()/(double)RAND_MAX*0.01;
        x3[i] = 0.2*i+0.08;
    }
    for (int i=0; i<x4.size(); ++i)
    {
        x4[i] = x3[i];
        y4[i] = (0.5-y4[i])+((x4[i]-2)*(x4[i]-2)*0.02);
    }

    // create and configure plottables:
    // QCPGraph *graph1 = ui->frequencyPlot->addGraph();
    // graph1->setData(x1, y1);
    // graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(Qt::white), 9));
    // graph1->setPen(QPen(QColor(120, 120, 120), 2));

    // QCPGraph *graph2 = ui->frequencyPlot->addGraph();
    // graph2->setData(x2, y2);
    // graph2->setPen(Qt::NoPen);
    // graph2->setBrush(QColor(200, 200, 200, 20));
    // graph2->setChannelFillGraph(graph1);

    QCPBars *bars1 = new QCPBars(ui->frequencyPlot->xAxis, ui->frequencyPlot->yAxis);
    bars1->setWidth(3/(double)x3.size());
    bars1->setData(x3, y3);
    bars1->setPen(Qt::NoPen);
    bars1->setBrush(QColor(10, 140, 70, 160));

    QCPBars *bars2 = new QCPBars(ui->frequencyPlot->xAxis, ui->frequencyPlot->yAxis);
    bars2->setWidth(3/(double)x4.size());
    bars2->setData(x4, y4);
    bars2->setPen(Qt::NoPen);
    bars2->setBrush(QColor(10, 100, 50, 70));
    bars2->moveAbove(bars1);

    // move bars above graphs and grid below bars:
    // ui->frequencyPlot->addLayer("abovemain", ui->frequencyPlot->layer("main"), QCustomPlot::limAbove);
    // ui->frequencyPlot->addLayer("belowmain", ui->frequencyPlot->layer("main"), QCustomPlot::limBelow);
    // graph1->setLayer("abovemain");
    // ui->frequencyPlot->xAxis->grid()->setLayer("belowmain");
    // ui->frequencyPlot->yAxis->grid()->setLayer("belowmain");

    // set some pens, brushes and backgrounds:
    ui->frequencyPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    ui->frequencyPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    ui->frequencyPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    ui->frequencyPlot->xAxis->setTickLabelColor(Qt::white);
    ui->frequencyPlot->yAxis->setTickLabelColor(Qt::white);
    ui->frequencyPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->frequencyPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    ui->frequencyPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    ui->frequencyPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    // ui->frequencyPlot->xAxis->grid()->setSubGridVisible(true);
    ui->frequencyPlot->yAxis->grid()->setSubGridVisible(true);
    ui->frequencyPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    ui->frequencyPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    // ui->frequencyPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    // ui->frequencyPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    ui->frequencyPlot->setBackground(plotGradient);
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    ui->frequencyPlot->axisRect()->setBackground(axisRectGradient);

    ui->frequencyPlot->rescaleAxes();
    ui->frequencyPlot->xAxis->setRange(0, 4);
    ui->frequencyPlot->yAxis->setRange(0, 2);
}


