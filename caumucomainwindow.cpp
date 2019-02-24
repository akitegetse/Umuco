#include "caumucomainwindow.h"
#include "ui_caumucomainwindow.h"
#include "cadevicemanager.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>

///
/// \brief CAUmucoMainWindow::CAUmucoMainWindow
/// \param parent
///
CAUmucoMainWindow::CAUmucoMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CAUmucoMainWindow)
{
    QDesktopWidget wid;

    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();

    int xNew = screenWidth/2 - width()/2;
    int yNew = screenHeight/2 - height()/2;
    setGeometry(xNew, yNew, width(), height());

    ui->setupUi(this);

    ui->startStopPB->setAutoFillBackground( true );
    ui->startStopPB->setStyleSheet("background-color: green");
    ui->startStopPB->setEnabled(false);
    ui->previewPB->setStyleSheet("background-color: yellow");
    ui->previewPB->setEnabled(false);
    ui->captureBP->setStyleSheet("background-color: white");
    ui->captureBP->setEnabled(false);
    ui->saveBP->setStyleSheet("background-color: white");
    ui->stagePB->setEnabled(false);
    ui->cameraPB->setEnabled(false);

    connect(ui->stagePUM, SIGNAL(currentIndexChanged(int)),this, SIGNAL(stageSelectionChanged(int)));
    connect(ui->stagePB, SIGNAL(clicked()), this, SIGNAL(stageDisplayRequest()));
    connect(ui->daqComboBox, SIGNAL(currentTextChanged(QString)), this, SIGNAL(daqDeviceNameChanged(QString)));
    connect(ui->daqPB, SIGNAL(clicked()), this, SIGNAL(daqDisplayRequest()));

    connect(ui->cameraPUM, SIGNAL(currentIndexChanged(int)), this, SIGNAL(cameraSelectionChanged(int)));
    connect(ui->cameraPB, SIGNAL(clicked()), this, SIGNAL(cameraDisplayRequest()));
    connect(ui->captureBP,SIGNAL(clicked()),this, SIGNAL(snapImage()));
    connect(ui->previewPB, SIGNAL(clicked()), this, SLOT(previewPBClicked()));

    connect(ui->experimentPB, SIGNAL(clicked()), this, SIGNAL(createExperiment()));
    connect(ui->startStopPB, SIGNAL(clicked()), this, SLOT(startStopPBClicked()));
    connect(ui->saveBP, SIGNAL(clicked()), this, SLOT(savePBClicked()));

    connect(ui->contrastAdjustSlider, SIGNAL(valueChanged(int)), this, SLOT(contrastAdjustSliderValueChanged(int)));
    connect(ui->synchronizeCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(acqScanSynchronize(bool)));

    createActions();
}

///
/// \brief CAUmucoMainWindow::~CAUmucoMainWindow
///
CAUmucoMainWindow::~CAUmucoMainWindow()
{
    delete ui;
}

///
/// \brief CAUmucoMainWindow::closeEvent
/// \param event
///
void CAUmucoMainWindow::closeEvent (QCloseEvent *event)
{
    const QString appname = QString("Umuco");
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, appname,
                                                                tr("Quit Umuco?\n"),
                                                                QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::No);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
        emit(closeRequest());
    }
}

///
/// \brief CAUmucoMainWindow::uiUpdate
///
void CAUmucoMainWindow::uiUpdate()
{
    //It is important to have these lines after the "connect" to make sure devices are initialized
    ui->cameraPUM->addItems(CADeviceManager::supportedCameras.keys());
    ui->stagePUM->addItems(CADeviceManager::supportedStages.keys());
    ui->daqComboBox->addItems(CADeviceManager::installedNIDaqDevices);
}

///
/// \brief CAUmucoMainWindow::createActions
///
void CAUmucoMainWindow::createActions()
{
    connect(ui->actionSAVE, SIGNAL(triggered()),this, SLOT(savePBClicked()));
    connect(ui->actionAlignment, SIGNAL(triggered()), this, SIGNAL(alignmentTool()));
    connect(ui->actionTopHat, SIGNAL(toggled(bool)), ui->imageView, SLOT(enableTopHat(bool)) );
}

///
/// \brief CAUmucoMainWindow::progressBar
/// \param val
///
void CAUmucoMainWindow::progressBar(double val)
{
    int value = val *10;
    ui->expProgressBar->setValue(value);
}

///
/// \brief CAUmucoMainWindow::stageState
/// \param state
///
void CAUmucoMainWindow::stageState(bool state)
{
    ui->stagePB->setEnabled(state);
}

///
/// \brief CAUmucoMainWindow::startStopPBClicked
///
void CAUmucoMainWindow::startStopPBClicked()
{
    if(ui->startStopPB->text().compare("STOP") != 0){
        ui->startStopPB->setText(QString("STOP"));
        ui->startStopPB->setStyleSheet("background-color: red");

        ui->previewPB->hide();
        ui->captureBP->hide();
        /// This signal must be emitted after interface changes
        /// to make sure changes from the listing slot are taken into account
        emit(startExperiment());
    } else {
        ui->startStopPB->setText(QString("START EXPERIM."));
        ui->startStopPB->setStyleSheet("background-color: green");

        ui->previewPB->show();
        ui->captureBP->show();

        /// This signal must be emitted after interface changes
        /// to make sure changes from the listing slot are taken into account
        emit(stopExperiment());
    }
}

///
/// \brief CAUmucoMainWindow::experimentCreated
///
void CAUmucoMainWindow::experimentCreated()
{
    ui->startStopPB->setEnabled(true);
}

///
/// \brief experimentComplete
///
void CAUmucoMainWindow::experimentComplete()
{
    ui->startStopPB->setText(QString("START EXPERIM."));
    ui->startStopPB->setStyleSheet("background-color: green");

    ui->previewPB->show();
    ui->captureBP->show();
}

///
/// \brief CAUmucoMainWindow::previewPBClicked
///
void CAUmucoMainWindow::previewPBClicked()
{
    if(ui->previewPB->text().compare("STOP")){
        emit(startPreview());
        ui->previewPB->setText(QString("STOP"));
        ui->previewPB->setStyleSheet("background-color: red");

        ui->startStopPB->hide();
        ui->captureBP->hide();
    } else {
        emit(stopPreview());
        ui->previewPB->setText(QString("START PREVIEW"));
        ui->previewPB->setStyleSheet("background-color: yellow");

        ui->startStopPB->show();
        ui->captureBP->show();
    }
}

///
/// \brief CAUmucoMainWindow::connectionCamera
/// \param status
///
void CAUmucoMainWindow::connectionCamera(bool status)
{
    ui->cameraPB->setEnabled(status);
    ui->previewPB->setEnabled(status);
    ui->captureBP->setEnabled(status);
}

///
/// \brief CAUmucoMainWindow::saveBP_clicked
///
void CAUmucoMainWindow::savePBClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File",
                                                    "","images (*.tif *tiff *.png)");
    if(fileName.compare("") != 0){
        CAImage imToSave = ui->imageView->getImage();
        std::vector<int> params;
        params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        params.push_back(2);
        cv::imwrite(fileName.toStdString().c_str(), imToSave, params);
    }
}

///
/// \brief CAUmucoMainWindow::displayImage
/// \param buf
///
void CAUmucoMainWindow::displayImage(CAImage buf)
{
    ui->imageView->setImage(&buf);
}

///
/// \brief CAUmucoMainWindow::displayTile
/// \param image
/// \param xPosInfo
/// \param yPosInfo
///
void CAUmucoMainWindow::displayTile(CAImage image, CARange<int> xPosInfo, CARange<int> yPosInfo)
{
    if (!isVisible()){
        int xTiles = xPosInfo.steps();
        int yTiles = yPosInfo.steps();
        ui->imageView->setImageSize(xTiles*image.cols, yTiles * image.rows);
        show();
    }

    double xDoneTiles = xPosInfo.stepsDone();
    double yDoneTiles = yPosInfo.stepsDone();

    int x = image.cols * xDoneTiles;
    int y = image.rows * yDoneTiles;
    ui->imageView->setSubImage(x, y, &image);
}

///
/// \brief CAUmucoMainWindow::contrastAdjustSliderValueChanged
/// \param value
///
void CAUmucoMainWindow::contrastAdjustSliderValueChanged(int value)
{
    double linearValue = pow(10.0,value/1000.0);
    ui->imageView->setImContrast(linearValue);
}

///
/// \brief CAUmucoMainWindow::resizeEvent
/// \param event
///
void CAUmucoMainWindow::resizeEvent(QResizeEvent * event)
{
    QMainWindow::resizeEvent(event);

    QSize size = event->size();
    int w = size.width();
    int h = size.height();
    double aspectRatio = h/w;
    if(aspectRatio < 0.84 || aspectRatio > 0.86){
        w = h/0.85;
    }
    QDesktopWidget wid;

    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();

    int xNew = screenWidth/2 - w/2;
    int yNew = screenHeight/2 - h/2;
    setGeometry(xNew, yNew, w, h);

}

///
/// \brief CAUmucoMainWindow::sizeHint
/// \return
///
QSize CAUmucoMainWindow::sizeHint() const
{
    QSize s = size();
    s.setWidth((s.height()*100)/85);
    s.setHeight(QMainWindow::sizeHint().height());
    return s;
}
