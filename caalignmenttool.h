#ifndef CAALIGNMENTTOOL_H
#define CAALIGNMENTTOOL_H

#include <QWidget>
#include "qCustomPlot/qcustomplot.h"
#include "cautilities.h"
#include <QMap>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

namespace Ui {
class CAAlignmentTool;
}

class CAAlignmentTool : public QWidget
{
    Q_OBJECT

public:
    explicit CAAlignmentTool(QSize pxSize, QWidget *parent = 0);
    ~CAAlignmentTool();
    QSize pixelSize;

private:
    Ui::CAAlignmentTool *ui;
    QMenuBar menuBar;
    QMenu * fileMenu;
    QAction * saveAction;

    void focusLongitudinalProfile(CAImage im);
    QFuture<void> focusProfileFuture;

    void findProfile(CAImage im);
    QFuture<void> profileFuture;

    void peakLongitudinalProfile(CAImage im);
    QFuture<void> peakProfileFuture;

    int fullWidthAtHalfMaximum(cv::Mat y);

private slots:
    void plotSignal(QCustomPlot * plot,QVector<double> xAxis, QVector<double> yAxis,
                    QString xLabel, QString yLabel, QString legend);
    void saveScreenShot();

public slots:
    void besselProfiles(CAImage image);
    void setPixelSize(QSize pxSize);
signals:
    void plotRequest(QCustomPlot * plot,QVector<double> xAxis, QVector<double> yAxis,
                     QString xLabel, QString yLabel, QString legend);
};

#endif // CAAlignmentTool_H
