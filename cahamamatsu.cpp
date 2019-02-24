#include "cahamamatsu.h"
#include "cahamamatsuwindow.h"
#include "caconstants.h"

#include <QtConcurrent/QtConcurrent>

#if !__APPLE__
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#include <PowrProf.h>
#include <windows.h>
#endif
///
/// \brief PowerState
/// \param bEnable
///
void PowerState(bool bEnable)
{
    // CPU idle state
#if !__APPLE__
    GUID *scheme;
    PowerGetActiveScheme(NULL, &scheme);
    PowerWriteACValueIndex(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_IDLE_DISABLE, bEnable ? 0 :  1);
    PowerSetActiveScheme(NULL, scheme);
#endif
}

///
/// \brief CAHamamatsu::CAHamamatsu
///
CAHamamatsu::CAHamamatsu():
    CACamera(),
    window(new CAHamamatsuWindow),
    device(new CAHamamatsuDevice)
{
    sprintf(deviceName, "Hamamatsu");

    //disable CPU idle state
    PowerState(false);

    qRegisterMetaType<QVector<int> >("QVector<int>");
    qRegisterMetaType<QMap<QString,int> >("QMap<QString,int>");
    qRegisterMetaType<QMap<QString,QMap<QString,QList<QString> > > >("QMap<QString,QMap<QString,QList<QString> > >");
    qRegisterMetaType<CARange<double> >("CARange<double>");
    qRegisterMetaType<CAImage >("CAImage");
    qRegisterMetaType<CARoi >("CARoi");

    connect(this, SIGNAL(deviceOpen()), device, SLOT(tryOpen()));
    connect(device, SIGNAL(online()), this, SLOT(deviceEstablishLink()));

    //start the thread
    device->moveToThread(this);
    start();

    emit(deviceOpen());
}

///
/// \brief CAHamamatsu::~CAHamamatsu
///
CAHamamatsu::~CAHamamatsu()
{
    //restore CPU idle state
    //PowerState(true);
    delete device;
    delete window;
    exit(0);
    while(isFinished());
}

///
/// \brief CAHamamatsu::deviceEstablishLink
///
void CAHamamatsu::deviceEstablishLink()
{
    //Interface actions
    connect(window, SIGNAL(exposureChanged(double)),
            device, SLOT(onExposureChanged(double)));
    connect(window, SIGNAL(exposureChanged(double)),
            this, SIGNAL(exposureChanged(double)));
    connect(window, SIGNAL(internalLineIntervalChanged(double)),
            device, SLOT(onInternalLineIntervalChanged(double)));
    connect(window, SIGNAL(binningChanged(long)), this, SLOT(serviceBinningChanged(long)));
    connect(this, SIGNAL(binningChanged(long)), device, SLOT(onBinningChanged(long)));
    connect(window, SIGNAL(triggerPolarityChanged(QString)), device, SLOT(onTriggerPolarityChanged(QString)));
    connect(window, SIGNAL(triggerSourceChanged(QString)), device, SLOT(onTriggerSourceChanged(QString)));
    connect(window, SIGNAL(triggerTypeChanged(QString)), device, SLOT(onTriggerTypeChanged(QString)));
    connect(window, SIGNAL(triggerOutputChanged(int , QString)), device, SLOT(onTriggerOutputChanged(int , QString)));
    connect(window, SIGNAL(sensorModeChanged(QString)), device, SLOT(onSensorModeChanged(QString)));
    connect(window, SIGNAL(scanModeChanged(long)), device, SLOT(onScanModeChanged(long)));
    connect(window, SIGNAL(globalExposureChanged(QString)), device, SLOT(onGlobalExposureChanged(QString)));
    connect(window, SIGNAL(readoutDirChanged(QString)), device, SLOT(onReadoutDirChanged(QString)));
    connect(window, SIGNAL(regionOfInterestChanged(CARoi)), device, SLOT(onROIChanged(CARoi)));
    connect(window, SIGNAL(pixelSizeChanged(QSize)), this, SLOT(servicePixelSizeAvailable(QSize)));
    //Interface update/initialization
    connect(device, SIGNAL(deviceInitialized(QMap<QString,int>,QVector<int>,QVector<int>,
                                             QMap<QString,int>,QMap<QString,int>)),
            window, SLOT(updateSensorInfo(QMap<QString,int>,QVector<int>,QVector<int>,
                                          QMap<QString,int>,QMap<QString,int>)),
            Qt::AutoConnection);
    connect(this, SIGNAL(roiChanged(CARoi)), window, SLOT(updateRegionOfInterest(CARoi)));
    connect(device, SIGNAL(sensorModesItems(QMap<QString,int>)),
            window, SLOT(updateSensorModes(QMap<QString,int>)));
    connect(device, SIGNAL(exposureLimits(CARange<double>)),
            window, SLOT(updateExposureLimits(CARange<double>)));
    connect(device, SIGNAL(internalLineIntervalBounds(CARange<double>)),
            window, SLOT(updateInternalLineIntervalBounds(CARange<double>)));
    connect(device, SIGNAL(triggerTypesItems(QMap<QString,int>)),
            window, SLOT(updateTriggerTypes(QMap<QString,int>)));
    connect(device, SIGNAL(triggerPolaritiesItems(QMap<QString,int>)),
            window, SLOT(updateTriggerPolarities(QMap<QString,int>)));
    connect(device, SIGNAL(globalExposureItems(QMap<QString,int>)),
            window, SLOT(updateGlobalExposure(QMap<QString,int>)));
    // acquisition actions (the command action comes from outside this scope)
    connect(this, SIGNAL(startCaptureRequest()), device, SLOT(startCapture()), Qt::AutoConnection);
    connect(this, SIGNAL(stopCaptureRequest()), device, SLOT(stopCapture()), Qt::AutoConnection);
    connect(this, SIGNAL(snapImageRequest()), device, SLOT(snapImage()), Qt::AutoConnection);

    //
    connect(device, SIGNAL(imageAvailable(CAImage)), this, SIGNAL(imageAvailable(CAImage)),Qt::QueuedConnection);
    connect(device, SIGNAL(roiChanged(CARoi)), this, SLOT(serviceRoiChanged(CARoi)));
    //connect(device, SIGNAL(pixelSizeAvailable(QSize)), this, SLOT(servicePixelSizeAvailable(QSize)));
    connect(device, SIGNAL(deviceInitialized(bool)), this, SIGNAL(connection(bool)), Qt::AutoConnection);
}

///
/// \brief CAHamamatsu::openWindow
///
void CAHamamatsu::openWindow()
{
    window->show();
    window->raise();
}
///
/// \brief CAHamamatsu::startCapture
/// \return
///
int CAHamamatsu::startCapture()
{
    emit(startCaptureRequest());

    return TRUE;
}
///
/// \brief CAHamamatsu::stopCapture
/// \return
///
int CAHamamatsu::stopCapture()
{
    emit(stopCaptureRequest());

    return TRUE;
}
///
/// \brief CAHamamatsu::snapImage
/// \return
///
int CAHamamatsu::snapImage()
{
    emit(snapImageRequest());

    return TRUE;
}

///
/// \brief CAHamamatsu::serviceBinningChanged
/// \param val
/// \return
///
void CAHamamatsu::serviceBinningChanged(long val)
{
    binning = val;
    emit(binningChanged(val));
}

///
/// \brief CAHamamatsu::serviceRoiChanged
/// \param roi
///
void CAHamamatsu::serviceRoiChanged(CARoi roi)
{
    regionOfInterest = roi;
    emit(roiChanged(regionOfInterest));
}

///
/// \brief CAHamamatsu::servicePixelSizeAvailable
/// \param size
///
void CAHamamatsu::servicePixelSizeAvailable(QSize size)
{
    pixelSize = size;
    emit(pixelSizeChanged(pixelSize));
}

///
/// \brief CAHamamatsuDevice::hDCAMModule
///
HINSTANCE CAHamamatsuDevice::hDCAMModule = 0;

///
/// \brief CAHamamatsuDevice::CAHamamatsuDevice
///
CAHamamatsuDevice::CAHamamatsuDevice() :
    QObject(),
    hDCAM(0),
    initialized(false),
    isOpen(false),
    acquiring(false),
    connectionTimer(NULL),
    //exposure must be initialized to 0 to avoid unproper func.
    exposure ({0,0,0,0}),
    currentOffset(0)
{
}
///
/// \brief CAHamamatsuDevice::~CAHamamatsuDevice
///
CAHamamatsuDevice::~CAHamamatsuDevice()
{
    if(connectionTimer == NULL){
        connectionTimer->stop();
        delete connectionTimer;
    }
    if (initialized){
        initialized = false;
        //shutdown();
        mainLoopFuture.waitForFinished();
    }
}

///
/// \brief CAHamamatsuDevice::tryOpen
///
void CAHamamatsuDevice::tryOpen()
{
    if(connectionTimer == NULL){
        connectionTimer = new QTimer();
        connectionTimer->setSingleShot(false);
        connectionTimer->start(5000);
        connect(connectionTimer, SIGNAL(timeout()), this, SLOT(tryOpen()), Qt::UniqueConnection);
    }

    if(!initialized){
        if (open()){
            emit(online());
            //initialize
            initialize();
        }
    }
}

///
/// \brief CAHamamatsuDevice::open
/// \return
///
bool CAHamamatsuDevice::open()
{
    if (initialized || isOpen)
        return TRUE;

    // initialize the DCAM dll
    if (hDCAMModule == 0)
        hDCAMModule = ::GetModuleHandle(NULL);

    long lnCameras(0);

    if (dcam_init(hDCAMModule, &lnCameras, NULL) ){
        if (lnCameras > 0) {
            if(dcam_open(&hDCAM, 0, NULL) || hDCAM){
                isOpen = TRUE;
            }
        } else dcam_uninit(hDCAMModule);
    } else {
        dcam_uninit(hDCAMModule);
        // clear the instance pointer
        hDCAMModule = 0;
        isOpen = FALSE;
    }

    return isOpen;
}

///
/// \brief CAHamamatsuDevice::initialize
/// \return
///
int CAHamamatsuDevice::initialize()
{
    // setup the camera
    // ----------------
    if (initialized)
        return TRUE;

    if(isOpen){
        DWORD	cap;
        if(dcam_getcapability(hDCAM, &cap, DCAM_QUERYCAPABILITY_FUNCTIONS)){

            binnings.push_back(1); // Binning 1x1 is always available
            if ( cap & DCAM_CAPABILITY_BINNING2 ) binnings.push_back(2);
            if ( cap & DCAM_CAPABILITY_BINNING4 ) binnings.push_back(4);
            if ( cap & DCAM_CAPABILITY_BINNING6 ) binnings.push_back(6);
            if ( cap & DCAM_CAPABILITY_BINNING8 ) binnings.push_back(8);
            if ( cap & DCAM_CAPABILITY_BINNING12) binnings.push_back(12);
            if ( cap & DCAM_CAPABILITY_BINNING16) binnings.push_back(16);
            if ( cap & DCAM_CAPABILITY_BINNING32) binnings.push_back(32);

            /*
                    // CameraName
                    dcam_getstring(hDCAM, DCAM_IDSTR_MODEL,deviceName, sizeof(deviceName));
                    // CameraID
                    dcam_getstring(hDCAM, DCAM_IDSTR_CAMERAID, deviceID, sizeof(deviceID));

                    // Camera Version
                    dcam_getstring(hDCAM, DCAM_IDSTR_CAMERAVERSION,deviceVersion, sizeof(deviceVersion));

                    // Driver Version
                    dcam_getstring(hDCAM, DCAM_IDSTR_DRIVERVERSION, driverVersion, sizeof(driverVersion));
                    */

            // setup image parameters
            // ----------------------

            // --- BIN already set to 1,1 above.
            // --- Hamamatsu sets the ROI by starting coordinates at 1 and not 0 !!!!
            SIZE	sz;
            if(dcam_getdatasize(hDCAM, &sz ) ){
                imageSize.setWidth(sz.cx);
                imageSize.setHeight(sz.cy);
                regionOfIntest.x = 0;
                regionOfIntest.y = 0;
                regionOfIntest.width = sz.cx;
                regionOfIntest.height = sz.cy;

                imageBuffer.resize(sz.cx, sz.cy);
                emit(roiChanged(regionOfIntest));
            } else return FALSE;

            ccDatatype dataType;
            if (dcam_getdatatype(hDCAM,&dataType)){
                currentDataType = dataType;
            } else return FALSE;

            // pixel data types
            DWORD	typeCap;
            if(dcam_getcapability(hDCAM, &typeCap, DCAM_QUERYCAPABILITY_DATATYPE)){
                if(typeCap & ccDatatype_uint8)
                    pixelsDataTypes.insert("8BITS",ccDatatype_uint8);
                if(typeCap & ccDatatype_uint16)
                    pixelsDataTypes.insert("16BITS",ccDatatype_uint16);
                if(typeCap & ccDatatype_uint32)
                    pixelsDataTypes.insert("32BITS",ccDatatype_uint32);
            } else return FALSE;

            // pixel size
            /*QSize pixelSize = getPixelSize();
            if(pixelSize.isValid()){
                emit(pixelSizeAvailable(pixelSize));
            }*/

            querySensorModes();

            initialized = true;
            emit(sensorModesItems(sensorModes));
            mainLoopFuture = QtConcurrent::run(this, &CAHamamatsuDevice::mainLoop);
        } else return FALSE;
    } else return FALSE;

    return TRUE;
}
///
/// \brief CAHamamatsuDevice::shutdown
/// \return
///
int CAHamamatsuDevice::shutdown()
{
    lock.lock();
    dcam_idle(hDCAM);
    dcam_freeframe(hDCAM);
    dcam_close(hDCAM);
    dcam_uninit(hDCAMModule);
    initialized = false;
    isOpen = false;

    // clear the instance pointer
    hDCAMModule = 0;
    lock.unlock();

    return TRUE;
}
///
/// \brief CAHamamatsuDevice::startCapture
/// \return
///
int CAHamamatsuDevice::startCapture()
{
    lock.lock();
    if (initialized){
        if(configureImageBuffer(ccCapture_Sequence)){
            // Start capture
            dcam_capture(hDCAM) ;
            acquiring = true;
        }
    }
    lock.unlock();

    return acquiring;
}
///
/// \brief CAHamamatsuDevice::stopCapture
/// \return
///
int CAHamamatsuDevice::stopCapture()
{
    lock.lock();
    // Stop image capture
    if (acquiring){
        // Stop capture
        if(dcam_idle(hDCAM)){
            // Release image transfer buffers
            if(dcam_freeframe(hDCAM)){
                acquiring = false;
            }
        }
    }
    lock.unlock();

    return !acquiring;;
}
///
/// \brief CAHamamatsuDevice::snapImage
/// \return
///
int CAHamamatsuDevice::snapImage()
{
    lock.lock();
    if (initialized){
        if(configureImageBuffer(ccCapture_Snap)){
            // Start capture
            dcam_capture(hDCAM) ;
            if (currentTriggerSource == DCAM_TRIGMODE_SOFTWARE) {
                if (!dcam_firetrigger(hDCAM))
                    return dcam_getlasterror(hDCAM, NULL, 0);
            }
        }
    }

    lock.unlock();
    return TRUE;
}
///
/// \brief CAHamamatsu::getImageBuffer
/// \return
///
CAImage *  CAHamamatsuDevice::getImageBuffer()
{
    CAImage * returnPtr = NULL;

    lock.lock();
    if (initialized){
        long lnTimeOut = (long) ((exposure.value + 5.0) * 1000.0);
        DWORD dwEvent = DCAM_EVENT_FRAMEEND;
        if (dcam_wait(hDCAM, &dwEvent, lnTimeOut, NULL)){
            // get pixels
            void* src;
            long sRow;
            if(dcam_lockdata(hDCAM, &src, &sRow, -1)){
                void* pixBuffer = const_cast<unsigned char*> (imageBuffer.data);
                memcpy(pixBuffer, src, imageBuffer.byteCount());
                dcam_unlockdata(hDCAM);
                returnPtr = &imageBuffer;
            }
        }
    }
    lock.unlock();

    return returnPtr;
}

///
/// \brief CAHamamatsuDevice::isAlive
/// \return
///
bool CAHamamatsuDevice::isAlive()
{
    double status = -1;

    dcam_getpropertyvalue(hDCAM, DCAM_IDPROP_SYSTEM_ALIVE, &status);
    return (status == DCAMPROP_SYSTEM_ALIVE__ONLINE);
}

///
/// \brief CAHamamatsuDevice::mainLoop
///
void CAHamamatsuDevice::mainLoop()
{
    emit(deviceInitialized(true));

    while(initialized) {
        CAImage * bufferPtr  = getImageBuffer();
        if(bufferPtr != NULL){
            CAImage im(bufferPtr->rows, bufferPtr->cols, bufferPtr->type());
            bufferPtr->contrastAdjust(im, currentOffset, 65535);
            emit(imageAvailable(im));
        } else if(!isAlive())
            break;
    }
    shutdown();
    emit(deviceInitialized(false));
}

///
/// \brief CAHamamatsuDevice::onExposureChanged
/// \param value
///
void CAHamamatsuDevice::onExposureChanged(double value)
{
    double currentValue = exposure.value;
    if(value != currentValue){
        setExposure(value);
    }
}

///
/// \brief CAHamamatsuDevice::onInternalLineIntervalChanged
/// \param value
///
void CAHamamatsuDevice::onInternalLineIntervalChanged(double value)
{
    double currentValue = internalLineInterval.value;
    if(value != currentValue){
        if(setInternalLineInterval(value)){
            queryExposureLimits();
            emit(exposureLimits(exposure));
        }
    }
}
///
/// \brief CAHamamatsuDevice::onBinningChanged
/// \param value
///
void CAHamamatsuDevice::onBinningChanged(long value)
{
    if(binnings.contains(value)){
        setBinning(value);
    }
}
///
/// \brief CAHamamatsuDevice::onScanModeChanged
/// \param value
///
void CAHamamatsuDevice::onScanModeChanged(long value)
{
    if(scanModes.contains(value)){
        setScanMode(value);
    }
}

///
/// \brief CAHamamatsuDevice::onGlobalExposureChanged
/// \param gexp
///
void CAHamamatsuDevice::onGlobalExposureChanged(QString gexp)
{
    // We need to make sure, we are not in progressive mode
    // If scan mode is set while in progressive mode, the sensor mode
    // is changed to area mode
    if (currentSensorMode != DCAMPROP_SENSORMODE__PROGRESSIVE){
        if(triggerGlobalExposure.contains(gexp)){
            int value = triggerGlobalExposure[gexp];
            setTriggerGlobalExposure(value);
        }
    }
}

///
/// \brief CAHamamatsuDevice::onReadoutDirChanged
/// \param dir
///
void CAHamamatsuDevice::onReadoutDirChanged(QString dir)
{
    if(readoutDirections.contains(dir)){
        int value = readoutDirections[dir];
        setScanMode(value);
    }
}

///
/// \brief CAHamamatsuDevice::onTriggerOutputChanged
/// \param outputNbr
/// \param mode
///
void CAHamamatsuDevice::onTriggerOutputChanged(int outputNbr, QString mode)
{
    if(triggerOutputs.contains(mode)) {
        int value = triggerOutputs[mode];
        if(value != currentTriggerOutputs[outputNbr]){
            setTriggerOutput(outputNbr, value);
        }
    }
}

///
/// \brief CAHamamatsuDevice::onSensorModeChanged
/// \param mode
///
void CAHamamatsuDevice::onSensorModeChanged(QString mode)
{
    if(sensorModes.contains(mode)) {
        int modeInt = sensorModes[mode];
        setSensorMode(modeInt);
    }
}
///
/// \brief CAHamamatsuDevice::onTriggerSourceChanged
/// \param mode
///
void CAHamamatsuDevice::onTriggerSourceChanged(QString source)
{
    if(triggerSources.contains(source)) {
        int sourceInt = triggerSources[source];
        setTriggerSource(sourceInt);
    }
}

void CAHamamatsuDevice::onTriggerTypeChanged(QString type)
{
    if(triggerTypes.contains(type)) {
        int typeInt = triggerTypes[type];
        setTriggerType(typeInt);
    }
}

///
/// \brief CAHamamatsuDevice::onTriggerPolarityChanged
/// \param pol
///
void CAHamamatsuDevice::onTriggerPolarityChanged(QString pol)
{
    if(triggerPolarities.contains(pol)) {
        int polInt = triggerPolarities[pol];
        setTriggerPolarity(polInt);
    }
}

///
/// \brief CAHamamatsuDevice::onROIChanged
/// \param roi
///
void CAHamamatsuDevice::onROIChanged(CARoi roi)
{
    setROI(roi);
}

///
/// \brief CAHamamatsuDevice::setTriggerSource
/// \param source
/// \return
///
int CAHamamatsuDevice::setTriggerSource(long source)
{
    bool isAcq = acquiring;
    if (isAcq)
        stopCapture();

    if (dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_TRIGGERSOURCE, source)) {
        currentTriggerSource = source;
        queryTriggerTypes();
        emit(triggerTypesItems(triggerTypes));
    }

    if (isAcq)
        startCapture();

    return TRUE;
}

///
/// \brief CAHamamatsuDevice::setTriggerType
/// \param type
/// \return
///
int CAHamamatsuDevice::setTriggerType(long type)
{
    bool isAcq = acquiring;
    if (isAcq)
        stopCapture();

    if (dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_TRIGGERACTIVE, type)) {
        currentTriggerType = type;
        queryTriggerPolarities();
        emit(triggerPolaritiesItems(triggerPolarities));
    }

    if (isAcq)
        startCapture();

    return TRUE;
}

///
/// \brief CAHamamatsuDevice::setTriggerPolarity
/// \param pol
/// \return
///
int CAHamamatsuDevice::setTriggerPolarity(long pol)
{
    if (dcam_settriggerpolarity(hDCAM, pol)){
        currentTriggerPolarity = pol;
        dcam_setexposuretime(hDCAM, exposure.value);
    }

    return TRUE;
}

///
/// \brief CAHamamatsuDevice::setExposure
/// \param dExp
/// \return
///
int CAHamamatsuDevice::setExposure(double dExp)
{
    if(dcam_setexposuretime(hDCAM, dExp)){
        exposure.value = dExp;
    }

    return TRUE;
}

///
/// \brief CAHamamatsuDevice::setInternalLineInterval
/// \param dInterval
/// \return
///
int CAHamamatsuDevice::setInternalLineInterval(double dInterval)
{
    int success = FALSE;
    if(dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_INTERNAL_LINEINTERVAL, dInterval)){
        internalLineInterval.value = dInterval;
        success = TRUE;
    }

    return success;
}

///
/// \brief CAHamamatsuDevice::setBinning
/// \param binSize
/// \return
///
int CAHamamatsuDevice::setBinning(long binSize)
{
    bool isAcq = acquiring;
    if (isAcq)
        stopCapture();

    if(dcam_setbinning(hDCAM, binSize)){
        currentBinning = binSize;
        currentOffset = 100 * pow(currentBinning,2);
        getImageSize();
    }

    if (isAcq)
        startCapture();

    return TRUE;
}
///
/// \brief CAHamamatsuDevice::setScanMode
/// \param mode
/// \return
///
int CAHamamatsuDevice::setScanMode(long mode)
{
    // We need to make sure, we are not in progressive mode
    // If scan mode is set while in progressive mode, the sensor mode
    // is chnaged to area mode
    if (currentSensorMode != DCAMPROP_SENSORMODE__PROGRESSIVE){
        bool isAcq = acquiring;
        if (isAcq)
            stopCapture();

        DCAM_PARAM_SCANMODE ScanMode;
        ZeroMemory((LPVOID)&ScanMode, sizeof(DCAM_PARAM_SCANMODE));
        ScanMode.hdr.cbSize = sizeof(DCAM_PARAM_SCANMODE);
        ScanMode.hdr.id = (DWORD) DCAM_IDPARAM_SCANMODE;
        ScanMode.speed = mode;
        if ( dcam_extended(hDCAM,DCAM_IDMSG_SETPARAM, (LPVOID)&ScanMode,
                           sizeof(DCAM_PARAM_SCANMODE)) == TRUE) {
            currentScanMode = mode;
            queryExposureLimits();
            emit(exposureLimits(exposure));
        }

        if (isAcq)
            startCapture();
    }

    return TRUE;
}


///
/// \brief CAHamamatsuDevice::setTriggerOutput
/// \param outputNbr
/// \param modeValue
/// \return
///
int CAHamamatsuDevice::setTriggerOutput(int outputNbr, long modeValue)
{
    outputNbr *= DCAM_IDPROP__OUTPUTTRIGGER;
    int success = dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_OUTPUTTRIGGER_KIND + outputNbr , DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE);
    success += dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + outputNbr , modeValue);
    success += dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + outputNbr , 2.0E-6);
    success += dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_OUTPUTTRIGGER_PREHSYNCCOUNT + outputNbr , 0);
    success += dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + outputNbr, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE);
    if (success == 5) {
        currentTriggerOutputs[outputNbr] = modeValue;
    }

    return TRUE;
}

int CAHamamatsuDevice::enableTriggerOutput(int outputNbr)
{
    return dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_OUTPUTTRIGGER_KIND | outputNbr , DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE);
}

///
/// \brief CAHamamatsuDevice::setTriggerOutputToTrigReay
/// \param outputNbr
/// \return
///
int CAHamamatsuDevice::setTriggerOutputToTrigReay(int outputNbr)
{
    double modeValue = DCAMPROP_OUTPUTTRIGGER_KIND__TRIGGERREADY;
    if (dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_OUTPUTTRIGGER_KIND | outputNbr , modeValue)) {
        currentTriggerOutputs[outputNbr] = -1;
    }

    return TRUE;
}

///
/// \brief CAHamamatsuDevice::setTriggerGlobalExposure
/// \param val
/// \return
///
int CAHamamatsuDevice::setTriggerGlobalExposure(long val)
{
    int success = FALSE;
    if (dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, val)) {
        success = TRUE;
    }

    return success;
}

///
/// \brief CAHamamatsuDevice::setSensorMode
/// \param modeValue
/// \return
///
int CAHamamatsuDevice::setSensorMode(long modeValue)
{
    bool isAcq = acquiring;
    if (isAcq)
        stopCapture();

    if (dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_SENSORMODE, modeValue)) {
        currentSensorMode = modeValue;
        queryReadoutDirection();
        queryScanModes();
        queryTriggerSources();
        queryTriggerOutputModes();
        queryExposureLimits();
        queryTriggerGlobalExposure();
        queryInternalLineInterval();
        emit(exposureLimits(exposure));
        emit(internalLineIntervalBounds(internalLineInterval));
        emit(globalExposureItems(triggerGlobalExposure));
        emit(deviceInitialized(readoutDirections, binnings,scanModes, triggerSources,triggerOutputs));
    }

    if (isAcq)
        startCapture();

    return TRUE;
}

long CAHamamatsuDevice::getSensorMode()
{
    double modeValue = -1;

    if (dcam_getpropertyvalue(hDCAM, DCAM_IDPROP_SENSORMODE, &modeValue)) {
        long modeValueInt = (long) modeValue;
        qDebug("%ld %ld", modeValueInt, currentSensorMode);
    }

    return (long) modeValue;
}

///
/// \brief CAHamamatsuDevice::setReadoutDirection
/// \param dir
/// \return
///
int CAHamamatsuDevice::setReadoutDirection(long dir)
{
    bool isAcq = acquiring;
    if (isAcq)
        stopCapture();

    if (dcam_setpropertyvalue(hDCAM, DCAM_IDPROP_READOUT_DIRECTION, dir)) {
        currentReadoutDir = dir;
        if (isAcq)
            startCapture();
    }

    return TRUE;
}

///
/// \brief CAHamamatsuDevice::setROI
/// \param roi
/// \return
///
int CAHamamatsuDevice::setROI(CARoi roi)
{
    bool isAcq = acquiring;
    if (isAcq)
        stopCapture();

    // inquire about capabilities
    DCAM_PARAM_SUBARRAY_INQ subArrayInquiry;
    ZeroMemory((LPVOID)&subArrayInquiry,sizeof(DCAM_PARAM_SUBARRAY_INQ));
    subArrayInquiry.hdr.cbSize = sizeof(DCAM_PARAM_SUBARRAY_INQ);
    subArrayInquiry.hdr.id = (DWORD)DCAM_IDPARAM_SUBARRAY_INQ;
    subArrayInquiry.binning= currentBinning;
    if (dcam_extended(hDCAM,DCAM_IDMSG_GETPARAM,(LPVOID)&subArrayInquiry,sizeof(DCAM_PARAM_SUBARRAY_INQ))){

        DCAM_PARAM_SUBARRAY subArrayValue;
        ZeroMemory((LPVOID)&subArrayValue, sizeof(DCAM_PARAM_SUBARRAY));
        subArrayValue.hdr.cbSize = sizeof(DCAM_PARAM_SUBARRAY);
        subArrayValue.hdr.id = (DWORD)DCAM_IDPARAM_SUBARRAY;

        unsigned newX = roi.x - (roi.x % subArrayInquiry.hposunit);
        unsigned newY = roi.y - (roi.y % subArrayInquiry.vposunit);
        unsigned newXSize = roi.width - (roi.width % subArrayInquiry.hunit);
        unsigned newYSize = roi.height - (roi.height % subArrayInquiry.vunit);

        // set new SubArray settings
        subArrayValue.hpos = newX;
        subArrayValue.hsize = newXSize;
        subArrayValue.vpos = newY;
        subArrayValue.vsize = newYSize;

        if (dcam_extended(hDCAM, DCAM_IDMSG_SETPARAM, (LPVOID)&subArrayValue, sizeof(DCAM_PARAM_SUBARRAY))){
            memcpy(&regionOfIntest,&roi,sizeof(roi));
            regionOfIntest.x = newX;
            regionOfIntest.y = newY;
            regionOfIntest.width = newXSize;
            regionOfIntest.height = newYSize;
            getImageSize();
            emit(roiChanged(regionOfIntest));
        }
    }

    if (isAcq)
        startCapture();

    return TRUE;
}

void CAHamamatsuDevice::getImageSize()
{
    SIZE	sz;
    if(dcam_getdatasize(hDCAM, &sz ) ){
        imageSize.setWidth(sz.cx);
        imageSize.setHeight(sz.cy);

        imageBuffer.resize(sz.cx, sz.cy);
    }
}

QSize CAHamamatsuDevice::getPixelSize()
{
    double pixelWidth, pixelHeight;
    QSize pxSize = QSize();
    bool wsuccess = dcam_getpropertyvalue(hDCAM,
                                          DCAM_IDPROP_IMAGEDETECTOR_PIXELWIDTH,
                                          &pixelWidth);
    bool hsuccess = dcam_getpropertyvalue(hDCAM,
                                          DCAM_IDPROP_IMAGEDETECTOR_PIXELHEIGHT,
                                          &pixelHeight);
    if (wsuccess && hsuccess){
        pxSize = QSize((int) pixelWidth * 1000,
                       (int) pixelHeight * 1000);
    }

    return pxSize;
}

///
/// \brief CAHamamatsuDevice::getFeatureInq
/// \param hdcam
/// \param idfeature
/// \param capflags
/// \param min
/// \param max
/// \param step
/// \param defaultvalue
/// \return
///
bool CAHamamatsuDevice::getFeatureInq( HDCAM hdcam,				///< [in]  camera handle
                                       long idfeature,			///< [in]  feature id
                                       long& capflags,			///< [out] ?
                                       double& min,				///< [out] min value of the feature
                                       double& max,				///< [out] max value of the feature
                                       double& step,			///< [out] ?
                                       double& defaultvalue )	///< [out] default value of the feature
const
{
    DCAM_PARAM_FEATURE_INQ	inq;
    memset( &inq, 0, sizeof( inq ) );
    inq.hdr.cbSize	= sizeof( inq );
    inq.hdr.id		= DCAM_IDPARAM_FEATURE_INQ;
    inq.hdr.iFlag	= dcamparam_featureinq_featureid
            | dcamparam_featureinq_capflags
            | dcamparam_featureinq_min
            | dcamparam_featureinq_max
            | dcamparam_featureinq_step
            | dcamparam_featureinq_defaultvalue
            | dcamparam_featureinq_units
            ;
    inq.featureid	= idfeature;

    if( !dcam_extended( hdcam, DCAM_IDMSG_GETPARAM, &inq, sizeof( inq ) ) )
    {
        return false;
    }

    if( inq.hdr.oFlag & dcamparam_featureinq_capflags )		capflags	 = inq.capflags;
    if( inq.hdr.oFlag & dcamparam_featureinq_min )			min			 = inq.min;
    if( inq.hdr.oFlag & dcamparam_featureinq_max )			max			 = inq.max;
    if( inq.hdr.oFlag & dcamparam_featureinq_step )			step		 = inq.step;
    if( inq.hdr.oFlag & dcamparam_featureinq_defaultvalue )	defaultvalue = inq.defaultvalue;

    return true;
}
///
/// \brief CAHamamatsuDevice::configureImageBuffer
/// \param captureMode
/// \return
///
int CAHamamatsuDevice::configureImageBuffer(ccCaptureMode captureMode)
{
    // format the image buffer
    if (currentDataType == ccDatatype_uint8 || currentDataType == ccDatatype_int8){
        //imageBuffer.resize(imageSize.width(),imageSize.height(), 1);
        imageBuffer = CAImage(imageSize.height(), imageSize.width(), 1, 1);
    }
    else if (currentDataType == ccDatatype_uint16 || currentDataType == ccDatatype_int16){
        //imageBuffer.resize(imageSize.width(),imageSize.height(), 2);
        imageBuffer = CAImage(imageSize.height(), imageSize.width(), 2, 1);
    }
    else{
        return DEVICE_UNSUPPORTED_DATA_FORMAT;
    }

    // setup the capture mode
    if (dcam_precapture(hDCAM, captureMode)){
        // set a 4 frame sequence buffercapt
        //const long frameBufSize = 3;
        if (dcam_freeframe(hDCAM)){
            if (dcam_allocframe(hDCAM, 4)) {
                long numFrames;
                if (dcam_getframecount(hDCAM, &numFrames)){

                    if (numFrames != 4)
                        return FALSE;
                }
            }

            DWORD dwDataBufferSize;
            if (dcam_getdataframebytes(hDCAM, &dwDataBufferSize)){
                if (imageBuffer.byteCount() != dwDataBufferSize)
                    return DEVICE_INTERNAL_INCONSISTENCY; // buffer sizes don't match ???
            }
        }
    }

    return TRUE;
}

///
/// \brief CAHamamatsuDevice::queryTriggerOutputModes
///
void CAHamamatsuDevice::queryTriggerOutputModes()
{
    DCAM_PROPERTYATTR triggerOutputProp;
    memset(&triggerOutputProp, 0, sizeof(triggerOutputProp));
    triggerOutputProp.cbSize = sizeof(triggerOutputProp);
    triggerOutputProp.iProp = DCAM_IDPROP_OUTPUTTRIGGER_SOURCE;
    triggerOutputs.clear();

    if(dcam_getpropertyattr(hDCAM,&triggerOutputProp )){

        // low number of values, make a list
        if ( (triggerOutputProp.valuemax >= triggerOutputProp.valuemin)
             && (triggerOutputProp.valuestep == 1.0 || triggerOutputProp.valuestep==0.0) ) {

            DCAM_PROPERTYVALUETEXT prop;
            char propName[64];

            for (long i = (long) triggerOutputProp.valuemin; i<= (long) triggerOutputProp.valuemax; i++) {
                memset(&prop, 0, sizeof(prop));
                prop.cbSize = sizeof(prop);
                prop.iProp = triggerOutputProp.iProp;
                prop.value = i;
                prop.text = propName;
                prop.textbytes = 64;
                if( dcam_getpropertyvaluetext(hDCAM,&prop))
                    triggerOutputs.insert(QString(propName), i);                                }
        }
    }
}

///
/// \brief CAHamamatsuDevice::queryScanModes
///
void CAHamamatsuDevice::queryScanModes()
{
    scanModes.clear();
    // seems that DCAMAPI does not have a way to know when readoutspeed
    // change is supported
    if (currentSensorMode != DCAMPROP_SENSORMODE__PROGRESSIVE){
        DCAM_PROPERTYATTR scanModeProp;
        memset(&scanModeProp, 0, sizeof(scanModeProp));
        scanModeProp.cbSize = sizeof(scanModeProp);
        scanModeProp.iProp = DCAM_IDPROP_READOUTSPEED;

        if(dcam_getpropertyattr(hDCAM,&scanModeProp )){

            // low number of values, make a list
            if ( (scanModeProp.valuemax >= scanModeProp.valuemin)
                 && (scanModeProp.valuestep == 1.0 || scanModeProp.valuestep==0.0) ) {

                for (long i = (long) scanModeProp.valuemin; i<= (long) scanModeProp.valuemax; i++) {
                    scanModes.push_back(i);
                }
            }
        }
    }
}

///
/// \brief CAHamamatsuDevice::querySensorModes
///
void CAHamamatsuDevice::querySensorModes()
{
    DCAM_PROPERTYATTR sensorModeProp;
    memset(&sensorModeProp, 0, sizeof(sensorModeProp));
    sensorModeProp.cbSize = sizeof(sensorModeProp);
    sensorModeProp.iProp = DCAM_IDPROP_SENSORMODE;
    sensorModes.clear();

    if(dcam_getpropertyattr(hDCAM,&sensorModeProp )){

        // low number of values, make a list
        if ( (sensorModeProp.valuemax >= sensorModeProp.valuemin)
             && (sensorModeProp.valuestep == 1.0 || sensorModeProp.valuestep==0.0) ) {

            DCAM_PROPERTYVALUETEXT prop;
            char propName[64];

            for (long i = (long) sensorModeProp.valuemin; i<= (long) sensorModeProp.valuemax; i++) {
                memset(&prop, 0, sizeof(prop));
                prop.cbSize = sizeof(prop);
                prop.iProp = DCAM_IDPROP_SENSORMODE;
                prop.value = i;
                prop.text = propName;
                prop.textbytes = 64;
                if( dcam_getpropertyvaluetext(hDCAM,&prop))
                    sensorModes.insert(QString(propName), i);
            }
        }
    }
}

///
/// \brief CAHamamatsuDevice::queryReadoutDirection
///
void CAHamamatsuDevice::queryReadoutDirection()
{
    DCAM_PROPERTYATTR readoutDirProp;
    memset(&readoutDirProp, 0, sizeof(readoutDirProp));
    readoutDirProp.cbSize = sizeof(readoutDirProp);
    readoutDirProp.iProp = DCAM_IDPROP_READOUT_DIRECTION;
    readoutDirections.clear();

    if(dcam_getpropertyattr(hDCAM,&readoutDirProp )){
        // low number of values, make a list
        if ( (readoutDirProp.valuemax >= readoutDirProp.valuemin)
             && (readoutDirProp.valuestep == 1.0 || readoutDirProp.valuestep==0.0) ) {

            DCAM_PROPERTYVALUETEXT prop;
            char propName[64];

            for (long i = (long) readoutDirProp.valuemin; i<= (long) readoutDirProp.valuemax; i++) {
                memset(&prop, 0, sizeof(prop));
                prop.cbSize = sizeof(prop);
                prop.iProp = DCAM_IDPROP_READOUT_DIRECTION;
                prop.value = i;
                prop.text = propName;
                prop.textbytes = 64;
                if( dcam_getpropertyvaluetext(hDCAM,&prop)){
                    readoutDirections.insert(QString(propName), i);
                }
            }
        }
    }
}

///
/// \brief CAHamamatsuDevice::queryTriggerSources
///
void CAHamamatsuDevice::queryTriggerSources()
{
    DCAM_PROPERTYATTR triggerSourceProp;
    memset(&triggerSourceProp, 0, sizeof(triggerSourceProp));
    triggerSourceProp.cbSize = sizeof(triggerSourceProp);
    triggerSourceProp.iProp = DCAM_IDPROP_TRIGGERSOURCE;
    triggerSources.clear();

    if(dcam_getpropertyattr(hDCAM,&triggerSourceProp )){

        // low number of values, make a list
        if ( (triggerSourceProp.valuemax >= triggerSourceProp.valuemin)
             && (triggerSourceProp.valuestep == 1.0 || triggerSourceProp.valuestep==0.0) ) {

            DCAM_PROPERTYVALUETEXT prop;
            char propName[64];

            for (long i = (long) triggerSourceProp.valuemin; i<= (long) triggerSourceProp.valuemax; i++) {
                memset(&prop, 0, sizeof(prop));
                prop.cbSize = sizeof(prop);
                prop.iProp = DCAM_IDPROP_TRIGGERSOURCE;
                prop.value = i;
                prop.text = propName;
                prop.textbytes = 64;
                if( dcam_getpropertyvaluetext(hDCAM,&prop)){
                    triggerSources.insert(QString(propName), i);
                }
            }
        }
    }
}

///
/// \brief CAHamamatsuDevice::queryTriggerTypes
///
void CAHamamatsuDevice::queryTriggerTypes()
{
    DCAM_PROPERTYATTR triggerTypeProp;
    memset(&triggerTypeProp, 0, sizeof(triggerTypeProp));
    triggerTypeProp.cbSize = sizeof(triggerTypeProp);
    triggerTypeProp.iProp = DCAM_IDPROP_TRIGGERACTIVE;
    triggerTypes.clear();

    if(dcam_getpropertyattr(hDCAM,&triggerTypeProp )){

        // low number of values, make a list
        if ( (triggerTypeProp.valuemax >= triggerTypeProp.valuemin)
             && (triggerTypeProp.valuestep == 1.0 || triggerTypeProp.valuestep==0.0) ) {

            DCAM_PROPERTYVALUETEXT prop;
            char propName[64];

            for (long i = (long) triggerTypeProp.valuemin; i<= (long) triggerTypeProp.valuemax; i++) {
                memset(&prop, 0, sizeof(prop));
                prop.cbSize = sizeof(prop);
                prop.iProp = DCAM_IDPROP_TRIGGERACTIVE;
                prop.value = i;
                prop.text = propName;
                prop.textbytes = 64;
                if( dcam_getpropertyvaluetext(hDCAM,&prop)){
                    triggerTypes.insert(QString(propName), i);
                }
            }
        }
    }
}

///
/// \brief CAHamamatsuDevice::queryTriggerPolarities
///
void CAHamamatsuDevice::queryTriggerPolarities()
{
    DCAM_PROPERTYATTR triggerPolarityProp;
    memset(&triggerPolarityProp, 0, sizeof(triggerPolarityProp));
    triggerPolarityProp.cbSize = sizeof(triggerPolarityProp);
    triggerPolarityProp.iProp = DCAM_IDPROP_TRIGGERPOLARITY;
    triggerPolarities.clear();

    if(dcam_getpropertyattr(hDCAM,&triggerPolarityProp )){

        // low number of values, make a list
        if ( (triggerPolarityProp.valuemax >= triggerPolarityProp.valuemin)
             && (triggerPolarityProp.valuestep == 1.0 || triggerPolarityProp.valuestep==0.0) ) {

            DCAM_PROPERTYVALUETEXT prop;
            char propName[64];

            for (long i = (long) triggerPolarityProp.valuemin; i<= (long) triggerPolarityProp.valuemax; i++) {
                memset(&prop, 0, sizeof(prop));
                prop.cbSize = sizeof(prop);
                prop.iProp = DCAM_IDPROP_TRIGGERPOLARITY;
                prop.value = i;
                prop.text = propName;
                prop.textbytes = 64;
                if( dcam_getpropertyvaluetext(hDCAM,&prop)){
                    triggerPolarities.insert(QString(propName), i);
                }
            }
        }
    }
}

///
/// \brief CAHamamatsuDevice::queryExposureLimits
/// \return
///
bool CAHamamatsuDevice::queryExposureLimits()
{
    // Retreive exposure time
    long	capflags;
    double	step, defaultvalue, expMin, expMax;
    if( getFeatureInq( hDCAM, DCAM_IDFEATURE_EXPOSURETIME,
                       capflags, expMin, expMax, step, defaultvalue ) ){
        exposure.min = expMin;
        exposure.max = expMax;
        exposure.step = step;
        if (exposure.value < expMin || exposure.value > expMax)
            exposure.value = defaultvalue;
        return TRUE;
    } else return FALSE;
}

///
/// \brief CAHamamatsuDevice::queryTriggerGlobalExposure
///
void CAHamamatsuDevice::queryTriggerGlobalExposure()
{
    triggerGlobalExposure.clear();

    if (currentSensorMode != DCAMPROP_SENSORMODE__PROGRESSIVE){
        DCAM_PROPERTYATTR globalExpposureProp;
        memset(&globalExpposureProp, 0, sizeof(globalExpposureProp));
        globalExpposureProp.cbSize = sizeof(globalExpposureProp);
        globalExpposureProp.iProp = DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE;

        if(dcam_getpropertyattr(hDCAM,&globalExpposureProp )){

            // low number of values, make a list
            if ( (globalExpposureProp.valuemax >= globalExpposureProp.valuemin)
                 && (globalExpposureProp.valuestep == 1.0 || globalExpposureProp.valuestep==0.0) ) {

                DCAM_PROPERTYVALUETEXT prop;
                char propName[64];

                for (long i = (long) globalExpposureProp.valuemin; i<= (long) globalExpposureProp.valuemax; i++) {
                    memset(&prop, 0, sizeof(prop));
                    prop.cbSize = sizeof(prop);
                    prop.iProp = DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE;
                    prop.value = i;
                    prop.text = propName;
                    prop.textbytes = 64;
                    if( dcam_getpropertyvaluetext(hDCAM,&prop)){
                        triggerGlobalExposure.insert(QString(propName), i);
                    }
                }
            }
        }
    }
}

///
/// \brief CAHamamatsuDevice::queryInternalLineInterval
///
void CAHamamatsuDevice::queryInternalLineInterval()
{
    DCAM_PROPERTYATTR internalLineIntervalProp;
    memset(&internalLineIntervalProp, 0, sizeof(internalLineIntervalProp));
    internalLineIntervalProp.cbSize = sizeof(internalLineIntervalProp);
    internalLineIntervalProp.iProp = DCAM_IDPROP_INTERNAL_LINEINTERVAL;

    if(dcam_getpropertyattr(hDCAM,&internalLineIntervalProp )){

        // low number of values, make a list
        if ( internalLineIntervalProp.valuemax >= internalLineIntervalProp.valuemin ) {
            internalLineInterval.min = internalLineIntervalProp.valuemin;
            internalLineInterval.max = internalLineIntervalProp.valuemax;
            internalLineInterval.step = internalLineIntervalProp.valuestep;
            //internalLineInterval.value = internalLineIntervalProp.valuedefault;
        }
    }
}
