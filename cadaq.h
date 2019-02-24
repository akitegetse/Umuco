#ifndef CADaq_H
#define CADaq_H

#include <QObject>
#include "canidaq.h"
#include "cadaqwindow.h"
#include <QFile>
#include <QTextStream>
#include <QTimer>

class CADaq : public QObject
{
    Q_OBJECT
public:
    explicit CADaq(QObject *parent = 0);
     ~CADaq();

private:
    /// properties
    QMutex lockData;
    CANiDaq * daqDevice;
    CADaqWindow * window;

    void * galvoScanTask;
    void * cameraTrigReadyTask;
    QTimer cameraTrigReadyTimer;

    QString galvoVoltChannel;
    QString galvoClockSource;
    QString cameraTrigReadyChannel;

    QString cameraTrigReadyTerm;
    QString cameraTriggerTerm;

    //
    bool running;
    int galvoScan;

    //
    QVector<double> galvoVolts; // in volts
    double galvoVoltLowerLimit; // in vols
    double galvoVoltUpperLimit; // in vols
    double galvoFreq; // in Hz
    double galvoTrigDelay;
    bool galvoCamSynchronize;
    double cameraFrameRate; // in Hz;

    /// methods
    void resetToDefault();
    void generateGalvoWfm(double lower, double upper, int slopeSign);
    static int periodMeasureCallback(void * taskHandle,
                              int eventType, void *callbackData);
    void framePeriodChanged();

signals:
    void daqChannelsChanged(QList<QString> aoChannels,
                            QList<QString> cirminals,
                            QList<QString> coChannels,
                            QList<QString> terminals);
    void synchronizationEnabled(bool ena);
    void framePeriodChangedSignal();

private slots:
        void updateFramePeriod();

public slots:
    void openWindow();
    void deviceNameChanged(QString name);
    void galvoClockSourceChanged(QString terminal);
    void galvoVoltChannelChanged(QString chan);
    void cameraTrigChannelChanged(QString chan);
    void cameraTrigReadyChannelChanged(QString chan);
    void synchronizeCameraWithGalvo(bool ena);
    void toggleCameraGalvoSynchronization();
    void galvoPositionChanged(int mVolt);
    void galvoTrigDelayChanged(int msec);
    void galvoScanTypeChanged(int val);
    void galvoLowerLimitChanged(int val);
    void galvoUpperLimitChanged(int val);

};

#endif // CADaq_H

