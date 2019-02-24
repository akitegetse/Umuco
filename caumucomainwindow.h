#ifndef CAUMUCOMAINWINDOW_H
#define CAUMUCOMAINWINDOW_H

#include <QMainWindow>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QSettings>
#include <QResizeEvent>
#include "cautilities.h"

namespace Ui {
class CAUmucoMainWindow;
}

class CAUmucoMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CAUmucoMainWindow(QWidget *parent = 0);
    ~CAUmucoMainWindow();
    void uiUpdate();

private:
    Ui::CAUmucoMainWindow *ui;
    void createActions();
    void resizeEvent(QResizeEvent *);
    QSize sizeHint() const;

protected:
    void closeEvent (QCloseEvent *event);

signals:
    //devices configuration
    void cameraDisplayRequest();
    void stageDisplayRequest();
    void daqDisplayRequest();
    void cameraSelectionChanged(int index);
    void stageSelectionChanged(int index);
    void daqDeviceNameChanged(QString);
    void createExperiment();
    //acquisition
    void startPreview();
    void stopPreview();
    void snapImage();
    void startExperiment();
    void stopExperiment();
    void alignmentTool();
    void acqScanSynchronize(bool);
    void closeRequest();

public slots:
    void displayImage(CAImage buf);
    void displayTile(CAImage image, CARange<int> xPosInfo, CARange<int> yPosInfo);
    void stageState(bool state);
    void experimentComplete();
    void experimentCreated();
    void connectionCamera(bool status);

private slots:
   void startStopPBClicked();
   void previewPBClicked();
   void savePBClicked();
   void progressBar(double val);
   void contrastAdjustSliderValueChanged(int value);
};

#endif // CAUmucoMainWindow_H
