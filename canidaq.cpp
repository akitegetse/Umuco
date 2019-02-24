#include "canidaq.h"
#include <QMap>
#include <QMessageBox>

#define DAQ_N_SAMPLES_EVENT     (2048)
#define DAQ_INPUT_BUFFER_SIZE   (2048)
#define DAQ_OUTPUT_BUFFER_SIZE  (2048)
#define DAQ_SAMPLE_FRQ          (6.0E5)
#define DAQ_MAX_SAMPLE_FREQ     (1.2E6)

#ifndef __APPLE__
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error;
#endif

///
/// \brief CANiDaq::CANiDaq
/// \param parent
///
CANiDaq::CANiDaq(QObject *parent) :
    QObject(parent)
{
}

///
/// \brief CANiDaq::~CANiDaq
///
CANiDaq::~CANiDaq()
{
}

///
/// \brief CANiDaq::createAnalogInputTask
/// \param taskName
/// \param task
/// \param channel
/// \param min
/// \param max
/// \return
///
int CANiDaq::createAnalogInputTask(const char * taskName, void ** task,
                                   const char * channel, double min, double max)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createAnalogInputChannel(*task, channel, min, max);
    }

    return error;
}

///
/// \brief CANiDaq::createAnalogOutputTask
/// \param taskName
/// \param task
/// \param channel
/// \param min
/// \param max
/// \return
///
int CANiDaq::createAnalogOutputTask(const char * taskName, void ** task,
                                    const char * channel, double min, double max)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createAnalogOutputChannel(*task, channel, min, max);
    }

    return error;
}

///
/// \brief CANiDaq::createFuncGenTask
/// \param taskName
/// \param task
/// \param channel
/// \param freq
/// \param ampl
/// \return
///
int CANiDaq::createFuncGenTask(const char * taskName, void ** task,
                                           const char * channel, double freq, double ampl)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createFuncGenChannel(*task, channel, freq, ampl);
    }

    return error;
}

///
/// \brief CANiDaq::createDigitalInputTask
/// \param taskName
/// \param task
/// \param channel
/// \return
///
int CANiDaq::createDigitalInputTask(const char * taskName,
                                    void ** task, const char * channel)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createDigitalInputChannel(*task, channel);
    }

    return error;
}

///
/// \brief CANiDaq::createAnalogOutputTask
/// \param taskName
/// \param task
/// \param channel
/// \return
///
int CANiDaq::createDigitalOutputTask(const char * taskName,
                                     void ** task, const char * channel)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createDigitalOutputChannel(*task, channel);
    }

    return error;
}

///
/// \brief CANiDaq::createAngularEncoderTask
/// \param taskName
/// \param task
/// \param cntName
/// \param maxCount
/// \return
///
int CANiDaq::createAngularEncoderTask(const char * taskName, void** task,
                                      const char * cntName, int maxCount)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createAngularEncoderCounter(*task, cntName, maxCount);
    }

    return error;
}

///
/// \brief CANiDaq::createPulseGenCounterTask
/// \param taskName
/// \param task
/// \param cntName
/// \param freq
/// \return
///
int CANiDaq::createPulseGenCounterTask(const char * taskName,
                                       void** task, const char * cntName,
                                       double freq)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createPulseGenCounter(*task, cntName, freq);
    }

    return error;
}

///
/// \brief CANiDaq::createSemiPeriodCounterTask
/// \param taskName
/// \param task
/// \param cntName
/// \return
///
int CANiDaq::createSemiPeriodCounterTask(const char * taskName,
                                       void** task, const char * cntName)
{
    int error = createTask(taskName, task);
    if(!error){
        error = createSemiPeriodCounter(*task, cntName);
    }

    return error;
}

///
/// \brief CANiDaq::createTask
/// \param taskName
/// \param task
/// \return
///
int CANiDaq::createTask(const char * taskName, void ** task)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxCreateTask(taskName, (TaskHandle*)task));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::clearTask
/// \param task
/// \return
///
int CANiDaq::clearTask(void * task)
{
    int error = 0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxClearTask((TaskHandle)task));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::startTask
/// \param taskHandle
/// \return
///
int CANiDaq::startTask(void * taskHandle)
{
    int         error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};

        DAQmxErrChk(DAQmxStartTask((TaskHandle)taskHandle));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::stopTask
/// \param taskHandle
/// \return
///
int CANiDaq::stopTask(void * taskHandle)
{
    int         error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};

        DAQmxErrChk(DAQmxStopTask((TaskHandle)taskHandle));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}
///
/// \brief CANiDaq::daqAiChannels
/// \param name
/// \return
///
QList<QString> CANiDaq::daqAiChannels(QString name)
{
    QList<QString> theList;
    char channelsNames[1024];
    ///
    /// Get comma-delimited list of the analog input
    /// channels installed in the system
    ///
#ifndef __APPLE__
    DAQmxGetDevAIPhysicalChans(name.toStdString().c_str(), channelsNames, 1024);
#endif

    char *token;

    /* get the first token */
    token = strtok(channelsNames, ",");

    /* walk through other tokens */
    while( token != NULL ){
        theList.append(token);
        token = strtok(NULL, ",");
    }
    return theList;
}

///
/// \brief CANiDaq::daqAoChannels
/// \param name
/// \return
///
QList<QString> CANiDaq::daqAoChannels(QString name)
{
    QList<QString> theList;
    char channelsNames[1024];
    ///
    /// Get comma-delimited list of the analog output
    /// channels installed in the system
    ///
#ifndef __APPLE__
    DAQmxGetDevAOPhysicalChans(name.toStdString().c_str(), channelsNames, 1024);
#endif

    char *token;

    /* get the first token */
    token = strtok(channelsNames, ",");

    /* walk through other tokens */
    while( token != NULL ){
        theList.append(token);
        token = strtok(NULL, ",");
    }
    return theList;
}

///
/// \brief CANiDaq::daqTerminals
/// \param name
/// \return
///
QList<QString> CANiDaq::daqTerminals(QString name)
{
    QList<QString> theList;
    char terminalNames[4096];
    ///
    /// Get comma-delimited list of the analog input
    /// channels installed in the system
    ///
#ifndef __APPLE__
    DAQmxGetDevTerminals(name.toStdString().c_str(), terminalNames, 4096);
#endif

    char *token;

    /* get the first token */
    token = strtok(terminalNames, ",");

    /* walk through other tokens */
    while( token != NULL ){
        theList.append(token);
        token = strtok(NULL, ",");
    }
    return theList;
}

///
/// \brief CANiDaq::daqDiChannels
/// \param name
/// \return
///
QList<QString> CANiDaq::daqDiChannels(QString name)
{
    QList<QString> theList;
    char channelsNames[1024];
    ///
    /// Get comma-delimited list of the analog input
    /// channels installed in the system
    ///
#ifndef __APPLE__
    DAQmxGetDevDILines(name.toStdString().c_str(), channelsNames, 1024);
#endif

    char *token;

    /* get the first token */
    token = strtok(channelsNames, ",");

    /* walk through other tokens */
    while( token != NULL ){
        theList.append(token);
        token = strtok(NULL, ",");
    }
    return theList;
}

///
/// \brief CANiDaq::daqDoChannels
/// \param name
/// \return
///
QList<QString> CANiDaq::daqDoChannels(QString name)
{
    QList<QString> theList;
    char channelsNames[1024];
    ///
    /// Get comma-delimited list of the analog output
    /// channels installed in the system
    ///
#ifndef __APPLE__
    DAQmxGetDevDOLines(name.toStdString().c_str(), channelsNames, 1024);
#endif

    char *token;

    /* get the first token */
    token = strtok(channelsNames, ",");

    /* walk through other tokens */
    while( token != NULL ){
        theList.append(token);
        token = strtok(NULL, ",");
    }
    return theList;
}

///
/// \brief CANiDaq::daqCIChannels
/// \param name
/// \return
///
QList<QString> CANiDaq::daqCIChannels(QString name)
{
    QList<QString> theList;
    char channelsNames[1024];
    ///
    /// Get comma-delimited list of the counter
    /// channels installed in the system
    ///
#ifndef __APPLE__
    DAQmxGetDevCIPhysicalChans(name.toStdString().c_str(), channelsNames, 1024);
#endif

    char *token;

    /* get the first token */
    token = strtok(channelsNames, ",");

    /* walk through other tokens */
    while( token != NULL ){
        theList.append(token);
        token = strtok(NULL, ",");
    }
    return theList;
}

///
/// \brief CANiDaq::daqCOChannels
/// \param name
/// \return
///
QList<QString> CANiDaq::daqCOChannels(QString name)
{
    QList<QString> theList;
    char channelsNames[1024];
    ///
    /// Get comma-delimited list of the counter
    /// channels installed in the system
    ///
#ifndef __APPLE__
    DAQmxGetDevCOPhysicalChans(name.toStdString().c_str(), channelsNames, 1024);
#endif

    char *token;

    /* get the first token */
    token = strtok(channelsNames, ",");

    /* walk through other tokens */
    while( token != NULL ){
        theList.append(token);
        token = strtok(NULL, ",");
    }
    return theList;
}

///
/// \brief CANiDaq::deviceChannels
/// \param deviceName
/// \return
///
QMap<QString, QStringList> CANiDaq::deviceChannels(QString name)
{    
    QMap<QString, QStringList> channels;
    channels["analogInputs"]   = this->daqAoChannels(name);
    channels["analogOutputs"]  = this->daqAoChannels(name);
    channels["digitalInputs"]  = this->daqDiChannels(name);
    channels["digitalOutputs"] = this->daqDoChannels(name);
    channels["outputCounters"] = this->daqCOChannels(name);
    channels["inputCounters"] = this->daqCIChannels(name);
    channels["terminals"] = this->daqTerminals(name);

    deviceName = name;

    return channels;
}

///
/// \brief CANiDaq::createAnalogOutputChannel
/// \param task
/// \param name
/// \param min
/// \param max
/// \return
///
int CANiDaq::createAnalogOutputChannel(void * task, const char * name,
                                       double min, double max)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxCreateAOVoltageChan((TaskHandle)task, name, NULL,
                                         min, max,
                                         DAQmx_Val_Volts, NULL));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::createFuncGenChannel
/// \param task
/// \param name
/// \param freq
/// \param ampl
/// \return
///
int CANiDaq::createFuncGenChannel(void * task, const char * name,
                                              double freq, double ampl)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxCreateAOFuncGenChan((TaskHandle)task, name, NULL,
                                         DAQmx_Val_Sawtooth, freq, ampl, 0));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::createAnalogInputChannel
/// \param task
/// \param name
/// \param min
/// \param max
/// \return
///
int CANiDaq::createAnalogInputChannel(void * task, const char * name,
                                      double min, double max)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxCreateAIVoltageChan((TaskHandle) task, name, "", DAQmx_Val_RSE,
                                         (double) min,(double) max,
                                         DAQmx_Val_Volts,NULL));
    DAQmxErrChk(DAQmxSetBufInputBufSize((TaskHandle)task, DAQ_INPUT_BUFFER_SIZE));
    DAQmxErrChk(DAQmxCfgSampClkTiming((TaskHandle)task, NULL, DAQ_SAMPLE_FRQ,
                                      DAQmx_Val_Rising,
                                      DAQmx_Val_ContSamps,DAQ_INPUT_BUFFER_SIZE));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::createDigitalOutputChannel
/// \param task
/// \param name
/// \return
///
int CANiDaq::createDigitalOutputChannel(void * task, const char * name)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxCreateDOChan((TaskHandle)task, name, NULL,
                                  DAQmx_Val_ChanPerLine));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::createDigitalInputChannel
/// \param task
/// \param name
/// \return
///
int CANiDaq::createDigitalInputChannel(void * task, const char * name)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxCreateDIChan((TaskHandle)task, name, NULL, DAQmx_Val_ChanPerLine));
    DAQmxErrChk(DAQmxSetBufInputBufSize((TaskHandle)task, DAQ_INPUT_BUFFER_SIZE));
    DAQmxErrChk(DAQmxCfgSampClkTiming((TaskHandle)task, NULL, DAQ_SAMPLE_FRQ, DAQmx_Val_Rising,
                                      DAQmx_Val_ContSamps,DAQ_INPUT_BUFFER_SIZE));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::createAngularEncoderCounter
/// \param task
/// \param cntName
/// \param maxCount
/// \return
///
int CANiDaq::createAngularEncoderCounter(void * task, const char * cntName, int maxCount)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    /// Create angle encoder channel
    DAQmxErrChk(DAQmxCreateCIAngEncoderChan((TaskHandle)task, cntName, "",
                                            DAQmx_Val_X1 , true, 0.0,
                                            DAQmx_Val_AHighBHigh , DAQmx_Val_Degrees,
                                            maxCount, 0.0, NULL));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::createPulseGenCounter
/// \param task
/// \param cntName
/// \param freq
/// \return
///
int CANiDaq::createPulseGenCounter(void * task, const char * cntName, double freq)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxCreateCOPulseChanFreq(task, cntName,"",
                                           DAQmx_Val_Hz,DAQmx_Val_Low,
                                           0.0,freq,0.1));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

int CANiDaq::createSemiPeriodCounter(void * task, const char * cntName)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk (DAQmxCreateCISemiPeriodChan(task,cntName,"",1.0E-2,10.0, DAQmx_Val_Seconds,""));
    DAQmxErrChk (DAQmxSetCISemiPeriodStartingEdge(task, "", DAQmx_Val_Rising));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::configureSampClkTiming
/// \param taskHandle
/// \param clksrc
/// \param bufferSize
/// \param rate
/// \param mode
/// \return
///
int CANiDaq::configureSampClkTiming(void * taskHandle,const char clksrc[],
                                    int bufferSize,
                                    double rate, int mode)
{
    int error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};

    switch(mode)
    {
    case 1:
        DAQmxErrChk(DAQmxCfgSampClkTiming((TaskHandle)taskHandle,clksrc,(float64)rate,
                                          DAQmx_Val_Rising,
                                          DAQmx_Val_FiniteSamps ,bufferSize));
        break;
    case 2:
        DAQmxErrChk(DAQmxCfgSampClkTiming((TaskHandle)taskHandle,clksrc, rate,
                                          DAQmx_Val_Rising,
                                          DAQmx_Val_HWTimedSinglePoint,1));
        break;
    default:
        DAQmxErrChk(DAQmxCfgSampClkTiming((TaskHandle)taskHandle,clksrc,(float64)rate,
                                          DAQmx_Val_Rising,
                                          DAQmx_Val_ContSamps ,bufferSize));
        break;
    }

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::configureSampClkTimingInternal
/// \param taskHandle
/// \param bufferSize
/// \param rate
/// \param mode
/// \return
///
int CANiDaq::configureSampClkTimingInternal(void * taskHandle,
                                            int bufferSize,
                                         double rate, int mode)
{
    return configureSampClkTiming(taskHandle,"",
                           bufferSize,rate, mode);
}

///
/// \brief CANiDaq::configureSampClkTimingAO
/// \param taskHandle
/// \param bufferSize
/// \param rate
/// \param mode
/// \return
///
int CANiDaq::configureSampClkTimingAO(void * taskHandle,
                                      int bufferSize,
                                      double rate, int mode)
{
    return configureSampClkTiming(taskHandle,"ao/SampleClock",
                           bufferSize,rate, mode);
}

///
/// \brief CANiDaq::configureSampClkTimingAI
/// \param taskHandle
/// \param bufferSize
/// \param rate
/// \param mode
/// \return
///
int CANiDaq::configureSampClkTimingAI(void * taskHandle,
                                      int bufferSize,
                                      double rate, int mode)
{
    return configureSampClkTiming(taskHandle,"ai/SampleClock",
                           bufferSize,rate, mode);
}

///
/// \brief CANiDaq::configureSamplesToAcquire
/// \param task
/// \param buffersize
/// \param continuous
/// \return
///
int CANiDaq::configureSamplesToAcquire(void * task,
                             unsigned long buffersize,
                             bool continuous)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    if (continuous){
        DAQmxErrChk (DAQmxCfgImplicitTiming(task,DAQmx_Val_ContSamps,(uInt64) buffersize));
    } else {
        DAQmxErrChk (DAQmxCfgImplicitTiming(task,DAQmx_Val_FiniteSamps,(uInt64) buffersize));
    }

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::setPulseGenCounterFreq
/// \param task
/// \param freq
/// \return
///
int CANiDaq::setPulseGenCounterFreq(void * task, double freq)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxSetCOPulseFreq(task, "", (float64) freq));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::trigWithCounter
/// \param task : the task to trigger with counter
/// \param counterName: the counter to use as trigger
/// \param delay_s: the trigger time delay in seconds
/// \return
///
int CANiDaq::trigWithCounter(void * task, const char counterName[],
                             double delay_s, bool retrig)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};
    delay_s = (delay_s < 1E-7)?1.0E-7:delay_s;

    QString theName = QString("/%1%2").arg(counterName).arg("InternalOutput");

    DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(task, theName.toStdString().c_str(),
                                         DAQmx_Val_Rising));
    DAQmxErrChk(DAQmxSetStartTrigRetriggerable(task, retrig));
    DAQmxSetStartTrigDelayUnits(task, DAQmx_Val_Seconds);
    DAQmxSetStartTrigDelay(task,delay_s);

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::connectTerminals
/// \param source
/// \param destination
/// \param invert
/// \return
///
int CANiDaq::connectTerminals(const char source[], const char destination[], bool invert)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxConnectTerms (source, destination,
                                   invert? DAQmx_Val_InvertPolarity:DAQmx_Val_DoNotInvertPolarity));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::disconnectTerminals
/// \param source
/// \param destination
/// \return
///
int CANiDaq::disconnectTerminals(const char source[], const char destination[])
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxDisconnectTerms (source, destination));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::trigWithTerminal
/// \param task
/// \param term
/// \param delay_s
/// \param retrig
/// \return
///
int CANiDaq::trigWithTerminal(void * task, const char term[],
                             double delay_s, bool retrig)
{
    Q_UNUSED(delay_s);
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};
    //delay_s = (delay_s < 1E-7)?1.0E-7:delay_s;

    DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(task, term,DAQmx_Val_Rising));
    DAQmxErrChk(DAQmxSetStartTrigRetriggerable(task, retrig));
    /*DAQmxSetStartTrigDelayUnits(task, DAQmx_Val_Seconds);
    DAQmxSetStartTrigDelay(task,delay_s);*/

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::getSemiPeriodCounterTerminal
/// \param taskHandle
/// \param channelName
/// \param terminalName
/// \param size
/// \return
///
int CANiDaq::getSemiPeriodCounterTerminal(void * taskHandle, const char channelName [], char * terminalName, int size)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxGetCISemiPeriodTerm((TaskHandle)taskHandle, channelName, terminalName, size));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::enableEveryNSamplesEvent
/// \param task
/// \param nsamples
/// \param callbackData
/// \return
///
int CANiDaq::enableEveryNSamplesEvent(void * task, int nsamples,
                                     int (*callbackFunction)(void * ,int, void *),
                                     void * callbackData)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(task,
                                                DAQmx_Val_Acquired_Into_Buffer,
                                                (uInt32)nsamples, 0,
                                                reinterpret_cast<DAQmxEveryNSamplesEventCallbackPtr> (callbackFunction),
                                                callbackData));


Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}


///
/// \brief CANiDaq::enableDigitalInputEvent
/// \param task
/// \param callbackFunction
/// \return
///
int CANiDaq::enableDigitalInputEvent(void * task,
                                     int (*callbackFunction)(void *,int, void *),
                                     void * callbackData)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxRegisterSignalEvent((TaskHandle)task,DAQmx_Val_ChangeDetectionEvent,0,
                                         reinterpret_cast<DAQmxSignalEventCallbackPtr> (callbackFunction),
                                         callbackData));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::enableTaskDoneEvent
/// \param task
/// \param callbackFunction
/// \param callbackData
/// \return
///
int CANiDaq::enableTaskDoneEvent(void * task,
                                 int (*callbackFunction)(void * ,int, void *),
                                 void * callbackData)
{
    int error=0;
#ifndef __APPLE__
    char errBuff[2048]={'\0'};

    //Unregister
    DAQmxErrChk(DAQmxRegisterDoneEvent((TaskHandle)task,0,NULL,NULL));
    //Register
    DAQmxErrChk(DAQmxRegisterDoneEvent((TaskHandle)task, 0,
                                       reinterpret_cast<DAQmxSignalEventCallbackPtr>(callbackFunction),
                                       callbackData));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif

    return error;
}

///
/// \brief CANiDaq::counterReadDoubleVector
/// \param taskHandle
/// \param buffer
/// \param samplesToRead
/// \return
///
int CANiDaq::counterReadDoubleVector(void * taskHandle, double * buffer,
                                     int * samplesToRead)
{
    int         error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};
    int32       readSamples;

    DAQmxErrChk(DAQmxReadCounterF64((TaskHandle)taskHandle,-1,0,
                                    buffer,*samplesToRead, &readSamples,NULL));
    *samplesToRead = readSamples;

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        qDebug("%s",errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::counterReadRawVector
/// \param taskHandle
/// \param buffer
/// \param samplesToRead
/// \return
///
int CANiDaq::counterReadRawVector(void * taskHandle, int32_t * buffer,
                                  int * samplesToRead)
{
    int         error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};
    int32       readSamples;
    int32       numBytesPerSample;

    DAQmxErrChk(DAQmxReadRaw((TaskHandle)taskHandle,-1,0,
                             buffer,(*samplesToRead) * 4,
                             &readSamples,&numBytesPerSample, NULL));

    *samplesToRead = readSamples;

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        qDebug("%s",errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::digitalInputReadShortVector
/// \param taskHandle
/// \param buffer
/// \param samplesToRead
/// \return
///
int CANiDaq::analogInputReadShortVector(void * taskHandle, int16_t * buffer,
                                        int * samplesToRead)
{
    int         error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};
    int32       readSamples;

    DAQmxErrChk(DAQmxReadBinaryI16((TaskHandle)taskHandle,-1,0,
                                   DAQmx_Val_GroupByChannel,
                                   buffer,*samplesToRead,&readSamples,NULL));
    *samplesToRead = readSamples;

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        qDebug("%s",errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::analogOutputWriteDoubleVector
/// \param taskHandle
/// \param buffer
/// \param samplesToWrite
/// \return
///
int CANiDaq::analogOutputWriteDoubleVector(void * taskHandle, double * buffer,
                                           int samplesToWrite)
{
    int error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};


    DAQmxErrChk(DAQmxWriteAnalogF64((TaskHandle)taskHandle,samplesToWrite,false,
                                    -1, DAQmx_Val_GroupByChannel,buffer,NULL,NULL));
Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::analogOutputWriteDoubleScalar
/// \param taskHandle
/// \param data
/// \return
///
int CANiDaq::analogOutputWriteDoubleScalar(void * taskHandle, double data)
{
    int error=0;
#ifndef __APPLE__
    char        errBuff[2048]={'\0'};

    DAQmxErrChk(DAQmxWriteAnalogScalarF64((TaskHandle)taskHandle, true,
                                    0, data, NULL));

Error:
    if( DAQmxFailed(error) ){
        DAQmxGetExtendedErrorInfo(errBuff,2048);
        displayError(errBuff);
    }
#endif
    return error;
}

///
/// \brief CANiDaq::displayError
/// \param text
///
void CANiDaq::displayError(const char text[])
{
//    QWidget msgBox;
//    QMessageBox::warning(&msgBox, tr("NI DAQ error"),
//                         tr(text),
//                         QMessageBox::Ok);
    qDebug("%s",text);
}
