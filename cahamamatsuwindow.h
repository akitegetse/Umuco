#ifndef CAHAMAMATSUWINDOW_H
#define CAHAMAMATSUWINDOW_H

#include <QWidget>
#include <QMap>
#include <QVector>
#include <QSettings>

#include "cautilities.h"

namespace Ui {
class CAHamamatsuWindow;
}

class CAHamamatsuWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CAHamamatsuWindow(QWidget *parent = 0);
    ~CAHamamatsuWindow();

private:
    Ui::CAHamamatsuWindow *ui;

    QSettings settings;

signals:
    void exposureChanged(double value);
    void internalLineIntervalChanged(double value);
    void scanModeChanged(long mode);
    void globalExposureChanged(QString exp);
    void readoutDirChanged(QString dir);
    void sensorModeChanged(QString mode);
    void triggerSourceChanged(QString mode);
    void triggerTypeChanged(QString type);
    void triggerPolarityChanged(QString pol);
    void triggerOutputChanged(int outputNbr, QString mode);
    void binningChanged(long binSize);
    void regionOfInterestChanged(CARoi);
    void pixelSizeChanged(QSize);

public slots:
    void updateSensorInfo(QMap<QString,int> dirs,QVector<int> bin, QVector<int> scan,
                          QMap<QString,int > trigSources,QMap<QString, int>  tOutputs);
    void updateSensorModes(QMap<QString, int>  sModes);
    void updateTriggerOutputItems(QMap<QString, int> tModes);
    void updateReadoutDirections(QMap<QString,int> dirs);
    void updateExposureLimits(CARange<double> exposure);
    void updateInternalLineIntervalBounds(CARange<double> interval);
    void updateScanModes(QVector<int> modes);
    void updateGlobalExposure(QMap<QString,int> gExp);
    void updateTriggerSources(QMap<QString,int> sources);
    void updateTriggerTypes(QMap<QString,int> types);
    void updateTriggerPolarities(QMap<QString,int> pol);
    void updateRegionOfInterest(CARoi);

private slots:
    void triggerTypePUMCurrentIndexChanged(const QString &arg1);
    void triggerPolarityPUMCurrentIndexChanged(const QString &arg1);
    void triggerSourcePUMCurrentIndexChanged(const QString &arg1);
    void sensorModePUMCurrentIndexChanged(const QString &arg1);
    void scanModePUMCurrentIndexChanged(const QString &arg1);
    void globalExposurePUMCurrentIndexChanged(const QString &arg1);
    void readoutDirPUMCurrentIndexChanged(const QString &arg1);
    void binningPUMCurrentIndexChanged(const QString &arg1);
    void regionOfInterestTEChanged(void);
    void exposureTETextChanged();
    void exposureSliderValueChanged(int value);
    void internalLineIntervalTETextChanged();
    void internalLineIntervalSliderValueChanged(int value);
    void triggerOutput1PUMIndexChanged(const QString &arg1);
    void triggerOutput2PUMIndexChanged(const QString &arg1);
    void triggerOutput3PUMIndexChanged(const QString &arg1);
    void pixelSizeEditTextChanged();
};

#endif // CAHAMAMATSUWINDOW_H
