#ifndef CAStageWindow_H
#define CAStageWindow_H

#include <QWidget>
#include <stdint.h>

namespace Ui {
class CAStageWindow;
}

class CAStageWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CAStageWindow(QWidget *parent = 0);
    ~CAStageWindow();

private slots:
    void targetPositionChanged(int val);

public slots:
    void positionUpdated(double x, double y, double z);

signals:
    void requestMoveTo(double x, double y, double z);

private:
    Ui::CAStageWindow *ui;
};

#endif // CAStageWindow_H
