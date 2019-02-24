#include "caexperiment.h"
#include <QtConcurrent/QtConcurrent>

///
/// \brief CAExperiment::CAExperiment
/// \param parent
/// \param holder
/// \param imSensor
///
CAExperiment::CAExperiment(QObject *parent,
                           CAStage * holder,
                           CACamera * imSensor):
    QObject(parent),
    window(new CAExperimentWindow),
    stage(holder),
    camera(imSensor),
    expFile(NULL),
    stacksTilesCfg(new CAStacksAndTiles()),
    stacksEna(false),
    tilesEna(false),
    tilesOverlap(0.0),
    pixelSize(imSensor->pixelSize),
    streamTimer(NULL),
    resumeTimer(new QTimer)
{
    connect(stage, SIGNAL(positionUpdated(double,double,double)),
            window, SLOT(displayPosition(double,double,double)));
    connect(window, SIGNAL(stacksAndTilesSetRange(CARange<double>,CARange<double>,double, CARange<double>)),
            this, SLOT(stacksAndTilesEnable(CARange<double>,CARange<double>,double, CARange<double>)));
    connect(window, SIGNAL(stacksAndTilesDisabled()), this, SLOT(stacksAndTilesDisable()));
    connect(window, SIGNAL(streamSetDuration(int)), this, SLOT(streamEnable(int)));
    connect(camera,SIGNAL(pixelSizeChanged(QSize)), this, SLOT(pixelSizeSet(QSize)));

    window->initialize();
}

///
/// \brief CAExperiment::~CAExperiment
///
CAExperiment::~CAExperiment()
{

    if(streamTimer != NULL){
        streamTimer->stop();
        delete streamTimer;
    }

    if(expFile != NULL)
        delete expFile;

    delete resumeTimer;
    delete stacksTilesCfg;
}

///
/// \brief CAExperiment::openWindow
///
void CAExperiment::openWindow()
{
    window->show();
    window->raise();
}

///
/// \brief CAExperiment::stacksAndtilesDisable
///
void CAExperiment::stacksAndTilesDisable()
{
    stacksEna = false;
    tilesEna = false;

    CARange<double> range = stacksTilesCfg->x;
    stacksTilesCfg->x.min = range.value;
    stacksTilesCfg->x.max = range.value;
    stacksTilesCfg->x.step = 0;

    CARange<double> rangeY = stacksTilesCfg->y;
    stacksTilesCfg->y.min = rangeY.value;
    stacksTilesCfg->y.max = rangeY.value;
    stacksTilesCfg->y.step = 0;

    CARange<double> rangeZ = stacksTilesCfg->z;
    stacksTilesCfg->z.min = rangeZ.value;
    stacksTilesCfg->z.max = rangeZ.value;
    stacksTilesCfg->z.step = 0;
}

///
/// \brief CAExperiment::stacksAndTilesEnable
/// \param widthRange
/// \param heightRange
/// \param tileOverlap
/// \param stacksRange
///
void CAExperiment::stacksAndTilesEnable(CARange<double> widthRange,
                                        CARange<double> heightRange,
                                        double tileOverlap, CARange<double> stacksRange)
{
    if(stage != NULL){
        stacksEna = stacksRange.min != stacksRange.max
                && (stacksRange.min + stacksRange.step) <= stacksRange.max;
        stacksTilesCfg->x = stacksRange;

        tilesOverlap = tileOverlap;
        double utilFrac = 1.0 - tileOverlap;
        widthRange.step = pixelSize.width()/1000.0 *camera->regionOfInterest.width * utilFrac;
        heightRange.step = pixelSize.height()/1000.0 *camera->regionOfInterest.height * utilFrac;

        tilesEna = (widthRange.min != widthRange.max
                && (widthRange.min + widthRange.step) <= widthRange.max)
                ||(heightRange.min != heightRange.max
                && (heightRange.min + heightRange.step) <= heightRange.max);
        widthRange.max = widthRange.min + (widthRange.steps()- 1 ) * widthRange.step;
        heightRange.max = heightRange.min + (heightRange.steps() - 1) * heightRange.step;
        stacksTilesCfg->y = widthRange;
        stacksTilesCfg->z = heightRange;

        window->setTileSize(QSize(stacksTilesCfg->y.step, stacksTilesCfg->z.step));
    }
}

///
/// \brief CAExperiment::pixelSizeSet
/// \param size
///
void CAExperiment::pixelSizeSet(QSize size)
{
    pixelSize = size;
    double utilFrac = 1.0 - tilesOverlap;
    stacksTilesCfg->y.step = pixelSize.width()/1000.0 *camera->regionOfInterest.width * utilFrac;;
    stacksTilesCfg->z.step = pixelSize.height()/1000.0 *camera->regionOfInterest.height * utilFrac;;
    window->setTileSize(QSize(stacksTilesCfg->y.step, stacksTilesCfg->z.step));
}

///
/// \brief CAExperiment::start
///
void CAExperiment::start()
{
    if(expFile != NULL)
        delete expFile;
    double bin = camera->binning * 1.0E-3;
    double voxelSize[3] = {bin * pixelSize.width(), bin * pixelSize.width(), stacksTilesCfg->x.step*1.0};
    expFile = new CAFileThread(voxelSize,camera->deviceName);
    if(expFile->isValid()){
        if(stacksEna || tilesEna){
            if(expFile->isValid()){
                expFile->createAttribute("tilesOverlap",&tilesOverlap,1);
                connect(stage, SIGNAL(connectionState(bool)),
                        this, SLOT(stageConnected(bool)));
                connect(this, SIGNAL(moveTo(double,double,double)),
                        stage, SLOT(moveToPosition(double,double,double)));
                connect(stage, SIGNAL(requestedMoveDone()),
                        this, SLOT(serviceDidMoveSH()));
                stacksTilesCfg->setToMin();
                CACoordinate position = stacksTilesCfg->getValues();
                emit(moveTo(position.x, position.y, position.z));
            }
        } else {
            if(expFile->isValid()){
                connect(camera, SIGNAL(imageAvailable(CAImage)),
                        this, SLOT(serviceFirstImage(CAImage)));
            }
        }
        camera->startCapture();
    } else {
        delete expFile;
        expFile = NULL;
        emit(experimentCompleted());
    }
}

///
/// \brief CAExperiment::stageConnected
/// \param state
///
void CAExperiment::stageConnected(bool state)
{
    if( !state){
        resumeTimer->setSingleShot(false);
        connect(resumeTimer, SIGNAL(timeout()), this, SLOT(resume()), Qt::UniqueConnection);
        resumeTimer->start(1000);
    }
}

void CAExperiment::resume()
{
    if (stage->isAlive){
        CACoordinate position = stacksTilesCfg->getValues();
        emit(moveTo(position.x, position.y, position.z));
    }
}

void CAExperiment::stop()
{
    camera->stopCapture();
    disconnect(camera, SIGNAL(imageAvailable(CAImage)),
               this, SLOT(serviceImageAvailable(CAImage)));
    disconnect(stage, SIGNAL(requestedMoveDone()),
               this, SLOT(serviceDidMoveSH()));
    disconnect(this, SIGNAL(moveTo(double,double,double)),
               stage, SLOT(moveToPosition(double,double,double)));
    if(streamTimer != NULL){
        streamTimer->stop();
        delete streamTimer;
        streamTimer = NULL;
    }
    delete expFile;
    expFile = NULL;
    emit(experimentCompleted());
}

///
/// \brief CAExperiment::serviceDidMoveSH
///
void CAExperiment::serviceDidMoveSH()
{
    disconnect(stage, SIGNAL(didnotMove()),
               this, SLOT(reemitMoveTo()));
    resumeTimer->stop();
    disconnect(resumeTimer, SIGNAL(timeout()), this, SLOT(resume()));
    connect(camera, SIGNAL(imageAvailable(CAImage)),
            this, SLOT(serviceImageAvailable(CAImage)));
}

void CAExperiment::reemitMoveTo()
{
    CACoordinate position = stacksTilesCfg->getValues();
    emit(moveTo(position.x, position.y, position.z));
}

void CAExperiment::serviceFirstImage(CAImage im)
{

    CAImage imCopy(im.rows, im.cols,im.type());
    im.copyTo(imCopy);

    disconnect(camera, SIGNAL(imageAvailable(CAImage)),
               this, SLOT(serviceFirstImage(CAImage)));
    connect(camera, SIGNAL(imageAvailable(CAImage)),
            this, SLOT(serviceImageAvailable(CAImage)));
    if(streamTimer != NULL){
        streamTimer->start();
    }
    expFile->writeData(imCopy, stacksTilesCfg->getValues());
}

///
/// \brief CAExperiment::serviceImageAvailable
/// \param buffer
///
void CAExperiment::serviceImageAvailable(CAImage buffer)
{
    double progress;
    CAImage imCopy(buffer.rows, buffer.cols, buffer.type());

    buffer.copyTo(imCopy);
    expFile->writeData(imCopy, stacksTilesCfg->getValues());
    if(stacksEna || tilesEna){
        if(tilesEna)
            emit(tileWrittenToFile(buffer, stacksTilesCfg->y, stacksTilesCfg->z));
        disconnect(camera, SIGNAL(imageAvailable(CAImage)),
                   this, SLOT(serviceImageAvailable(CAImage)));
        if(stacksTilesCfg->isComplete()){
            progress = 100.0;
            stop();
        } else {
            CACoordinate position = stacksTilesCfg->getValues();
            stacksTilesCfg->increment();
            position = stacksTilesCfg->getValues();
            progress = 100.0 * stacksTilesCfg->progress();
            connect(stage, SIGNAL(didnotMove()),
                       this, SLOT(reemitMoveTo()));
            emit(moveTo(position.x, position.y, position.z));
        }
    } else {
        int rem = streamTimer->remainingTime();
        int per = streamTimer->interval();
        progress = (per-rem)*100.0/per;
    }
    emit(experimentProgress(progress));
}

///
/// \brief CAExperiment::streamEnable
/// \param val
///
void CAExperiment::streamEnable(int val)
{
    streamTimer = new QTimer();
    streamTimer->setSingleShot(1);
    streamTimer->setInterval(val*1000);
    connect(streamTimer, SIGNAL(timeout()), this, SLOT(stop()));
}


