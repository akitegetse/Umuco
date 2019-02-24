#ifndef _CAHAMAMATSU_H_
#define _CAHAMAMATSU_H_

#include <QLibrary>

#if !__APPLE__
#include <windows.h>
#else
#include <memory.h>
void ZeroMemory(void* mem, int size)
{
    memset(mem,0,size);
}
#endif

#include "cacamera.h"
#include "cahamamatsuwindow.h"
#include "cautilities.h"
#include "dcamapi.h"
#include "dcamapi3.h"
#include "dcamprop.h"

#include <QString>
#include <QMap>
#include <QFuture>
#include <QMutex>

class CAHamamatsuDevice; //forward declaration

///
/// \brief The CAHamamatsu class
///
class CAHamamatsu : public CACamera
{
    Q_OBJECT
public:
    CAHamamatsu();
    ~CAHamamatsu();

protected:
   //void run();

   CAHamamatsuWindow * window;
   CAHamamatsuDevice * device;

signals:
   void deviceOpen();
   void startCaptureRequest();
   void stopCaptureRequest();
   void snapImageRequest();
   void binningChanged(long);
   void roiChanged(CARoi);

public slots:
   //this slots will not be executed in this new thread
   // They are executed in the thread "creator"
   virtual void openWindow();
   virtual int startCapture();
   virtual int stopCapture();
   virtual int snapImage();
   void deviceEstablishLink();

private slots:
   void serviceBinningChanged(long binning);
   void serviceRoiChanged(CARoi roi);
   void servicePixelSizeAvailable(QSize size);
};

///
/// \brief The CAHamamatsuDevice class
///
class CAHamamatsuDevice : public QObject
{
    Q_OBJECT
public:
   CAHamamatsuDevice();
   ~CAHamamatsuDevice();
   bool open();
   int shutdown();
   bool initialized;

private:
   HDCAM hDCAM;
   static HINSTANCE hDCAMModule; // DCAM dll handle
   bool isOpen;
   bool acquiring;
   QMutex lock;
   QTimer * connectionTimer;

   int configureImageBuffer(ccCaptureMode captureMode);
   CAImage * getImageBuffer();
   void mainLoop();
   QFuture<void> mainLoopFuture;

   void querySensorModes();
   void queryReadoutDirection();
   void queryTriggerOutputModes();
   bool queryExposureLimits();
   void queryScanModes();
   void queryTriggerSources();
   void queryTriggerTypes();
   void queryTriggerPolarities();
   void queryTriggerGlobalExposure();
   void queryInternalLineInterval();

protected:

   //properties validity
   QVector<int> binnings;
   QVector<int> scanModes;
   QMap<QString, int> pixelsDataTypes;
   QMap<QString, int> sensorModes;
   QMap<QString, int> triggerSources;
   QMap<QString, int> triggerTypes;
   QMap<QString, int> triggerPolarities;
   QMap<QString, int> readoutDirections;
   QMap<QString, int> triggerGlobalExposure;

   QMap<QString, int> triggerOutputs;

   QSize imageSize;
   CAImage imageBuffer;

   //settable properties
   CARange<double> exposure;
   CARange<double> internalLineInterval;
   long currentBinning;
   long currentScanMode;
   long currentSensorMode;
   long currentReadoutDir;
   long currentTriggerSource;
   long currentTriggerType;
   long currentTriggerPolarity;
   ccDatatype currentDataType;
   long currentTriggerOutputs[3];
   unsigned int currentOffset;

   CARoi regionOfIntest;
   long currentFrameCount;
   long bufLastImage;
   long bufSize;

   bool isAlive();

   //set methods
   int setTriggerSource(long source);
   int setTriggerType(long type);
   int setTriggerPolarity(long pol);
   int setExposure(double dExp);
   int setInternalLineInterval(double dInterval);
   int setBinning(long binSize);
   int setScanMode(long mode);
   int setTriggerGlobalExposure(long val);
   int setTriggerOutput(int outputNbr, long modeValue);
   int setTriggerOutputToTrigReay(int outputNbr);
   int setSensorMode(long mode);
   int setReadoutDirection(long dir);
   int setROI(CARoi roi);
   int enableTriggerOutput(int outputNbr);

   //get methods
   long getSensorMode();
   void getImageSize();
   QSize getPixelSize();
   bool getFeatureInq( HDCAM hdcam,				///< [in]  camera handle
                       long idfeature,			///< [in]  feature id
                       long& capflags,			///< [out] ?
                       double& min,				///< [out] min value of the feature
                       double& max,				///< [out] max value of the feature
                       double& step,			///< [out] ?
                       double& defaultvalue )	///< [out] default value of the feature
   const;

signals:
   void online();
   void deviceInitialized(QMap<QString,int> dirs,QVector<int> bin, QVector<int> scan,
                          QMap< QString, int> trigSources,QMap<QString, int> tOutputs);
   void deviceInitialized(bool status);
   void imageAvailable(CAImage buf);
   void roiChanged(CARoi roi);
   void pixelSizeAvailable(QSize);
   void exposureLimits(CARange<double>);
   void internalLineIntervalBounds(CARange<double>);
   void globalExposureItems(QMap<QString, int> gexp);
   void sensorModesItems(QMap<QString, int> sMode);
   void triggerTypesItems(QMap<QString, int> types);
   void triggerPolaritiesItems(QMap<QString, int> pol);

public slots:
   void tryOpen();
   int initialize();
   int startCapture();
   int stopCapture();
   int snapImage();

   void onExposureChanged(double value);
   void onInternalLineIntervalChanged(double value);
   void onBinningChanged(long value);
   void onScanModeChanged(long value);
   void onGlobalExposureChanged(QString gexp);
   void onReadoutDirChanged(QString dir);
   void onTriggerOutputChanged(int outputNbr, QString mode);
   void onSensorModeChanged(QString mode);
   void onTriggerSourceChanged(QString source);
   void onTriggerTypeChanged(QString type);
   void onTriggerPolarityChanged(QString pol);
   void onROIChanged(CARoi roi);
};




#endif //_HAMAMATSU_H_
