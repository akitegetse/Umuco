#ifndef CADEVICEMANAGER_H
#define CADEVICEMANAGER_H

#include <QObject>
//add all stages devices here
#include "castage.h"
#include "casuttermp285roe200.h"
//add all cameras here
#include "cacamera.h"
#ifndef __APPLE__
#include "cahamamatsu.h"
#include "NIDAQmx.h"
#endif
#include "caimagesimulator.h"


template<typename T> CAStage * createStage() { return new CAStage(0, new T); }
typedef QMap<QString, CAStage *(*)()> StageMapType;

template<typename T> CACamera * createCamera() { return new T; }
typedef QMap<QString, CACamera *(*)()> CameraMapType;

class CADeviceManager : public QObject
{
    Q_OBJECT
public:
    //properties
    static StageMapType supportedStages;
    static CameraMapType supportedCameras;
    static QList<QString> installedNIDaqDevices;
    //methods
    explicit CADeviceManager(QObject *parent = 0);

private:
    static StageMapType supportedStagesInit();
    static CameraMapType supportedCamerasInit();
    static QList<QString> installedNIDaqDevicesInit();
signals:

public slots:
};

#endif // CADEVICEMANAGER_H
