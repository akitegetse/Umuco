#include "caimagesimulator.h"

///
/// \brief CAImageSimulator::CAImageSimulator
///
CAImageSimulator::CAImageSimulator():
    CACamera(),
    image (new CAImage(2048, 2048, 2, 1)),
    genTimer(new QTimer)
{
    sprintf(deviceName, "Simulator");
    regionOfInterest = CARoi(0,0,2048,2048);
    triangle();
    connect(genTimer, SIGNAL(timeout()), this,SLOT(updateImage()));
    genTimer->setSingleShot(false);
    initialized = true;
}

///
/// \brief CAImageSimulator::~CAImageSimulator
///
CAImageSimulator::~CAImageSimulator()
{
    genTimer->stop();
    delete image;
    delete genTimer;
}

///
/// \brief CAImageSimulator::updateImage
///
void CAImageSimulator::updateImage()
{
    unsigned long bytesPerLine = (unsigned long) image->step[0];
    unsigned long bytesPerImage = image->byteCount();

    unsigned char * lastLine = new unsigned char [bytesPerLine];
    unsigned char * lastLinePtr = (unsigned char *) image->row(image->rows-1).data;
    unsigned char * firstLinePtr = (unsigned char *) image->row(0).data;
    unsigned char * secondLinePtr = (unsigned char *) image->row(1).data;
    memcpy(lastLine,lastLinePtr,bytesPerLine);

    memmove(secondLinePtr, firstLinePtr, bytesPerImage-bytesPerLine);
    memcpy(firstLinePtr, lastLine, bytesPerLine);
    delete [] lastLine;
    emit(imageAvailable(*image));
}

///
/// \brief CAImageSimulator::startCapture
/// \return
///
int CAImageSimulator::startCapture()
{
    genTimer->setSingleShot(false);
    genTimer->start(10);

    return 0;
}

///
/// \brief CAImageSimulator::stopCapture
/// \return
///
int CAImageSimulator::stopCapture()
{
    genTimer->stop();

    return 0;
}

///
/// \brief CAImageSimulator::snapImage
/// \return
///
int CAImageSimulator::snapImage()
{
    genTimer->setSingleShot(true);
    genTimer->start(10);
    return 0;
}

///
/// \brief CAImageSimulator::triangle
///
void CAImageSimulator::triangle()
{
    for(int x = 0; x < image->cols; x++){
        for (int y = 0; y < image->rows; y++){
            if((x <= y) && (x%7)){
                image->at<unsigned short> (cv::Point(x,y)) = 125;
            } else {
                image->at <unsigned short> (cv::Point(x,y)) = 272;

            }
        }
    }

    /*cv::Mat mean = cv::Mat::zeros(1,1,CV_16UC1);
    cv::Mat sigma = cv::Mat::ones(1,1,CV_16UC1);
    sigma = 256 * sigma;
    cv::randn((cv::Mat)*image,  mean, sigma);*/
}

