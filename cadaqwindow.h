#ifndef CADAQWINDOW_H
#define CADAQWINDOW_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class CADaqWindow;
}

class CADaqWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CADaqWindow(QWidget *parent = 0);
    ~CADaqWindow();
    void initialize();

private slots:
    void onGalvoVoltChannelChanged(QString text);
    void onCameraTrigChannelChanged(QString text);
    void onCameraTrigReadyChannelChanged(QString text);
    void onSetLowerPushButton();
    void onSetUpperPushButton();
    void onGalvoTrigDelayChanged(int val);
    void onGalvoClockSourceChanged(QString text);
    void onGalvoVoltSpinBoxValueChanged(int val);
    void onGalvoTrigDelaySpinBoxValueChanged(int val);

public slots:
    void daqChannelsChanged(QList<QString> aoChannels,
                            QList<QString> ciChannels,
                            QList<QString> coChannels,
                            QList<QString> terminals);
    void disableGalvoScanType(bool ena);

signals:
    void galvoVoltChannelChanged(QString text);
    void cameraTrigChannelChanged(QString text);
    void cameraTrigReadyChannelChanged(QString text);
    void galvoPositionChanged(int val);
    void galvoTrigDelayChanged(int val);
    void galvoClockSourceChanged(QString text);
    void galvoScanType(int val);
    void galvoLowerLimitChanged(int val);
    void galvoUpperLimitChanged(int val);

private:
    Ui::CADaqWindow *ui;
    QSettings settings;
};

#endif // CADAQWINDOW_H
