#include "caalignmenttool.h"
#include "ui_caalignmenttool.h"
#include <QtConcurrent/QtConcurrent>
#include <QPixmap>

bool lessThan(const double a, const double b){ return a < b;}

using namespace cv;

///
/// \brief CAAlignmentTool::CAAlignmentTool
/// \param parent
///
CAAlignmentTool::CAAlignmentTool(QSize pxSize, QWidget *parent) :
    pixelSize(pxSize),
    QWidget(parent),
    ui(new Ui::CAAlignmentTool),
    menuBar()
{
    qRegisterMetaType<QVector<double> >("QVector<double>");
    ui->setupUi(this);

    /*this->layout()->addWidget(&menuBar);
    fileMenu = menuBar.addMenu("File");
    saveAction = fileMenu->addAction("Save as image");

    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveScreenShot()));*/

    connect(this, SIGNAL(plotRequest(QCustomPlot*,QVector<double>,QVector<double>,QString,QString,QString)),
            this, SLOT(plotSignal(QCustomPlot*,QVector<double>,QVector<double>,QString,QString,QString)));
}

///
/// \brief CAAlignmentTool::~CAAlignmentTool
///
CAAlignmentTool::~CAAlignmentTool()
{
    delete ui;
}

///
/// \brief CAAlignmentTool::findProfile
/// \param im
///
void CAAlignmentTool::findProfile(CAImage im)
{
    cv::Mat col;
    cv::Mat colSum;
    cv::Mat image(im.rows, im.cols, CV_32FC1);
    im.convertTo(image, CV_32FC1);
    int fwhm = 0;
    double pxSize = pixelSize.width()*1.0E-3;

    for (int j = 0; j < image.cols; ++j){
        image.col(j).copyTo(col);
        fwhm += fullWidthAtHalfMaximum(col);
    }

    double dFWHM = fwhm * pxSize/image.cols;
    double val = 0.0;

    cv::reduce(image, colSum,1, CV_REDUCE_AVG);
    QVector<double> x(im.rows);
    QVector<double> y(im.rows);
    for( int i=0; i< im.rows; ++i ){
        y[i] =  colSum.at<float>(i);
        x[i] = val;
        val += pxSize;
    }
    QString xLabel("Distance [um]");
    QString yLabel("Intensity");
    QString legend = QString("FWHM = %1 um").arg(dFWHM, 6, 'f', 1);
    emit(plotRequest(ui->transversProfile,x, y, xLabel, yLabel, legend));
}

///
/// \brief CAAlignmentTool::peakLongitudinalProfile
/// \param im
///
void CAAlignmentTool::peakLongitudinalProfile(CAImage im)
{

    double pxSize = pixelSize.width()*1.0E-3;
    int res = im.cols/16;
    double xStep = pxSize*16;
    double val = 0.0;
    QVector<double> x(res);
    QVector<double> y(res);

    cv::Mat column(im.rows,1, CV_32FC1);
    cv::Mat image(im.rows, 16, CV_32FC1);

    for (int i = 0; i < res; i++){
        cv::Point minLoc, maxLoc;
        double min, max;
        im.colRange(i*16, i*16+15).convertTo(image, CV_32FC1);
        cv::reduce(image, column,1, CV_REDUCE_AVG);
        //image.col(1).copyTo(column);

        cv::minMaxLoc(column, &min, &max, &minLoc, &maxLoc);
        y[i] = maxLoc.y;
        x[i] = val;
        val += xStep;
    }
    QString xLabel("Distance [um]");
    QString yLabel("Peak position");
    QString legend("");
    emit(plotRequest(ui->peakPositionProfile,x, y, xLabel, yLabel, legend));
}


///
/// \brief CAAlignmentTool::focusLongitudinalProfile
/// \param im
///
void CAAlignmentTool::focusLongitudinalProfile(CAImage im)
{
    double pxSize = pixelSize.width()*1.0E-3;
    int res = im.cols/32;
    double xStep = pxSize*32;
    double val = 0.0;
    QVector<double> x(res);
    QVector<double> y(res);

    cv::Mat column(im.rows,1, CV_32FC1);
    cv::Mat image(im.rows, 32, CV_32FC1);

    for (int i = 0; i < res; i++){
        //cv::Mat column(im.rows,1, CV_32FC1);
        //im.col(i*16).convertTo(column, CV_32FC1);
        im.colRange(i*32, i*32+31).convertTo(image, CV_32FC1);
        //cv::reduce(image, column,1, CV_REDUCE_AVG);
        //int fwhm = fullWidthAtHalfMaximum(column);
        //y[i] =  fwhm * pxSize;

        int fwhm = 0;
        for (int j = 0; j < image.cols; ++j){
            image.col(j).copyTo(column);
            fwhm += fullWidthAtHalfMaximum(column);
        }

        double dFWHM = fwhm * pxSize/image.cols;

        y[i] = dFWHM;
        x[i] = val;
        val += xStep;
    }
    QString xLabel("Distance [um]");
    QString yLabel("FWHM [um]");
    QString legend("");
    emit(plotRequest(ui->focusLongitudinalProfile,x, y, xLabel, yLabel, legend));
}

///
/// \brief CAAlignmentTool::besselProfiles
/// \param image
///
void CAAlignmentTool::besselProfiles(CAImage image)
{
    if(profileFuture.isFinished()){
        profileFuture = QtConcurrent::run(this, &CAAlignmentTool::findProfile, image);
    }

    if(peakProfileFuture.isFinished()){
        peakProfileFuture = QtConcurrent::run(this, &CAAlignmentTool::peakLongitudinalProfile, image);
    }

    if(focusProfileFuture.isFinished()){
        focusProfileFuture = QtConcurrent::run(this, &CAAlignmentTool::focusLongitudinalProfile, image);
    }
}

///
/// \brief CAAlignmentTool::plotSignal
/// \param plot
/// \param xAxis
/// \param yAxis
/// \param xLabel
/// \param yLabel
/// \param legend
///
void CAAlignmentTool::plotSignal(QCustomPlot * plot,QVector<double> xAxis, QVector<double> yAxis,
                                           QString xLabel, QString yLabel, QString legend)
{

    plot->clearGraphs();
    plot->clearItems();
    // create graph and assign data to it:
    plot->addGraph();
    plot->graph(0)->setData(xAxis, yAxis);
    // give the axes some labels:
    plot->xAxis->setLabel(xLabel);
    plot->yAxis->setLabel(yLabel);
    // set axes ranges, so we see all data:
    qSort(xAxis.begin(),xAxis.end(),lessThan);
    qSort(yAxis.begin(),yAxis.end(),lessThan);
    plot->xAxis->setRange(xAxis.first(), xAxis.last());
    const auto mean = std::accumulate(yAxis.begin(), yAxis.end(), .0) / yAxis.size();
    double yRange = yAxis.last() - yAxis.first();
    double yMin = floor(mean - yRange);
    double yMax = ceil(mean + yRange);
    plot->yAxis->setRange(yMin, yMax);

    // add the text label
    if (legend.compare("") != 0){
        QCPItemText *textLabel = new QCPItemText(plot);
        plot->addItem(textLabel);
        textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
        textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
        textLabel->position->setCoords(0.8, 0); // place position at center/top of axis rect
        textLabel->setText(legend);
        textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
        textLabel->setPen(QPen(Qt::red)); // show black border around text
    }

    plot->replot();
}

int CAAlignmentTool::fullWidthAtHalfMaximum(Mat y)
{
    assert(y.cols == 1);
    assert(y.type() == CV_32FC1);
    cv::Point minLoc, maxLoc;
    double min, max;

    cv::minMaxLoc(y, &min, &max, &minLoc, &maxLoc);

    double thres = (min + max)/2;
    Mat index(y.rows, 1.0, CV_32FC1);
    cv::threshold(y, index, thres, 1, THRESH_BINARY);

    int lastVal = 0;
    int firstIndex = 0;
    int lastIndex = 0;
    for (int i = 0; i< y.rows; i++){
        if(lastVal == 0 && index.at<float>(i)!= 0){
            firstIndex = i;
            lastVal = 1;
        } else if(lastVal == 1 && index.at<float>(i) == 0){
            lastIndex = i;
            lastVal = 0;
        }
    }

    return (lastIndex - firstIndex);
}

///
/// \brief CAAlignmentTool::saveScreenShot
///
void CAAlignmentTool::saveScreenShot()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File",
                                                    "","png format images (*.png)");
    if(fileName.compare("") != 0){
        QFileInfo fileInfo(fileName);
        if(fileInfo.completeSuffix().compare("png", Qt::CaseSensitive) == 0){
            QPixmap pixmap(this->size());
            this->render(&pixmap, QPoint());
            pixmap.save(fileName);
        } else {
            QWidget msgBox;
            QMessageBox::critical(&msgBox, tr("FILE FORMAT"),
                                  tr("Only PNG files can be written\n"),
                                  QMessageBox::Ok);
        }
    }
}

///
/// \brief CAAlignmentTool::setPixelSize
/// \param pxSize
///
void CAAlignmentTool::setPixelSize(QSize pxSize)
{
    pixelSize = pxSize;
}
