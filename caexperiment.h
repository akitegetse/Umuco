#ifndef CAEXPERIEMENT_H
#define CAEXPERIEMENT_H

#include <QObject>
#include "castage.h"
#include "cacamera.h"
#include "caexperimentwindow.h"
#include "cafile.h"
#include "cautilities.h"

class CAExperiment : public QObject
{
    Q_OBJECT
public:
    explicit CAExperiment(QObject *parent, CAStage * holder,
                          CACamera * imSensor);
    ~CAExperiment();

private:
    CAExperimentWindow * window;
    CAStage * stage;
    CACamera * camera;
    CAFileThread * expFile;
    CAStacksAndTiles * stacksTilesCfg;
    bool stacksEna;
    bool tilesEna;
    double tilesOverlap;
    QSize pixelSize;
    QTimer * streamTimer;
    QTimer *resumeTimer;

signals:
    void experimentCompleted();
    void experimentProgress(double val);
    void moveTo(double x, double y, double z);
    void tileWrittenToFile(CAImage, CARange<double>, CARange<double>);

public slots:
    void start();
    void stop();
    void resume();
    void stageConnected(bool state);
    void openWindow();
    void serviceDidMoveSH();
    void reemitMoveTo();
    void serviceFirstImage(CAImage im);
    void serviceImageAvailable(CAImage buffer);
    void streamEnable(int val);

    void stacksAndTilesDisable();
    void stacksAndTilesEnable(CARange<double> widthRange, CARange<double> heightRange,
                              double tileOverlap, CARange<double> stacksRange);
    void pixelSizeSet(QSize size);
};

#endif // CAEXPERIEMENT_H
