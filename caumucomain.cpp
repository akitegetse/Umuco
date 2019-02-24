#include "caumucomain.h"
#include "cadevicemanager.h"

///
/// \brief CAUmucoMain::CAUmucoMain
/// \param parent
///
CAUmucoMain::CAUmucoMain(QApplication *parent) :
    QObject(parent),
    window(new CAUmucoMainWindow),
    daq(new CADaq())
{
    stage = NULL;
    camera = NULL;
    experiment = NULL;
    alignTool = NULL;

    connect(window, SIGNAL(closeRequest()), parent, SLOT(quit()));

    //devices initialization
    connect(window, SIGNAL(stageSelectionChanged(int)), this, SLOT(stageSelectionChanged(int)));
    connect(window, SIGNAL(cameraSelectionChanged(int)), this, SLOT(cameraSelectionChanged(int)));
    connect(window, SIGNAL(daqDeviceNameChanged(QString)), daq, SLOT(deviceNameChanged(QString)));
    connect(window, SIGNAL(createExperiment()), this, SLOT(createExperiment()));
    connect(this, SIGNAL(experimentCreated()), window, SLOT(experimentCreated()));
    connect(window, SIGNAL(daqDisplayRequest()), daq, SLOT(openWindow()));
    connect(window, SIGNAL(acqScanSynchronize(bool)), daq, SLOT(synchronizeCameraWithGalvo(bool)));

    //This must be the last line to make sure UI is updated when all signals and slots are connected
    window->uiUpdate();
}

///
/// \brief CAUmucoMain::~CAUmucoMain
///
CAUmucoMain::~CAUmucoMain()
{
    delete window;
    delete daq;

    if(stage != NULL)
        delete stage;
    if(camera != NULL)
        delete camera;
    if(alignTool != NULL)
        delete alignTool;
}

///
/// \brief CAUmucoMain::openWindow
///
void CAUmucoMain::openWindow()
{
    window->show();
}

///
/// \brief CAUmucoMain::createExperiment
///
void CAUmucoMain::createExperiment()
{
    if(experiment == NULL && camera != NULL && stage != NULL){
        experiment = new CAExperiment(0,stage, camera);
        connect(window, SIGNAL(startExperiment()), experiment, SLOT(start()));
        connect(window, SIGNAL(stopExperiment()), experiment, SLOT(stop()));
        connect(window, SIGNAL(createExperiment()), experiment, SLOT(openWindow()));
        connect(experiment, SIGNAL(experimentCompleted()), window, SLOT(experimentComplete()));
        connect(experiment, SIGNAL(experimentProgress(double)), window, SLOT(progressBar(double)));

        /*connect(experiment, SIGNAL(tileWrittenToFile(CAImage,CARange<int>,CARange<int>)),
                window, SLOT(displayTile(CAImage,CARange<int>,CARange<int>)));*/
        disconnect(window, SIGNAL(createExperiment()), this, SLOT(createExperiment()));
        emit(experimentCreated());

        experiment->openWindow();
    }
}

///
/// \brief CAUmucoMain::stageSelectionChanged
/// \param index
///
void CAUmucoMain::stageSelectionChanged(int index)
{
    if(stage != NULL)
        delete stage;

    QStringList items = CADeviceManager::supportedStages.keys();
    QString selected = items.value(index);

    stage = CADeviceManager::supportedStages[selected]();
    connect(stage, SIGNAL(connectionState(bool)), window, SLOT(stageState(bool)));
    connect(window, SIGNAL(stageDisplayRequest()), stage, SLOT(openWindow()));
    //the first signal will likely be missed
    if(stage->isAlive) window->stageState(true);
}

///
/// \brief CAUmucoMain::cameraSelectionChanged
/// \param index
///
void CAUmucoMain::cameraSelectionChanged(int index)
{
    if(camera != NULL)
        delete camera;

    QStringList items = CADeviceManager::supportedCameras.keys();
    QString selected = items.value(index);

    camera = CADeviceManager::supportedCameras[selected]();
    if(camera->initialized)
        cameraConnection(true);
    connect(camera, SIGNAL(connection(bool)), this, SLOT(cameraConnection(bool)));
    //display
    connect(window, SIGNAL(cameraDisplayRequest()), camera, SLOT(openWindow()));
    connect(camera, SIGNAL(imageAvailable(CAImage)), window, SLOT(displayImage(CAImage)), Qt::AutoConnection);
    //acquisition
    connect(window, SIGNAL(startPreview()), camera, SLOT(startCapture()));
    connect(window, SIGNAL(stopPreview()), camera, SLOT(stopCapture()));
    connect(window, SIGNAL(snapImage()), camera, SLOT(snapImage()));
    connect(window, SIGNAL(alignmentTool()), this, SLOT(alignmentTool()));
}

///
/// \brief CAUmucoMain::cameraConnection
/// \param status
///
void CAUmucoMain::cameraConnection(bool status)
{
    if(!status){
        delete camera;
        camera = NULL;

        if(experiment != NULL){
            delete experiment;
            connect(window, SIGNAL(createExperiment()), this, SLOT(createExperiment()));
        }
    }
    window->connectionCamera(status);
}

///
/// \brief CAUmucoMain::alignmentTool
///
void CAUmucoMain::alignmentTool()
{
    if(alignTool == NULL){
        alignTool = new CAAlignmentTool(camera->pixelSize);
        connect(camera, SIGNAL(imageAvailable(CAImage)), alignTool, SLOT(besselProfiles(CAImage)));
        connect(camera, SIGNAL(pixelSizeChanged(QSize)), alignTool, SLOT(setPixelSize(QSize)));
    }
    alignTool->show();
    alignTool->raise();
}
