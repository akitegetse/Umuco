#ifndef CACamera_H
#define CACamera_H

#include <QThread>
#include <QSize>
#include "cautilities.h"

class CACamera : public QThread
{
    Q_OBJECT
public:
    //methods
    explicit CACamera(QObject *parent = 0);
    ~CACamera();

    //properties
    char deviceName[64];
    long binning;
    CARoi regionOfInterest;
    QSize pixelSize;
    bool acquiring;
    bool initialized;

signals:
    void exposureChanged(double value);
    void pixelSizeChanged(QSize);
    void imageAvailable(CAImage buffer);
    void connection(bool status);

public slots:
    //this slots will not be executed in this new thread
    // They are executed in the creator of this one
    virtual void openWindow();
    virtual int startCapture();
    virtual int stopCapture();
    virtual int snapImage();
};

#endif // CACamera_H
