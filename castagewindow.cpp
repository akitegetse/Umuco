#include "castagewindow.h"
#include "ui_CAStageWindow.h"

///
/// \brief CAStageWindow::CAStageWindow
/// \param parent
///
CAStageWindow::CAStageWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CAStageWindow)
{
    ui->setupUi(this);

    connect(ui->xValueTE, SIGNAL(valueChanged(int)),
            this, SLOT(targetPositionChanged(int)));
    connect(ui->yValueTE, SIGNAL(valueChanged(int)),
            this, SLOT(targetPositionChanged(int)));
    connect(ui->zValueTE, SIGNAL(valueChanged(int)),
            this, SLOT(targetPositionChanged(int)));

}

///
/// \brief CAStageWindow::~CAStageWindow
///
CAStageWindow::~CAStageWindow()
{
    delete ui;
}

///
/// \brief CAStageWindow::targetPositionChanged
/// \param val
///
void CAStageWindow::targetPositionChanged(int val)
{
    int x = ui->xValueTE->value();
    int y = ui->yValueTE->value();
    int z = ui->zValueTE->value();

    emit(requestMoveTo((double)x, (double)y, (double)z));
    if(val < 0) qDebug("Index error");
}

///
/// \brief CAStageWindow::positionUpdated
/// \param x
/// \param y
/// \param z
///
void CAStageWindow::positionUpdated(double x, double y, double z)
{
    ui->xValueTE->setValue((int)x);
    ui->yValueTE->setValue((int)y);
    ui->zValueTE->setValue((int)z);
}

