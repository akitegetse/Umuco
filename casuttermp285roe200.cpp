#include "casuttermp285roe200.h"
#include <QMessageBox>

#define MICROSTEPSPERMICRON 16
#define SUTTER_BAUDRATE 128000
#define SUTTER_POSITION_LIMIT 128000

///
/// \brief CASutterMP285ROE200::CASutterMP285ROE200
///
CASutterMP285ROE200::CASutterMP285ROE200():
    currentState(CASutterMP285ROE200:: IDLE),
    description(QString("Sutter Instrument ROE-200 USB Serial Port")),
    serial(new QSerialPort(this)),
    positionPollTimer(new QTimer(this)),
    responseTimer(new QTimer(this)),
    reconnectTimer(new QTimer(this)),
    disconnectionMsgBox(new QMessageBox()),
    lock(new QMutex(QMutex::NonRecursive)),
    rxBytes(0)
{
    reconnectTimer->setSingleShot(true);
    positionPollTimer->setSingleShot(true);
    responseTimer->setSingleShot(true);

    //disconnectionMsgBox->setAttribute( Qt::WA_DeleteOnClose );
    disconnectionMsgBox->setStandardButtons( QMessageBox::Ok );
    disconnectionMsgBox->setWindowTitle( tr("Sample Holder disconnected") );
    disconnectionMsgBox->setText( tr("The sutter ROE-200 timed out\n"
                        "Make sure the device is connected to the computer\n"));
    disconnectionMsgBox->setModal( false );
    setupConnection();
}

///
/// \brief CASutterMP285ROE200::~CASutterMP285ROE200
///
CASutterMP285ROE200::~CASutterMP285ROE200()
{
    lock->tryLock();
    lock->unlock();
    delete lock;
    positionPollTimer->stop();
    responseTimer->stop();
    serial->close();
    delete positionPollTimer;
    delete responseTimer;
    delete reconnectTimer;
    delete disconnectionMsgBox;
}

///
/// \brief CASutterMP285ROE200::autodetect
/// \return
///
int CASutterMP285ROE200::autodetect()
{
    int success = 0;
    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo port, serialPorts){
        QSerialPortInfo info = QSerialPortInfo(port.portName());
        QString desc = info.description();

        if(desc.compare(description) == 0){
            serial->close();
            delete serial;
            serial = new QSerialPort(port.portName(), this);
            success = 1;
            break;
        }
    }

    return success;
}

///
/// \brief CASutterMP285ROE200::setupConnection
///
void CASutterMP285ROE200::setupConnection()
{
    positionPollTimer->stop();
    responseTimer->stop();
    reconnectTimer->stop();

    disconnect(positionPollTimer, SIGNAL(timeout()), this, SLOT(doFetchPosition()));
    disconnect(reconnectTimer, SIGNAL(timeout()), this, SLOT(setupConnection()));
    disconnect(responseTimer, SIGNAL(timeout()), this, SLOT(responseTimeOut()));

    if(autodetect()){
        bool failed = false;

        //setup serial port
        if (!serial->open(QIODevice::ReadWrite)) {
            failed = true;
            emit error(QString("Can't open %1, error code %2")
                       .arg(serial->portName()).arg(serial->error()));
        } else if (!serial->setBaudRate(SUTTER_BAUDRATE)) {
            failed = true;
            emit error(QString("Can't set baud rate 9600 baud to port %1, error code %2")
                       .arg(serial->portName()).arg(serial->error()));
        } else if (!serial->setDataBits(QSerialPort::Data8)) {
            failed = true;
            emit error(QString("Can't set 8 data bits to port %1, error code %2")
                       .arg(serial->portName()).arg(serial->error()));
        } else if (!serial->setParity(QSerialPort::NoParity)) {
            failed = true;
            emit error(QString("Can't set no patity to port %1, error code %2")
                       .arg(serial->portName()).arg(serial->error()));
        }else if (!serial->setStopBits(QSerialPort::OneStop)) {
            failed = true;
            emit error(QString("Can't set 1 stop bit to port %1, error code %2")
                       .arg(serial->portName()).arg(serial->error()));
        }else if (!serial->setFlowControl(QSerialPort::NoFlowControl)) {
            failed = true;
            emit error(QString("Can't set no flow control to port %1, error code %2")
                       .arg(serial->portName()).arg(serial->error()));
        } else { // serial initialization successful
            connect(serial, SIGNAL(readyRead()),
                    this, SLOT(sutterRxHandle()), Qt::UniqueConnection);
            connect(positionPollTimer, SIGNAL(timeout()),
                    this, SLOT(doFetchPosition()), Qt::UniqueConnection);
            positionPollTimer->start(250); //time specified in ms
            connect(responseTimer, SIGNAL(timeout()),
                    this, SLOT(responseTimeOut()),Qt::UniqueConnection);
            connect(responseTimer, SIGNAL(timeout()),
                    disconnectionMsgBox, SLOT(open()),Qt::UniqueConnection);
            currentState = CASutterMP285ROE200:: IDLE;
            //emit(connectionState(true));
        }

        if (failed){
            serial->close();
            connect(reconnectTimer, SIGNAL(timeout()),
                    this, SLOT(setupConnection()), Qt::UniqueConnection);
            reconnectTimer->start(5000); //try to connect in 5 sec
        }
    } else {
        connect(reconnectTimer, SIGNAL(timeout()),
                this, SLOT(setupConnection()), Qt::UniqueConnection);
        reconnectTimer->start(5000); //try to connect in 5 sec
    }
}

///
/// \brief CASutterMP285ROE200::doMoveToPosition
/// \param x
/// \param y
/// \param z
///
void CASutterMP285ROE200::doMoveToPosition(double x, double y, double z)
{
    if(x <= SUTTER_POSITION_LIMIT
            && y <= SUTTER_POSITION_LIMIT
            && z <= SUTTER_POSITION_LIMIT){
        uint8_t txData[15];
        txData[0] = 'M';
        CASutterMP285ROE200::formatTxData(x, y, z, txData+1);
        txData[13] = '\r';
        txData[14] = '\0';

        //send txData
        if(lock->tryLock(5000)){
            positionPollTimer->stop();
            serial->flush();
            if (-1 != serial->write((const char *)txData, 13)){
                //set waiting state
                currentState = CASutterMP285ROE200::MOVEINPROGRESS;
                responseTimer->start(10000); //10 sec timeout
                //lock->unlock(); //must stay locked till device answers
            } else {
                lock->unlock();
                responseTimeOut();
            }
        } else {
            emit(didnotMove());
        }
    }
}

///
/// \brief CASutterMP285ROE200::doFetchPosition
///
void CASutterMP285ROE200::doFetchPosition()
{
    uint8_t txData[3];
    txData[0] = 'C';
    txData[1] = '\r';
    txData[2] = '\0';
    //send 'C' to ask for current position
    if(lock->tryLock(100)){
        //serial->flush();
        if (-1 != serial->write((const char *)txData, 2)){
            //set waiting state
            currentState = CASutterMP285ROE200::FETCHINGPOSITION;
            responseTimer->start(5000); //1 sec timeout
            //lock->unlock(); //must stay locked till device answers
        } else {
            lock->unlock();
            responseTimeOut();
        }
    }
}

///
/// \brief CASutterMP285ROE200::sutterRxHandle
///
void CASutterMP285ROE200::sutterRxHandle()
{
    double x, y, z;

    //lock->lock(); //already locked
    switch(currentState){
    case CASutterMP285ROE200::MOVEINPROGRESS :
        rxBytes += serial->read(&rxBuffer[rxBytes], 2);
        if(rxBytes > 0 && strchr(rxBuffer, '\r') != NULL){
            serial->readAll();
            // The carriage return is sent after move completes
            rxBytes = 0;
            responseTimer->stop(); //stop timeout timer
            //lock->tryLock();
            lock->unlock();
            positionPollTimer->start();

            // Notify
            emit(requestedMoveDone());
            currentState = CASutterMP285ROE200::IDLE;
        }
        break;
    case CASutterMP285ROE200::FETCHINGPOSITION :
        // read the binary data
        rxBytes += serial->read(&rxBuffer[rxBytes], 14);
        if(rxBytes > 13){
            //isAlive = true;
            emit(connectionState(true));
            serial->readAll();
            // format received data to uint32_t
            CASutterMP285ROE200::formatRxData(rxBuffer, &x, &y, &z);
            rxBytes = 0;
            currentState = CASutterMP285ROE200::IDLE;

            responseTimer->stop(); //stop timeout timer
            //lock->tryLock();
            lock->unlock();
            positionPollTimer->start();

            //broadcast an event
            emit(positionUpdated(x,y,z));
        }
        break;
    default:
        qDebug("default %d %lld", currentState, serial->bytesAvailable());
        serial->readAll();
    }
}

///
/// \brief CASutterMP285ROE200::responseTimeOut
///
void CASutterMP285ROE200::responseTimeOut()
{
    serial->close();
    //emit something
    lock->tryLock();
    lock->unlock();

    /*isAlive = false;
    window->close();*/
    emit(connectionState(false));
    connect(reconnectTimer, SIGNAL(timeout()), this, SLOT(setupConnection()), Qt::UniqueConnection);
    reconnectTimer->start(5000);
}

///
/// \brief CASutterMP285ROE200::formatTxData
/// \param x
/// \param y
/// \param z
/// \param outBuffer
///
void CASutterMP285ROE200::formatTxData(double x, double y, double z, unsigned char* outBuffer)
{
    /* This code works on all machines processors regardless of endianness */
    int i = 0;
    uint32_t xint = (uint32_t) (x *MICROSTEPSPERMICRON);
    for (i = 0; i < 4; i++)
        outBuffer[i] = (xint >> i * 8) & 0xff;

    uint32_t yint = (uint32_t) (y * MICROSTEPSPERMICRON);
    for (i = 0; i < 4; i++)
        outBuffer[4+i] = (yint >> i * 8) & 0xff;

    uint32_t zint = (uint32_t) (z * MICROSTEPSPERMICRON);
    for (i = 0; i < 4; i++)
        outBuffer[8+i] = (zint >> i * 8) & 0xff;
}

///
/// \brief CASutterMP285ROE200::formatRxData
/// \param inBuffer
/// \param x
/// \param y
/// \param z
///
void CASutterMP285ROE200::formatRxData(const char* inBuffer, double* x, double* y, double* z)
{
    /* This code works on all machines processors regardless of endianness */
    int i = 0;

    uint32_t xint = 0;
    for (i = 0; i < 4; ++i)
        xint |= ((uint32_t)inBuffer[i] & 0xFF) << (i * 8);

    *x = xint/(MICROSTEPSPERMICRON * 256.0);

    uint32_t yint = 0;
    for (i = 0; i < 4; ++i)
        yint |= ((uint32_t)inBuffer[4+i] & 0xFF) << (i * 8);

    *y = yint/(MICROSTEPSPERMICRON * 256.0);

    uint32_t zint = 0;
    for (i = 0; i < 4; ++i)
        zint |= ((uint32_t)inBuffer[8+i] & 0xFF) << (i * 8);
    *z = zint/(MICROSTEPSPERMICRON * 256.0);
}
