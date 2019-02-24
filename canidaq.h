#ifndef CANiDaq_H
#define CANiDaq_H

#include <QObject>
#include <QMap>

#ifndef __APPLE__
#include "NIDAQmx.h"
#endif

#include <stdint.h>

#ifndef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0
#endif

class CANiDaq : public QObject
{
    Q_OBJECT

public:
    explicit CANiDaq(QObject *parent = 0);
    ~CANiDaq();
    QString deviceName;
    QMap<QString, QStringList> deviceChannels(QString name);

    /// Task creation
    int createAnalogInputTask(const char * taskName, void ** task, const char * channel, double min, double max);
    int createAnalogOutputTask(const char * taskName, void ** task, const char * channel, double min, double max);
    int createFuncGenTask(const char * taskName, void ** task, const char * channel, double freq, double ampl);
    int createDigitalInputTask(const char * taskName, void ** task, const char * channel);
    int createDigitalOutputTask(const char * taskName, void ** task, const char * channel);
    int createAngularEncoderTask(const char * taskName, void ** task, const char * cntName, int maxCount);
    int createPulseGenCounterTask(const char * taskName, void** task, const char * cntName, double freq);
    int createSemiPeriodCounterTask(const char * taskName,void** task, const char * cntName);

    /// start/stop/clear
    int clearTask(void * task);
    int startTask(void * taskHandle);
    int stopTask(void * taskHandle);

    /// triggering methods
    int trigWithCounter(void * task, const char name [], double delay_s, bool retrig);
    int trigWithTerminal(void * task, const char term[],double delay_s, bool retrig);

    ///
    int connectTerminals(const char source[], const char destination[], bool invert);
    int disconnectTerminals(const char source[], const char destination[]);
    int getSemiPeriodCounterTerminal(void * taskHandle, const char channelName [], char * terminalName, int size);

    /// Events
    int enableDigitalInputEvent(void * task, int (*callbackFunction)(void *,int, void *),
                                void * callbackData);
    int enableTaskDoneEvent( void * task, int (*callbackFunction)(void *,int, void *),
                             void * callbackData);
    int enableEveryNSamplesEvent(void * task, int nsamples,
                                 int (*callbackFunction)(void *, int, void *),
                                 void * callbackData);
    /// read methods
    int counterReadDoubleVector(void * taskHandle, double * buffer, int * samplesToRead);
    int counterReadRawVector(void * taskHandle, int32_t * buffer, int * samplesToRead);
    int analogInputReadShortVector(void * taskHandle, int16_t * buffer, int * samplesToRead);

    /// clock and buffer config. methods
    int configureSampClkTiming(void * taskHandle,const char clksrc[],
                                    int bufferSize,double rate, int mode);
    int configureSampClkTimingInternal(void * taskHandle,int bufferSize,
                                             double rate, int mode);
    int configureSampClkTimingAO(void * taskHandle,int bufferSize,
                                             double rate, int mode);
    int configureSampClkTimingAI(void * taskHandle,int bufferSize,
                                             double rate, int mode);
    int configureSamplesToAcquire(void * task, unsigned long buffersize,
                                  bool continuous);
    int setPulseGenCounterFreq(void * task, double freq);

    /// write methods
    int analogOutputWriteDoubleVector(void * taskHandle, double * buffer,
                                      int samplesToWrite);
    int analogOutputWriteDoubleScalar(void * taskHandle, double data);

private:
    QList<QString> daqTerminals(QString name);
    QList<QString> daqDiChannels(QString name);
    QList<QString> daqDoChannels(QString name);
    QList<QString> daqAiChannels(QString name);
    QList<QString> daqAoChannels(QString name);
    QList<QString> daqCIChannels(QString name);
    QList<QString> daqCOChannels(QString name);

    /// channels creation methos
    int createTask(const char * taskName, void ** task);
    int createAnalogOutputChannel(void * task, const char * name, double min, double max);
    int createFuncGenChannel(void * task, const char * name, double freq, double ampl);
    int createAnalogInputChannel(void * task, const char * name, double min, double max);
    int createDigitalOutputChannel(void * task, const char * name);
    int createDigitalInputChannel(void * task, const char * name);
    int createAngularEncoderCounter(void * task, const char * cntName, int maxCount);
    int createPulseGenCounter(void * task, const char * cntName, double freq);
    int createSemiPeriodCounter(void * task, const char * cntName);
    void displayError(const char text[]);
};

#endif // CANiDaq_H
