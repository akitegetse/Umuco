#include "castage.h"

///
/// \brief CAPosition::CAPosition
/// \param x
/// \param y
/// \param z
///
CAPosition::CAPosition(double x, double y , double z)
    : x(x), y(y), z(z)
{
}

///
/// \brief CAStageDevice::CAStageDevice
/// \param parent
///
CAStageDevice::CAStageDevice(QObject *parent) :
    QObject(parent)
{
}

///
/// \brief CAStageDevice::~CAStageDevice
///
CAStageDevice:: ~CAStageDevice()
{
}

void CAStageDevice::doMoveToPosition(double x, double y, double z)
{
    emit(requestedMoveDone());
    emit(positionUpdated(x,y,z));
}

///
/// \brief CAStage::CAStage
/// \param parent
/// \param theDevice
///
CAStage::CAStage(QObject * parent, CAStageDevice * theDevice) :
    QThread(parent),
    isAlive(false),
    device(theDevice),
    position(new CAPosition(0,0,0)),
    window(new CAStageWindow)
{
    connect(this, SIGNAL(doMoveTo(double,double,double)),
            device, SLOT(doMoveToPosition(double,double,double)));
    connect(device, SIGNAL(positionUpdated(double,double,double)),
            this, SLOT(positionChanged(double,double,double)));
    connect(device,SIGNAL(positionUpdated(double, double, double)),
            window, SLOT(positionUpdated(double, double, double)));
    connect(device, SIGNAL(connectionState(bool)),
            this, SLOT(deviceStateChanged(bool)));
    connect(device, SIGNAL(requestedMoveDone()), this, SIGNAL(requestedMoveDone()));
    connect(device, SIGNAL(didnotMove()), this, SIGNAL(didnotMove()));
    device->moveToThread(this);
    start();
}

///
/// \brief CAStage::~CAStage
///
CAStage:: ~CAStage()
{
    delete position;
    delete window;
    exit(0);
    while(!isFinished());
}

///
/// \brief CAStage::openWindow
///
void CAStage::openWindow()
{
    window->show();
}

///
/// \brief CAStage::deviceStateChanged
/// \param state
///
void CAStage::deviceStateChanged(bool state)
{
    isAlive = state;
    emit(connectionState(state));
}

///
/// \brief CAStage::positionChanged
/// \param x
/// \param y
/// \param z
///
void CAStage::positionChanged(double x, double y, double z)
{
    position->x = x;
    position->y = y;
    position->z = z;
    emit(positionUpdated(x,y,z));
}

///
/// \brief CAStage::moveToPosition
/// \param x
/// \param y
/// \param z
///
void CAStage::moveToPosition(double x, double y, double z)
{
    position->x = x;
    position->y = y;
    position->z = z;

    emit(doMoveTo(position->x,
                  position->y,
                  position->z));
    emit(positionUpdated(position->x,
                         position->y,
                         position->z));
}

///
/// \brief CAStage::moveByDistance
/// \param x
/// \param y
/// \param z
///
void CAStage::moveByDistance(double x, double y, double z)
{
    position->x = position->x + x;
    position->y = position->y + y;
    position->z = position->z + z;

    emit(doMoveTo(position->x,
                  position->y,
                  position->z));
    emit(positionUpdated(position->x,
                         position->y,
                         position->z));
}

///
/// \brief CAStage::readPosition
/// \param xPtr
/// \param yPtr
/// \param zPtr
///
void CAStage::readPosition(double *xPtr, double *yPtr, double *zPtr)
{
    *xPtr = position->x;
    *yPtr = position->y;
    *zPtr = position->z;
}
