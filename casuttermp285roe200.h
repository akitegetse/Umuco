#ifndef CASUTTERMP285ROE200_H
#define CASUTTERMP285ROE200_H

#include "castage.h"
#include <QMessageBox>

class CASutterMP285ROE200 : public CAStageDevice
{
    Q_OBJECT

public:
    enum SutterStateType {IDLE, FETCHINGPOSITION, MOVEINPROGRESS};
    CASutterMP285ROE200();
    ~CASutterMP285ROE200();

private:
    SutterStateType  currentState;
    QString description;
    QSerialPort * serial;
    QTimer * positionPollTimer;
    QTimer * responseTimer;
    QTimer * reconnectTimer;
    QMessageBox* disconnectionMsgBox;
    QMutex * lock;
    char rxBuffer[14];
    int rxBytes;

    int autodetect();
    virtual void doMoveToPosition(double x, double y, double z);

    static void formatTxData(double x, double y, double z, unsigned char* outBuffer);
    static void formatRxData(const char* inBuffer, double* x, double* y, double* z);

signals:
    void error(QString);

private slots:
    void setupConnection();
    void responseTimeOut();
    void doFetchPosition();
    void sutterRxHandle();
};

#endif // CASUTTERMP285ROE200_H
