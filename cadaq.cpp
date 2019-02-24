#include "cadaq.h"

inline int waveformSign(int scantype)
{
    int sign;

    switch(scantype){
    case 0:
        sign = 1;
        break;
    case 1:
        sign = -1;
        break;
    case 3:
        sign = 2;
        break;
    case 4:
        sign = -2;
        break;
    default:
        sign = 1;
        break;
    }
    return sign;
}

#define GALVO_FREQ_DEFAULT (130.0)
///
/// \brief CADaq::CADaq
/// \param parent
///
CADaq::CADaq(QObject *parent) :
    QObject(parent),
    daqDevice(new CANiDaq),
    window(new CADaqWindow),
    galvoScanTask(NULL),
    cameraTrigReadyTask(NULL),
    cameraTrigReadyTerm(32,0),
    cameraTriggerTerm(32,0),
    galvoClockSource(QString("")),
    running (false),
    galvoScan(0),
    galvoVoltLowerLimit(-1.0),
    galvoVoltUpperLimit(1.0),
    galvoFreq(GALVO_FREQ_DEFAULT),
    galvoTrigDelay(0.0),
    galvoCamSynchronize(false),
    cameraFrameRate(100.0)
{
    ///
    /// Connect to Graphic interface
    ///
    connect(this, SIGNAL(daqChannelsChanged(QList<QString>,QList<QString>,
                                            QList<QString>,QList<QString>)),
            window, SLOT(daqChannelsChanged(QList<QString>,QList<QString>,
                                            QList<QString>, QList<QString>)));
    connect(window, SIGNAL(galvoClockSourceChanged(QString)),
                this, SLOT(galvoClockSourceChanged(QString)));
    connect(window, SIGNAL(galvoVoltChannelChanged(QString)),
            this, SLOT(galvoVoltChannelChanged(QString)));
    connect(window, SIGNAL(galvoPositionChanged(int)),
            this, SLOT(galvoPositionChanged(int)));
    connect(window, SIGNAL(galvoTrigDelayChanged(int)),
            this, SLOT(galvoTrigDelayChanged(int)));

    connect(window, SIGNAL(cameraTrigChannelChanged(QString)),
            this, SLOT(cameraTrigChannelChanged(QString)));

    connect(window, SIGNAL(cameraTrigReadyChannelChanged(QString)),
            this, SLOT(cameraTrigReadyChannelChanged(QString)));

    connect(window, SIGNAL(galvoLowerLimitChanged(int)),
            this, SLOT(galvoLowerLimitChanged(int)));
    connect(window, SIGNAL(galvoUpperLimitChanged(int)),
            this, SLOT(galvoUpperLimitChanged(int)));

    connect(window, SIGNAL(galvoScanType(int)),
            this, SLOT(galvoScanTypeChanged(int)));

    connect(this, SIGNAL(synchronizationEnabled(bool)),
            window, SLOT(disableGalvoScanType(bool)));

    connect(this, SIGNAL(framePeriodChangedSignal()),
            this, SLOT(updateFramePeriod()));

    generateGalvoWfm(galvoVoltLowerLimit,
                     galvoVoltUpperLimit,
                     1);
}

///
/// \brief CADaq::~CADaq
///
CADaq::~CADaq()
{
    if (galvoScanTask != NULL){
        double wdata[2] = {0, 0};
        daqDevice->stopTask(galvoScanTask);
        daqDevice->analogOutputWriteDoubleVector(galvoScanTask,
                                                 wdata,2);
        daqDevice->startTask(galvoScanTask);
    }

    if(galvoCamSynchronize)
        daqDevice->disconnectTerminals(cameraTrigReadyTerm.toStdString().c_str(),
                                       cameraTriggerTerm.toStdString().c_str());

    if (cameraTrigReadyTask != NULL)
        daqDevice->clearTask(cameraTrigReadyTask);
    if(galvoScanTask)
        daqDevice->clearTask(galvoScanTask);

    delete daqDevice;
    delete window;
}

///
/// \brief CADaq::openWindow
///
void CADaq::openWindow()
{
    window->show();
    window->raise();
}

///
/// \brief CADaq::resetToDefault
///
void CADaq::resetToDefault()
{
    galvoFreq = GALVO_FREQ_DEFAULT;
    galvoVoltChannelChanged(galvoVoltChannel);
    cameraTrigReadyChannelChanged(cameraTrigReadyChannel);
}

///
/// \brief CADaq::synchronizeCameraWithGalvo
/// \param ena
///
void CADaq::synchronizeCameraWithGalvo(bool ena)
{
    if (ena) {
        if(galvoScanTask != NULL){
            double rate = galvoVolts.count() * galvoFreq;
            // stop tasks
            daqDevice->stopTask(galvoScanTask);
            daqDevice->stopTask(cameraTrigReadyTask);
            // Galvo scan
            daqDevice->trigWithTerminal(galvoScanTask,
                                       cameraTrigReadyTerm.toStdString().c_str(),
                                       galvoTrigDelay, true);
            daqDevice->configureSampClkTiming(galvoScanTask,
                                              galvoClockSource.toStdString().c_str(),
                                              galvoVolts.count(),
                                              rate, 1);
            daqDevice->analogOutputWriteDoubleVector(galvoScanTask, galvoVolts.data(),
                                                     galvoVolts.count());
            // Camera trigger
            daqDevice->connectTerminals(cameraTrigReadyTerm.toStdString().c_str(),
                                        cameraTriggerTerm.toStdString().c_str(), false);
            // Start tasks
            daqDevice->startTask(galvoScanTask);
            daqDevice->startTask(cameraTrigReadyTask);
        } else ena = false;
    } else {
        daqDevice->stopTask(galvoScanTask);
        daqDevice->disconnectTerminals(cameraTrigReadyTerm.toStdString().c_str(),
                                    cameraTriggerTerm.toStdString().c_str());
        resetToDefault();
    }
    galvoCamSynchronize = ena;
    emit(synchronizationEnabled(ena));
}

///
/// \brief CADaq::toggleCameraGalvoSynchronization
///
void CADaq::toggleCameraGalvoSynchronization()
{
    daqDevice->disconnectTerminals(cameraTrigReadyTerm.toStdString().c_str(),
                                cameraTriggerTerm.toStdString().c_str());
    daqDevice->connectTerminals(cameraTrigReadyTerm.toStdString().c_str(),
                                cameraTriggerTerm.toStdString().c_str(), false);
}

///
/// \brief CADaq::generateGalvoWfm
/// \param lower
/// \param upper
/// \param slopeSign
///
void CADaq::generateGalvoWfm(double lower, double upper, int slopeSign)
{
    if (lower < upper){
        double step = (upper - lower)/1024;
        double volt = lower;

        if (slopeSign < 0){
            volt = upper;
            step *= -1;
        }

        galvoVolts.clear();
        if(abs(slopeSign) == 2){
            step /= 2;
            for(int ii = 0; ii < 2048; ii++){
                galvoVolts.push_back(volt);
                volt += step;
            }
        } else {
            for(int ii = 0; ii < 1024; ii++){
                galvoVolts.push_back(volt);
                volt += step;
            }

            for(int ii = 1; ii < 1024; ii++){
                galvoVolts.push_back(galvoVolts.at(1024-ii));
            }
        }
    }
}

///
/// \brief CADaq::deviceNameChanged
/// \param name
///
void CADaq::deviceNameChanged(QString name)
{
    QMap<QString, QStringList> channels = daqDevice->deviceChannels(name);
    emit(daqChannelsChanged(channels["analogOutputs"],
            channels["inputCounters"], channels["outputCounters"], channels["terminals"]));
}

///
/// \brief CADaq::galvoClockSourceChanged
/// \param terminal
///
void CADaq::galvoClockSourceChanged(QString terminal)
{
    galvoClockSource = terminal;
}


///
/// \brief CADaq::galvoVoltChannelChanged
/// \param chan
///
void CADaq::galvoVoltChannelChanged(QString chan)
{
    galvoVoltChannel = chan;
    daqDevice->clearTask(galvoScanTask);
    daqDevice->createAnalogOutputTask("galvoScanTask",&galvoScanTask,
                                      galvoVoltChannel.toStdString().c_str(), -10, 10);
    double rate = (double) (galvoVolts.count() * galvoFreq);
    daqDevice->configureSampClkTimingInternal(galvoScanTask,
                                              galvoVolts.count(),rate, 0);
    daqDevice->analogOutputWriteDoubleVector(galvoScanTask, galvoVolts.data(),
                                             galvoVolts.count());
    daqDevice->startTask(galvoScanTask);
}

///
/// \brief CADaq::cameraTrigChannelChanged
/// \param chan
///
void CADaq::cameraTrigChannelChanged(QString chan)
{
    cameraTriggerTerm = chan;
}

///
/// \brief CADaq::cameraTrigReadyChannelChanged
/// \param chan
///
void CADaq::cameraTrigReadyChannelChanged(QString chan)
{
    cameraTrigReadyChannel = chan;
    daqDevice->clearTask(cameraTrigReadyTask);
    daqDevice->createSemiPeriodCounterTask("cameraTrigReadyTask",&cameraTrigReadyTask,
                                           cameraTrigReadyChannel.toStdString().c_str());
    daqDevice->configureSamplesToAcquire(cameraTrigReadyTask, 4, false);
    daqDevice->enableTaskDoneEvent(cameraTrigReadyTask, CADaq::periodMeasureCallback,
                                   reinterpret_cast<void*>(this));
    char name[32];
    daqDevice->getSemiPeriodCounterTerminal(cameraTrigReadyTask,cameraTrigReadyChannel.toStdString().c_str(),
                                            name,32);
    cameraTrigReadyTerm = QString(name);
    /// Do not start the task.
}

///
/// \brief CADaq::galvoPositionChanged
/// \param mVolt
///
void CADaq::galvoPositionChanged(int mVolt)
{
    if (galvoScan == 2){
        double temp = mVolt* 1.0E-3;
        double wdata[2] = {temp, temp};
        daqDevice->stopTask(galvoScanTask);
        daqDevice->analogOutputWriteDoubleVector(galvoScanTask,
                                                 wdata,2);
        daqDevice->startTask(galvoScanTask);
    }
}

///
/// \brief CADaq::galvoLowerLimitChanged
/// \param val
///
void CADaq::galvoLowerLimitChanged(int val)
{
    if (galvoScan != 2){
        daqDevice->stopTask(galvoScanTask);
    }
    int sign = waveformSign(galvoScan);
    galvoVoltLowerLimit = val * 1.0E-3;
    galvoVoltUpperLimit = (galvoVoltLowerLimit < galvoVoltUpperLimit)?
                galvoVoltUpperLimit:galvoVoltLowerLimit;
    generateGalvoWfm(galvoVoltLowerLimit,
                     galvoVoltUpperLimit,
                     sign);
    if (galvoScan != 2){
        daqDevice->analogOutputWriteDoubleVector(galvoScanTask, galvoVolts.data(),
                                                 galvoVolts.count());
        daqDevice->startTask(galvoScanTask);
    }
}

///
/// \brief CADaq::galvoUpperLimitChanged
/// \param val
///
void CADaq::galvoUpperLimitChanged(int val)
{
    if (galvoScan != 2){
        daqDevice->stopTask(galvoScanTask);
    }
    int sign = waveformSign(galvoScan);
    galvoVoltUpperLimit = val * 1.0E-3;
    galvoVoltLowerLimit = (galvoVoltLowerLimit < galvoVoltUpperLimit)?
                galvoVoltLowerLimit:galvoVoltUpperLimit;
    generateGalvoWfm(galvoVoltLowerLimit,
                     galvoVoltUpperLimit,
                     sign);
    if (galvoScan != 2){
        daqDevice->analogOutputWriteDoubleVector(galvoScanTask, galvoVolts.data(),
                                                 galvoVolts.count());
        daqDevice->startTask(galvoScanTask);
    }
}

///
/// \brief CADaq::galvoTrigDelayChanged
/// \param msec
///
void CADaq::galvoTrigDelayChanged(int msec)
{
    galvoTrigDelay = msec * 1.0E-4; //not ms but ms/10
    if(galvoCamSynchronize){
        synchronizeCameraWithGalvo(true);
    }
}

///
/// \brief CADaq::updateFramePeriod
///
void CADaq::updateFramePeriod()
{
    double period[4];
    int nsamples = 4;

    if(0 == daqDevice->counterReadDoubleVector(cameraTrigReadyTask, period,&nsamples)){
        /*double fullPeriod = period[2] + period[3];
        double frameRate = 1.0/fullPeriod;
        qDebug("Framerate = %.0f", frameRate);*/
    }
}

///
/// \brief CADaq::framePeriodChanged
///
void CADaq::framePeriodChanged()
{
    emit(framePeriodChangedSignal());
}

///
/// \brief CADaq::periodMeasureCallback
/// \param taskHandle
/// \param eventType
/// \param callbackData
/// \return
///
int CADaq::periodMeasureCallback(void* taskHandle,
                                 int eventType,
                                 void *callbackData)
{
    Q_UNUSED(eventType);
    Q_UNUSED(taskHandle);

    CADaq* objHandle = reinterpret_cast<CADaq*>(callbackData);
    objHandle->framePeriodChanged();

    return 0;
}

///
/// \brief CADaq::galvoScanTypeChanged
/// \param val
///
void CADaq::galvoScanTypeChanged(int val)
{
    daqDevice->stopTask(galvoScanTask);
    double rate = 0.0;

    if (val != 2){
        int sign = waveformSign(val);
        generateGalvoWfm(galvoVoltLowerLimit,
                         galvoVoltUpperLimit,
                         sign);
        rate = galvoVolts.count() * galvoFreq;
        daqDevice->configureSampClkTimingInternal(galvoScanTask,
                                          galvoVolts.count(),
                                          rate, 0);
        daqDevice->analogOutputWriteDoubleVector(galvoScanTask,
                                                 galvoVolts.data(),
                                                 galvoVolts.count());
    }else {
        double wdata[2] = {0.0, 0.0};
        rate = 1.0E4;
        daqDevice->configureSampClkTimingInternal(galvoScanTask,
                                          2,rate, 1);
        daqDevice->analogOutputWriteDoubleVector(galvoScanTask,
                                                 wdata, 2);
    }
    galvoScan = val;
    daqDevice->startTask(galvoScanTask);
}
