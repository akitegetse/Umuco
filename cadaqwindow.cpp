#include "cadaqwindow.h"
#include "ui_CADaqWindow.h"

///
/// \brief CADaqWindow::CADaqWindow
/// \param parent
///
CADaqWindow::CADaqWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CADaqWindow)
{
    ui->setupUi(this);
}

///
/// \brief CADaqWindow::~CADaqWindow
///
CADaqWindow::~CADaqWindow()
{
    delete ui;
}

///
/// \brief CADaqWindow::initialize
///
void CADaqWindow::initialize()
{
    int val = settings.value("daq/galvoChannel").toInt();
    ui->galvoVoltChannelPUM->setCurrentIndex(val);
    emit(galvoVoltChannelChanged(ui->galvoVoltChannelPUM->currentText()));

    val = settings.value("daq/cameraTrigChannel").toInt();
    ui->cameraTrigChannelPUM->setCurrentIndex(val);
    emit(cameraTrigChannelChanged(ui->cameraTrigChannelPUM->currentText()));

    val = settings.value("daq/cameraTrigReadyChannel").toInt();
    ui->cameraTrigReadyChannelPUM->setCurrentIndex(val);
    emit(cameraTrigReadyChannelChanged(ui->cameraTrigReadyChannelPUM->currentText()));

    val = settings.value("daq/galvoPosition").toInt();
    ui->galvoVoltSpinBox->setValue(val);
    emit(galvoPositionChanged(val));

    val = settings.value("daq/galvoClockSource").toInt();
    ui->galvoClockSourcePUM->setCurrentIndex(val);
    emit(galvoClockSourceChanged(ui->galvoClockSourcePUM->currentText()));

    val = settings.value("daq/galvoUpperLimit").toInt();
    ui->setUpperPushButton->setText(QString("SET TOP (CURRENT %1)").arg(val));
    emit(galvoUpperLimitChanged(val));

    val = settings.value("daq/galvoLowerLimit").toInt();
    ui->setLowerPushButton->setText(QString("SET BOTTOM (CURRENT %1)").arg(val));
    emit(galvoLowerLimitChanged(val));

    ui->galvoTrigDelaySlider->setTracking(false);

    connect(ui->setLowerPushButton, SIGNAL(released()),
            this, SLOT(onSetLowerPushButton()));
    connect(ui->setUpperPushButton, SIGNAL(released()),
            this, SLOT(onSetUpperPushButton()));

    connect(ui->galvoVoltChannelPUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(onGalvoVoltChannelChanged(QString)),
            Qt::UniqueConnection);
    connect(ui->galvoClockSourcePUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(onGalvoClockSourceChanged(QString)),
            Qt::UniqueConnection);
    connect(ui->cameraTrigChannelPUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(onCameraTrigChannelChanged(QString)),
            Qt::UniqueConnection);
    connect(ui->cameraTrigReadyChannelPUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(onCameraTrigReadyChannelChanged(QString)),
            Qt::UniqueConnection);
    connect(ui->galvoTrigDelaySlider, SIGNAL(valueChanged(int)),
            this, SLOT(onGalvoTrigDelayChanged(int)),
            Qt::UniqueConnection);
    connect(ui->galvoScanTypeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(galvoScanType(int)), Qt::UniqueConnection);
    connect(ui->galvoVoltSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(onGalvoVoltSpinBoxValueChanged(int)));
    connect(ui->galvoTrigDelaySpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(onGalvoTrigDelaySpinBoxValueChanged(int)));

}

///
/// \brief CADaqWindow::onGalvoVoltChannelChanged
/// \param text
///
void CADaqWindow::onGalvoVoltChannelChanged(QString text)
{
    settings.setValue("daq/galvoChannel",
                       ui->galvoVoltChannelPUM->currentIndex());
    emit(galvoVoltChannelChanged(text));

}

///
/// \brief CADaqWindow::onCameraTrigChannelChanged
/// \param text
///
void CADaqWindow::onCameraTrigChannelChanged(QString text)
{
    settings.setValue("daq/cameraTrigChannel",
                       ui->cameraTrigChannelPUM->currentIndex());
    emit(cameraTrigChannelChanged(text));
}

///
/// \brief CADaqWindow::onCameraTrigReadyChannelChanged
/// \param text
///
void CADaqWindow::onCameraTrigReadyChannelChanged(QString text)
{
    settings.setValue("daq/cameraTrigReadyChannel",
                      ui->cameraTrigReadyChannelPUM->currentIndex());
    emit(cameraTrigReadyChannelChanged(text));
}

///
/// \brief CADaqWindow::onSetUpperPushButton
///
void CADaqWindow::onSetUpperPushButton()
{
    int val = ui->galvoVoltSpinBox->value();
    settings.setValue("daq/galvoUpperLimit",val);
    ui->setUpperPushButton->setText(QString("SET TOP (CURRENT %1)").arg(val));
    emit(galvoUpperLimitChanged(val));
}

///
/// \brief CADaqWindow::onSetLowerPushButton
///
void CADaqWindow::onSetLowerPushButton()
{
    int val = ui->galvoVoltSpinBox->value();
    settings.setValue("daq/galvoLowerLimit",val);
    ui->setLowerPushButton->setText(QString("SET BOTTOM (CURRENT %1)").arg(val));
    emit(galvoLowerLimitChanged(val));
}

///
/// \brief CADaqWindow::onGalvoTrigDelayChanged
/// \param val
///
void CADaqWindow::onGalvoTrigDelayChanged(int val)
{
    settings.setValue("daq/galvoTrigDelay",val);
    disconnect(ui->galvoTrigDelaySpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(onGalvoTrigDelaySpinBoxValueChanged(int)));
    ui->galvoTrigDelaySpinBox->setValue(val);
    connect(ui->galvoTrigDelaySpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(onGalvoTrigDelaySpinBoxValueChanged(int)));
    emit(galvoTrigDelayChanged(val));
}

///
/// \brief CADaqWindow::onGalvoClockSourceChanged
/// \param text
///
void CADaqWindow::onGalvoClockSourceChanged(QString text)
{
    settings.setValue("daq/galvoClockSource",
                      ui->galvoClockSourcePUM->currentIndex());
    emit(galvoClockSourceChanged(text));
}

void CADaqWindow::onGalvoVoltSpinBoxValueChanged(int val)
{
    settings.setValue("daq/galvoPosition",val);
    emit(galvoPositionChanged(val));
}

void CADaqWindow::onGalvoTrigDelaySpinBoxValueChanged(int val)
{

    settings.setValue("daq/galvoTrigDelay",val);
    disconnect(ui->galvoTrigDelaySlider, SIGNAL(valueChanged(int)),
            this, SLOT(onGalvoTrigDelayChanged(int)));
    ui->galvoTrigDelaySlider->setValue(val);
    connect(ui->galvoTrigDelaySlider, SIGNAL(valueChanged(int)),
            this, SLOT(onGalvoTrigDelayChanged(int)),
            Qt::UniqueConnection);
    emit(galvoTrigDelayChanged(val));
}

///
/// \brief CADaqWindow::daqChannelsChanged
/// \param aoChannels
/// \param ciChannels
/// \param coChannels
/// \param terminals
///
void CADaqWindow::daqChannelsChanged(QList<QString> aoChannels,
                                     QList<QString> ciChannels,
                                     QList<QString> coChannels,
                                     QList<QString> terminals)
{
    Q_UNUSED(coChannels);

    ui->galvoVoltChannelPUM->clear();
    ui->galvoVoltChannelPUM->addItems(aoChannels);
    ui->cameraTrigChannelPUM->clear();
    ui->cameraTrigChannelPUM->addItems(terminals);
        ui->cameraTrigReadyChannelPUM->clear();
        ui->cameraTrigReadyChannelPUM->addItems(ciChannels);
    ui->galvoClockSourcePUM->clear();
    ui->galvoClockSourcePUM->addItems(terminals);
    initialize();
}

///
/// \brief CADaqWindow::disableGalvoScanType
/// \param ena
///
void CADaqWindow::disableGalvoScanType(bool ena)
{
    ui->galvoScanTypeComboBox->setEnabled(!ena);
}

