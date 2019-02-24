#ifndef CAIMAGESIMULATOR_H
#define CAIMAGESIMULATOR_H

#include "cacamera.h"
#include "cautilities.h"
#include <QObject>
#include <QTimer>

class CAImageSimulator : public CACamera
{
    Q_OBJECT
public:
    CAImageSimulator();
    ~CAImageSimulator();

    int startCapture();
    int stopCapture();
    int snapImage();

private:
    CAImage * image;
    QTimer * genTimer;
    void triangle();

private slots:
    void updateImage();
};

#endif // CAIMAGESIMULATOR_H
