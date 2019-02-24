#include "cacamera.h"

///
/// \brief CACamera::CACamera
/// \param parent
///
CACamera::CACamera(QObject *parent) :
    QThread(parent),
    binning(1),
    regionOfInterest(CARoi()),
    pixelSize(QSize()),
    acquiring(false),
    initialized(false)
{
    qRegisterMetaType<CAImage >("CAImage");
}

///
/// \brief CACamera::~CACamera
///
CACamera::~CACamera()
{
    // release resources
}

///
/// \brief CACamera::openWindow
///
void CACamera::openWindow()
{
}

///
/// \brief CACamera::startCapture
/// \return
///
int CACamera::startCapture()
{
    return 0;
}

///
/// \brief CACamera::stopCapture
/// \return
///
int CACamera::stopCapture()
{
    return 0;
}

///
/// \brief CACamera::snapImage
/// \return
///
int CACamera::snapImage()
{
   return 0;
}

