#ifndef CAEXPERIMENTWINDOW_H
#define CAEXPERIMENTWINDOW_H

#include <QWidget>
#include <QSettings>
#include "cautilities.h"

namespace Ui {
class CAExperimentWindow;
}

class CAExperimentWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CAExperimentWindow(QWidget *parent = 0);
    ~CAExperimentWindow();
    void initialize();
    void setTileSize(QSize size);

signals:
    void streamSetDuration(int val);
    void stacksAndTilesDisabled();
    void stacksAndTilesSetRange(CARange<double> rangeX,
                                CARange<double> rangeY,
                                double tilesOverlap,
                                CARange<double> rangeZ);

private slots:
    void stacksAndTilesCheckBoxToggled(bool checked);
    void streamDurationEditTextChanged();
    void stackIntervalEditTextChanged();
    void stackFirstPushButtonClicked();
    void stackLastPushButtonClicked();

    void tilesOverlapEditTextChanged();
    void widthLoPushButtonClicked();
    void widthHiPushButtonClicked();
    void heightLoPushButtonClicked();
    void heightHiPushButtonClicked();

    void displayPosition(double x, double y, double z);

private:
    Ui::CAExperimentWindow *ui;

    CARange<double> stacksRange;
    CARange<double> tilesWidthRange;
    CARange<double> tilesHeightRange;
    double tilesOverlap;

    QSettings settings;

    void updateStacksAndTilesDisplay();
    void saveStacksAndTilesConfig();
};

#endif // CAEXPERIMENTWINDOW_H
