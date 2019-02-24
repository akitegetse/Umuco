#ifndef CAUMUCOMAIN_H
#define CAUMUCOMAIN_H

#include <QObject>
#include <QSettings>
#include <QCloseEvent>

#include "caumucomainwindow.h"
#include "cacamera.h"
#include "castage.h"
#include "cadaq.h"
#include "caexperiment.h"
#include "caalignmenttool.h"

class CAUmucoMain : public QObject
{
    Q_OBJECT
public:
    explicit CAUmucoMain(QApplication *parent = 0);
    ~CAUmucoMain();
    void openWindow();

    CAUmucoMainWindow * window;
    CAStage * stage;
    CACamera * camera;
    CADaq * daq;
    CAExperiment * experiment;
    CAAlignmentTool * alignTool;

signals:
    void experimentCreated();
    void daqDeviceNameChanged(QString);

private slots:
    void alignmentTool();

public slots:
    void stageSelectionChanged(int index);
    void cameraSelectionChanged(int index);
    void createExperiment();
    void cameraConnection(bool status);
};

#endif // CAUMUCOMAIN_H
