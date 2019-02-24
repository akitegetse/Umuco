#include "cahamamatsuwindow.h"
#include "ui_cahamamatsuwindow.h"
#include <math.h>

///
/// \brief CAHamamatsuWindow::CAHamamatsuWindow
/// \param parent
///
CAHamamatsuWindow::CAHamamatsuWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CAHamamatsuWindow)
{
    ui->setupUi(this);
    connect(ui->triggerSourcePUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerSourcePUMCurrentIndexChanged(QString)));
    connect(ui->triggerTypePUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerTypePUMCurrentIndexChanged(QString)));
    connect(ui->triggerPolarityPUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerPolarityPUMCurrentIndexChanged(QString)));
    connect(ui->exposureTE, SIGNAL(editingFinished()),
            this, SLOT(exposureTETextChanged()));
    connect(ui->exposureSlider, SIGNAL(valueChanged(int)),
            this, SLOT(exposureSliderValueChanged(int)));
    connect(ui->internalLineIntervalTE, SIGNAL(editingFinished()),
            this, SLOT(internalLineIntervalTETextChanged()));
    connect(ui->internalLineIntervalSlider, SIGNAL(valueChanged(int)),
            this, SLOT(internalLineIntervalSliderValueChanged(int)));
    connect(ui->scanModePUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(scanModePUMCurrentIndexChanged(QString)));
    connect(ui->globalExposurePUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(globalExposurePUMCurrentIndexChanged(QString)));
    connect(ui->sensorModePUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(sensorModePUMCurrentIndexChanged(QString)));
    connect(ui->readoutDirPUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(readoutDirPUMCurrentIndexChanged(QString)));
    connect(ui->binningPUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(binningPUMCurrentIndexChanged(QString)));
    connect(ui->roiXTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    connect(ui->roiYTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    connect(ui->roiWidthTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    connect(ui->roiWidthTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    connect(ui->pixelWidthEdit, SIGNAL(returnPressed()), this, SLOT(pixelSizeEditTextChanged()));
    connect(ui->pixelHeightEdit, SIGNAL(returnPressed()), this, SLOT(pixelSizeEditTextChanged()));
}

///
/// \brief CAHamamatsuWindow::~CAHamamatsuWindow
///
CAHamamatsuWindow::~CAHamamatsuWindow()
{
    delete ui;
}

///
/// \brief CAHamamatsuWindow::updateSensorInfo
/// \param dirs
/// \param bin
/// \param scan
/// \param trigTree
/// \param tOutputs
///
void CAHamamatsuWindow::updateSensorInfo
(
        QMap<QString,int> dirs,
        QVector<int> bin,
        QVector<int> scan,
        QMap<QString,int > trigSources,
        QMap<QString, int>  tOutputs
        )
{
    int val = settings.value("hamamatsu/binning").toInt();
    ui->binningPUM->clear();
    QVector<int>::iterator iterator;
    for(iterator = bin.begin(); iterator != bin.end(); iterator++){
        ui->binningPUM->addItem(QString::number((long) *iterator ,10));
    }
    ui->binningPUM->setCurrentIndex(val);

    updateTriggerSources(trigSources);
    updateTriggerOutputItems(tOutputs);
    updateScanModes(scan);
    updateReadoutDirections(dirs);

    QSize pixel;
    val = settings.value("experiment/pixelHeight").toInt();
    QString newText = QString("");
    newText.append(QString("%1 nm").arg(val,0));
    ui->pixelHeightEdit->setText(newText);
    pixel.setHeight(val);

    val = settings.value("experiment/pixelWidth").toInt();
    newText = QString("");
    newText.append(QString("%1 nm").arg(val,0));
    ui->pixelWidthEdit->setText(newText);
    pixel.setWidth(val);
    emit(pixelSizeChanged(pixel));
}

///
/// \brief CAHamamatsuWindow::updateSensorModes
/// \param sModes
///
void CAHamamatsuWindow::updateSensorModes(QMap<QString, int>  sModes)
{
    QString text = settings.value("hamamatsu/sensorMode").toString();
    ui->sensorModePUM->clear();
    ui->sensorModePUM->addItems(sModes.keys());
    ui->sensorModePUM->setCurrentText(text);
}

///
/// \brief CAHamamatsuWindow::updateTriggerSources
/// \param sources
///
void CAHamamatsuWindow::updateTriggerSources(QMap<QString, int> sources)
{
    if (sources.count() != 0){
        QString text = settings.value("hamamatsu/triggerSource").toString();
        ui->triggerSourcePUM->setEnabled(true);
        ui->triggerSourcePUM->clear();
        ui->triggerSourcePUM->addItems(sources.keys());
        ui->triggerSourcePUM->setCurrentText(text);
    } else ui->triggerSourcePUM->setEnabled(false);
}

///
/// \brief CAHamamatsuWindow::updateTriggerTypes
/// \param types
///
void CAHamamatsuWindow::updateTriggerTypes(QMap<QString, int> types)
{
    if (types.count() != 0){
        QString text = settings.value("hamamatsu/triggerType").toString();
        ui->triggerTypePUM->setEnabled(true);
        ui->triggerTypePUM->clear();
        ui->triggerTypePUM->addItems(types.keys());
        ui->triggerTypePUM->setCurrentText(text);
    } else ui->triggerTypePUM->setEnabled(false);
}

///
/// \brief CAHamamatsuWindow::updateTriggerPolarities
/// \param pol
///
void CAHamamatsuWindow::updateTriggerPolarities(QMap<QString, int> pol)
{
    if (pol.count() != 0){
        QString text = settings.value("hamamatsu/triggerPolarity").toString();
        ui->triggerPolarityPUM->setEnabled(true);
        ui->triggerPolarityPUM->clear();
        ui->triggerPolarityPUM->addItems(pol.keys());
        ui->triggerPolarityPUM->setCurrentText(text);
    } else ui->triggerPolarityPUM->setEnabled(false);
}

///
/// \brief CAHamamatsuWindow::triggerTypePUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::triggerSourcePUMCurrentIndexChanged(const QString &arg1)
{
    if(arg1.compare("") != 0){
        settings.setValue("hamamatsu/triggerSource", arg1);
        emit(triggerSourceChanged(arg1));
    }
}

///
/// \brief CAHamamatsuWindow::triggerTypePUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::triggerTypePUMCurrentIndexChanged(const QString &arg1)
{
    if(arg1.compare("") != 0){
        settings.setValue("hamamatsu/triggerType", arg1);
        emit(triggerTypeChanged(arg1));
    }
}

///
/// \brief CAHamamatsuWindow::triggerPolarityPUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::triggerPolarityPUMCurrentIndexChanged(const QString &arg1)
{
    if(arg1.compare("") != 0){
        settings.setValue("hamamatsu/triggerPolarity", arg1);
        emit(triggerPolarityChanged(arg1));
    }
}

///
/// \brief CAHamamatsuWindow::updateRegionOfInterest
/// \param roi
///
void CAHamamatsuWindow::updateRegionOfInterest(CARoi roi)
{
    disconnect(ui->roiXTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    disconnect(ui->roiYTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    disconnect(ui->roiWidthTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    disconnect(ui->roiWidthTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));

    ui->roiXTE->setText(QString::number(roi.x));
    ui->roiYTE->setText(QString::number(roi.y));
    ui->roiWidthTE->setText(QString::number(roi.width));
    ui->roiHeightTE->setText(QString::number(roi.height));

    connect(ui->roiXTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    connect(ui->roiYTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    connect(ui->roiWidthTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
    connect(ui->roiWidthTE, SIGNAL(returnPressed()), this, SLOT(regionOfInterestTEChanged()));
}

///
/// \brief CAHamamatsuWindow::pixelSizeEditTextChanged
///
void CAHamamatsuWindow::pixelSizeEditTextChanged()
{
    QString text =  ui->pixelWidthEdit->text();
    QStringList list = text.split(" ");
    int nbrStr = list.count();
    int xValue;

    if(nbrStr > 0){
        xValue = list[0].toDouble();
    } else {
        xValue = 0;
    }
    settings.setValue("experiment/pixelWidth", xValue);

    QString newTextX("");
    newTextX.append(QString("%1 nm").arg(xValue,0));
    ui->pixelWidthEdit->setText(newTextX);

    text =  ui->pixelHeightEdit->text();
    list = text.split(" ");
    nbrStr = list.count();
    int yValue;

    if(nbrStr > 0){
        yValue = list[0].toDouble();
    } else {
        yValue = 0;
    }
    settings.setValue("experiment/pixelHeight", yValue);

    QString newTextY("");
    newTextY.append(QString("%1 nm").arg(yValue,0));
    ui->pixelHeightEdit->setText(newTextY);

    QSize pixelSize(xValue, yValue);
    emit(pixelSizeChanged(pixelSize));
}

///
/// \brief CAHamamatsuWindow::exposureTETextChanged
///
void CAHamamatsuWindow::exposureTETextChanged()
{
    QString text =  ui->exposureTE->text();
    QStringList list = text.split(" ");
    int nbrStr = list.count();
    double value;
    double sliderValue = pow(10.0, ui->exposureSlider->value()/1000.0);
    double sliderMin = pow(10.0, ui->exposureSlider->minimum()/1000.0);
    double sliderMax = pow(10.0, ui->exposureSlider->maximum()/1000.0);

    bool formatNeeded = true;
    if(nbrStr > 0){
        value = list[0].toDouble();
        if (nbrStr > 1){
            QString units = list[1];
            formatNeeded = false;
            if (units.compare("ms", Qt::CaseInsensitive) == 0){
                value *= 1.0E-3;
            } else if (units.compare("us", Qt::CaseInsensitive) == 0){
                value *= 1.0E-6;
            } else if (units.compare("s", Qt::CaseInsensitive) != 0){
                value *= 1.0;
                formatNeeded = true;
            }
        }
        //Do not change value if out of bounds
        if (value < sliderMin || value > sliderMax)
            value = sliderValue;
    } else {
        value = sliderValue;
    }

    if(value != sliderValue){
        disconnect(ui->exposureSlider, SIGNAL(valueChanged(int)),
                this, SLOT(exposureSliderValueChanged(int)));
        ui->exposureSlider->setValue(log10(value)*1000);
        connect(ui->exposureSlider, SIGNAL(valueChanged(int)),
                this, SLOT(exposureSliderValueChanged(int)));
    }

    if(formatNeeded){
        QString newText("");
        if (value >= 1.0){ //seconds
            newText.append(QString("%1 s").arg(value,0,'f',3));
        } else if (value >= 1.0E-3){ // milliseconds
            newText.append(QString("%1 ms").arg(1.0E3*value,0,'f',3));
        } else { // microseconds
            newText.append(QString("%1 us").arg(1.0E6*value,0,'f',3));
        }
        disconnect(ui->exposureTE, SIGNAL(editingFinished()), this, SLOT(exposureTETextChanged()));
        ui->exposureTE->setText(newText);
        connect(ui->exposureTE, SIGNAL(editingFinished()), this, SLOT(exposureTETextChanged()));
    }

    emit(exposureChanged(value));
}

///
/// \brief CAHamamatsuWindow::exposureSliderValueChanged
/// \param value
///
void CAHamamatsuWindow::exposureSliderValueChanged(int value)
{
    double linearValue = pow(10.0,value/1000.0);
    ui->exposureTE->setText(QString("%1").arg(linearValue,0,'f',3));
    exposureTETextChanged();
}

///
/// \brief CAHamamatsuWindow::internalLineIntervalTETextChanged
///
void CAHamamatsuWindow::internalLineIntervalTETextChanged()
{
    QString text =  ui->internalLineIntervalTE->text();
    QStringList list = text.split(" ");
    int nbrStr = list.count();
    double value;
    double sliderValue = pow(10.0, ui->internalLineIntervalSlider->value()/1000.0);
    double sliderMin = pow(10.0, ui->internalLineIntervalSlider->minimum()/1000.0);
    double sliderMax = pow(10.0, ui->internalLineIntervalSlider->maximum()/1000.0);

    bool formatNeeded = true;
    if(nbrStr > 0){
        value = list[0].toDouble();
        if (nbrStr > 1){
            QString units = list[1];
            formatNeeded = false;
            if (units.compare("ms", Qt::CaseInsensitive) == 0){
                value *= 1.0E-3;
            } else if (units.compare("us", Qt::CaseInsensitive) == 0){
                value *= 1.0E-6;
            } else if (units.compare("s", Qt::CaseInsensitive) != 0){
                value *= 1.0;
                formatNeeded = true;
            }
        }
        //Do not change value if out of bounds
        if (value < sliderMin || value > sliderMax)
            value = sliderValue;
    } else {
        value = sliderValue;
    }

    if(value != sliderValue){
        disconnect(ui->internalLineIntervalSlider, SIGNAL(valueChanged(int)),
                this, SLOT(internalLineIntervalSliderValueChanged(int)));
        ui->internalLineIntervalSlider->setValue(log10(value)*1000);
        connect(ui->internalLineIntervalSlider, SIGNAL(valueChanged(int)),
                this, SLOT(internalLineIntervalSliderValueChanged(int)));
    }

    if(formatNeeded){
        QString newText("");
        if (value >= 1.0){ //seconds
            newText.append(QString("%1 s").arg(value,0,'f',3));
        } else if (value >= 1.0E-3){ // milliseconds
            newText.append(QString("%1 ms").arg(1.0E3*value,0,'f',3));
        } else { // microseconds
            newText.append(QString("%1 us").arg(1.0E6*value,0,'f',3));
        }
        disconnect(ui->internalLineIntervalTE, SIGNAL(editingFinished()), this, SLOT(internalLineIntervalTETextChanged()));
        ui->internalLineIntervalTE->setText(newText);
        connect(ui->internalLineIntervalTE, SIGNAL(editingFinished()), this, SLOT(internalLineIntervalTETextChanged()));
    }

    emit(internalLineIntervalChanged(value));
}

///
/// \brief CAHamamatsuWindow::internalLineIntervalSliderValueChanged
/// \param value
///
void CAHamamatsuWindow::internalLineIntervalSliderValueChanged(int value)
{
    double linearValue = pow(10.0,value/1000.0);
    ui->internalLineIntervalTE->setText(QString("%1").arg(linearValue,0,'f',3));
    internalLineIntervalTETextChanged();
}

///
/// \brief CAHamamatsuWindow::sensorModePUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::sensorModePUMCurrentIndexChanged(const QString &arg1)
{
    emit(sensorModeChanged(arg1));
    settings.setValue("hamamatsu/sensorMode", arg1);
}

///
/// \brief CAHamamatsuWindow::scanModePUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::scanModePUMCurrentIndexChanged(const QString &arg1)
{
    long longValue = arg1.toLong();
    emit(scanModeChanged(longValue));
    settings.setValue("hamamatsu/scanMode", ui->scanModePUM->currentIndex());
}

///
/// \brief CAHamamatsuWindow::globalExposurePUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::globalExposurePUMCurrentIndexChanged(const QString &arg1)
{
    settings.setValue("hamamatsu/globalExposure", arg1);
    emit(globalExposureChanged(arg1));
}

///
/// \brief CAHamamatsuWindow::readoutDirPUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::readoutDirPUMCurrentIndexChanged(const QString &arg1)
{
    emit(readoutDirChanged(arg1));
    settings.setValue("hamamatsu/readoutDirection", arg1);
}


///
/// \brief CAHamamatsuWindow::binningPUMCurrentIndexChanged
/// \param arg1
///
void CAHamamatsuWindow::binningPUMCurrentIndexChanged(const QString &arg1)
{
    long longValue = arg1.toLong();
    emit(binningChanged(longValue));
    settings.setValue("hamamatsu/binning", ui->binningPUM->currentIndex());
}

///
/// \brief CAHamamatsuWindow::regionOfInterestTEChanged
///
void CAHamamatsuWindow::regionOfInterestTEChanged(void)
{
    CARoi roi;
    roi.x = ui->roiXTE->text().toInt();
    roi.y = ui->roiYTE->text().toInt();
    roi.width = ui->roiWidthTE->text().toInt();
    roi.height = ui->roiHeightTE->text().toInt();
    settings.setValue("hamamatsu/roiX", roi.x);
    settings.setValue("hamamatsu/roiY", roi.y);
    settings.setValue("hamamatsu/roiWidth", roi.width);
    settings.setValue("hamamatsu/roiHeight", roi.height);
    emit(regionOfInterestChanged(roi));
}

void CAHamamatsuWindow::triggerOutput1PUMIndexChanged(const QString &arg1)
{
    settings.setValue("hamamatsu/output1",arg1);
    emit(triggerOutputChanged(0, arg1));
}

void CAHamamatsuWindow::triggerOutput2PUMIndexChanged(const QString &arg1)
{
    settings.setValue("hamamatsu/output2",arg1);
    emit(triggerOutputChanged(1, arg1));
}

void CAHamamatsuWindow::triggerOutput3PUMIndexChanged(const QString &arg1)
{
    settings.setValue("hamamatsu/output3",arg1);
    emit(triggerOutputChanged(2, arg1));
}

void CAHamamatsuWindow::updateTriggerOutputItems(QMap<QString, int> tModes)
{
    disconnect(ui->output1PUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerOutput1PUMIndexChanged(QString)));
    disconnect(ui->output2PUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerOutput2PUMIndexChanged(QString)));
    disconnect(ui->output3PUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerOutput3PUMIndexChanged(QString)));

    QString text = settings.value("hamamatsu/output1").toString();
    ui->output1PUM->clear();
    ui->output1PUM->addItems(tModes.keys());
    ui->output1PUM->setCurrentText(text);
    emit(triggerOutputChanged(0, text));

    text = settings.value("hamamatsu/output2").toString();
    ui->output2PUM->clear();
    ui->output2PUM->addItems(tModes.keys());
    ui->output2PUM->setCurrentText(text);
    emit(triggerOutputChanged(1, text));

    text = settings.value("hamamatsu/output3").toString();
    ui->output3PUM->clear();
    ui->output3PUM->addItems(tModes.keys());
    ui->output3PUM->setCurrentText(text);
    emit(triggerOutputChanged(2, text));

    connect(ui->output1PUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerOutput1PUMIndexChanged(QString)));
    connect(ui->output2PUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerOutput2PUMIndexChanged(QString)));
    connect(ui->output3PUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(triggerOutput3PUMIndexChanged(QString)));
}

void CAHamamatsuWindow::updateExposureLimits(CARange<double> exposure)
{
    if(exposure.max > exposure.min){
        ui->exposureTE->setEnabled(true);
        ui->exposureSlider->setEnabled(true);
        ui->exposureSlider->setRange(log10(exposure.min)*1000,log10(exposure.max)*1000);
        ui->exposureSlider->setValue(log10(exposure.value)*1000);
        ui->exposureSlider->setTickInterval(500);
        ui->exposureSlider->setOrientation(Qt::Horizontal);
        ui->exposureSlider->setTracking(false);

        QString newText("");
        if (exposure.value >= 1.0){ //seconds
            newText.append(QString("%1 s").arg(exposure.value,0,'f',3));
        } else if (exposure.value >= 1.0E-3){ // milliseconds
            newText.append(QString("%1 ms").arg(1.0E3*exposure.value,0,'f',3));
        } else { // microseconds
            newText.append(QString("%1 us").arg(1.0E6*exposure.value,0,'f',3));
        }
        ui->exposureTE->setText(newText);
    } else {
        ui->exposureTE->setEnabled(false);
        ui->exposureSlider->setEnabled(false);
    }
}

void CAHamamatsuWindow::updateInternalLineIntervalBounds(CARange<double> interval)
{
    if (interval.max > interval.min){
        ui->internalLineIntervalTE->setEnabled(true);
        ui->internalLineIntervalSlider->setEnabled(true);
        ui->internalLineIntervalSlider->setRange(log10(interval.min)*1000,log10(interval.max)*1000);
        ui->internalLineIntervalSlider->setValue(log10(interval.value)*1000);
        ui->internalLineIntervalSlider->setTickInterval(500);
        ui->internalLineIntervalSlider->setOrientation(Qt::Horizontal);
        ui->internalLineIntervalSlider->setTracking(false);

        QString newText("");
        if (interval.value >= 1.0){ //seconds
            newText.append(QString("%1 s").arg(interval.value,0,'f',3));
        } else if (interval.value >= 1.0E-3){ // milliseconds
            newText.append(QString("%1 ms").arg(1.0E3*interval.value,0,'f',3));
        } else { // microseconds
            newText.append(QString("%1 us").arg(1.0E6*interval.value,0,'f',3));
        }
        ui->internalLineIntervalTE->setText(newText);
    } else {
        ui->internalLineIntervalTE->setEnabled(false);
        ui->internalLineIntervalSlider->setEnabled(false);
    }
}

void CAHamamatsuWindow::updateReadoutDirections(QMap<QString,int> dirs)
{
    QString text = settings.value("hamamatsu/readoutDirection").toString();
    ui->readoutDirPUM->clear();
    disconnect(ui->readoutDirPUM, SIGNAL(currentIndexChanged(QString)),
               this, SLOT(readoutDirPUMCurrentIndexChanged(QString)));
    ui->readoutDirPUM->addItems(dirs.keys());
    connect(ui->readoutDirPUM, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(readoutDirPUMCurrentIndexChanged(QString)));
    ui->readoutDirPUM->setCurrentText(text);
}

void CAHamamatsuWindow::updateScanModes(QVector<int> modes)
{
    if (modes.count() != 0){
        ui->scanModePUM->setEnabled(true);
        QVector<int>::iterator iterator;
        ui->scanModePUM->clear();
        for(iterator = modes.begin(); iterator != modes.end(); iterator++){
            ui->scanModePUM->addItem(QString::number((long) *iterator ,10));
        }
    } else ui->scanModePUM->setEnabled(false);
}

void CAHamamatsuWindow::updateGlobalExposure(QMap<QString,int> gExp)
{
    if (gExp.count() != 0){
        QString text = settings.value("hamamatsu/globalExposure").toString();
        ui->globalExposurePUM->setEnabled(true);
        ui->globalExposurePUM->clear();
        ui->globalExposurePUM->addItems(gExp.keys());
        ui->globalExposurePUM->setCurrentText(text);
    } else ui->globalExposurePUM->setEnabled(false);
}
