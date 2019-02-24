#include "cadevicemanager.h"

StageMapType  CADeviceManager::supportedStages = CADeviceManager::supportedStagesInit();
CameraMapType  CADeviceManager::supportedCameras = CADeviceManager::supportedCamerasInit();
QList<QString> CADeviceManager::installedNIDaqDevices = CADeviceManager::installedNIDaqDevicesInit();

///
/// \brief CADeviceManager::installedNIDaqDevicesInit
/// \return
///
QList<QString> CADeviceManager::installedNIDaqDevicesInit()
{
    QList<QString> theList;
    char devicesNames[1024];
    //Get comma-delimited list of the devices installed in the system
#ifndef __APPLE__
    DAQmxGetSysDevNames((char*)devicesNames, (uInt32) 1024);
#endif
    char *token;

    /* get the first token */
    token = strtok(devicesNames, ",");

    /* walk through other tokens */
    while( token != NULL )
    {
        theList.append(token);
        token = strtok(NULL, ",");
    }
     return theList;
}

///
/// \brief CADeviceManager::supportedStagesInit
/// \return
///
StageMapType CADeviceManager::supportedStagesInit(){
    StageMapType theMap;
    theMap["SUTTER ROE200"] = &createStage<CASutterMP285ROE200>;
    return theMap;
}

///
/// \brief CADeviceManager::supportedCamerasInit
/// \return
///
CameraMapType CADeviceManager::supportedCamerasInit(){
    CameraMapType theMap;
#ifndef __APPLE__
    theMap["ORCA FLASH 4.0 V2"] = &createCamera<CAHamamatsu>;
#endif
    theMap["SIMULATOR"] = &createCamera<CAImageSimulator>;

    return theMap;
}

///
/// \brief CADeviceManager::CADeviceManager
/// \param parent
///
CADeviceManager::CADeviceManager(QObject *parent) : QObject(parent)
{

}

