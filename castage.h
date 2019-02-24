#ifndef CAStage_H
#define CAStage_H

#include <QObject>
#include <QThread>
#include <QtSerialPort/QtSerialPort>
#include "castagewindow.h"

class CAPosition
{
public:
    double x;
    double y;
    double z;

    // default + parameterized constructor
    CAPosition(double x, double y , double z);
};

class CAStageDevice : public QObject
{
    Q_OBJECT
public:
    //methods
    explicit CAStageDevice(QObject *parent = 0);
     ~CAStageDevice();

public slots:
    virtual void doMoveToPosition(double x, double y, double z);

signals:
    void positionUpdated(double x, double y, double z);
    void requestedMoveDone();
    void didnotMove();
    void connectionState(bool connected);
};


class CAStage : public QThread //QObject
{
    Q_OBJECT
public:
    //properties
    bool isAlive;
    CAStageDevice * device;

    //methods
    explicit CAStage(QObject * parent, CAStageDevice * theDevice);
     ~CAStage();
    void readPosition(double *xPtr, double *yPtr, double *zPtr);


protected:
    CAPosition * position;
    CAStageWindow * window;

signals:
    void positionUpdated(double x, double y, double z);
    void requestedMoveDone();
    void didnotMove();
    void doMoveTo(double x, double y, double z);
    void connectionState(bool connected);

public slots:
    virtual void openWindow();
    virtual void moveToPosition(double x, double y, double z);
    virtual void moveByDistance(double x, double y, double z);

private slots:
    void deviceStateChanged(bool state);
    void positionChanged(double x, double y, double z);
};


#endif // CAStage_H
