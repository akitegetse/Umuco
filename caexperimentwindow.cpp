#include "caexperimentwindow.h"
#include "ui_caexperimentwindow.h"

///
/// \brief CAExperimentWindow::CAExperimentWindow
/// \param parent
///
CAExperimentWindow::CAExperimentWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CAExperimentWindow),
    stacksRange(CARange<double>()),
    tilesWidthRange(CARange<double>()),
    tilesHeightRange(CARange<double>()),
    tilesOverlap(0.0)
{
    ui->setupUi(this);
    connect(ui->stackFirstPushButton, SIGNAL(clicked()), this, SLOT(stackFirstPushButtonClicked()));
    connect(ui->stackLastPushButton, SIGNAL(clicked()), this, SLOT(stackLastPushButtonClicked()));
    connect(ui->stackIntervalEdit, SIGNAL(returnPressed()), this, SLOT(stackIntervalEditTextChanged()));

    connect(ui->streamDurationEdit, SIGNAL(returnPressed()), this, SLOT(streamDurationEditTextChanged()));

    connect(ui->stacksAndTilesCheckBox, SIGNAL(toggled(bool)), this, SLOT(stacksAndTilesCheckBoxToggled(bool)));

    connect(ui->overlapEdit, SIGNAL(returnPressed()), this, SLOT(tilesOverlapEditTextChanged()));
    connect(ui->widthLoPushButton, SIGNAL(clicked()), this, SLOT(widthLoPushButtonClicked()));
    connect(ui->widthHiPushButton, SIGNAL(clicked()), this, SLOT(widthHiPushButtonClicked()));
    connect(ui->heightLoPushButton, SIGNAL(clicked()), this, SLOT(heightLoPushButtonClicked()));
    connect(ui->heightHiPushButton, SIGNAL(clicked()), this, SLOT(heightHiPushButtonClicked()));

    ui->stacksBox->setVisible(false);
    ui->tilesBox->setVisible(false);
}

///
/// \brief CAExperimentWindow::~CAExperimentWindow
///
CAExperimentWindow::~CAExperimentWindow()
{
    delete ui;
}

void CAExperimentWindow::initialize()
{

    int val = settings.value("experiment/streamDuration").toInt();
    QString newText("");
    newText.append(QString("%1 s").arg(val,0));
    ui->streamDurationEdit->setText(newText);
    emit(streamSetDuration(val));

    double dval = settings.value("experiment/stacksFirst").toDouble();
    stacksRange.setMin(dval);

    dval = settings.value("experiment/stacksLast").toDouble();
    stacksRange.setMax(dval);

    dval = settings.value("experiment/stackInterval").toDouble();
    stacksRange.step = dval;
    newText = QString("");
    newText.append(QString("%1 um").arg(dval,0, 'f', 3));
    ui->stackIntervalEdit->setText(newText);

    val = settings.value("experiment/tilesWidthLo").toInt();
    tilesWidthRange.setMin(val);

    val = settings.value("experiment/tilesWidthHi").toInt();
    tilesWidthRange.setMax(val);

    val = settings.value("experiment/tilesHeightLo").toInt();
    tilesHeightRange.setMin(val);

    val = settings.value("experiment/tilesHeightHi").toInt();
    tilesHeightRange.setMax(val);

    tilesOverlap = settings.value("experiment/tilesOverlap").toDouble();
    int utilFrac = tilesOverlap * 100;
    newText = QString("");
    newText.append(QString("%1 %").arg(utilFrac,0));
    ui->overlapEdit->setText(newText);


    val = settings.value("experiment/stacksAndTilesEnable").toBool();
    ui->stacksAndTilesCheckBox->setChecked(val);

    updateStacksAndTilesDisplay();

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::stacksAndTilesCheckBoxToggled
/// \param checked
///
void CAExperimentWindow::stacksAndTilesCheckBoxToggled(bool checked)
{
    ui->stacksBox->setVisible(checked);
    ui->tilesBox->setVisible(checked);
    settings.setValue("experiment/stacksAndTilesEnable", checked);
    ui->streamBox->setVisible(!checked);

    if(!checked) {
        stacksRange.setMin(stacksRange.value);
        stacksRange.setMax(stacksRange.value);
        tilesWidthRange.setMin(tilesWidthRange.value);
        tilesWidthRange.setMax(tilesWidthRange.value);
        tilesHeightRange.setMin(tilesHeightRange.value);
        tilesHeightRange.setMax(tilesHeightRange.value);

        saveStacksAndTilesConfig();
        updateStacksAndTilesDisplay();

        emit(stacksAndTilesDisabled());
    }
}

///
/// \brief CAExperimentWindow::streamDurationEditTextChanged
///
void CAExperimentWindow::streamDurationEditTextChanged()
{
    QString text =  ui->streamDurationEdit->text();
    QStringList list = text.split(" ");
    int nbrStr = list.count();
    int value;

    bool formatNeeded = true;
    if(nbrStr > 0){
        value = list[0].toDouble();
        if (nbrStr > 1){
            QString units = list[1];
            if (units.compare("s", Qt::CaseInsensitive) == 0){
                value *= 1;
                formatNeeded = false;
            } else if (units.compare("min", Qt::CaseInsensitive) == 0){
                value *= 60;
            } else if (units.compare("h", Qt::CaseInsensitive) == 0){
                value *= 3600;
            }
        }
    } else {
        value = 0;
    }
    settings.setValue("experiment/streamDuration", value);

    if(formatNeeded){
        QString newText("");
        newText.append(QString("%1 s").arg(value,0));
        ui->streamDurationEdit->setText(newText);
    }

    emit(streamSetDuration(value));
}

///
/// \brief CAExperimentWindow::stackIntervalEditTextChanged
///
void CAExperimentWindow::stackIntervalEditTextChanged()
{
    QString text =  ui->stackIntervalEdit->text();
    QStringList list = text.split(" ");
    int nbrStr = list.count();
    double value;

    if(nbrStr > 0){
        value = list[0].toDouble();
    } else {
        value = 0;
    }
    settings.setValue("experiment/stackInterval", value);

    QString newText("");
    newText.append(QString("%1 um").arg(value,0, 'f', 3));
    ui->stackIntervalEdit->setText(newText);

    stacksRange.setStep(value);

    text = QString("%1").arg(stacksRange.steps(),5,10,QLatin1Char(' '));
    ui->stackSlicesValue->setText(text);

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::stackFirstPushButtonClicked
///
void CAExperimentWindow::stackFirstPushButtonClicked()
{
    stacksRange.setMin(stacksRange.value);

    saveStacksAndTilesConfig();
    updateStacksAndTilesDisplay();

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::stackLastPushButtonClicked
///
void CAExperimentWindow::stackLastPushButtonClicked()
{
    stacksRange.setMax(stacksRange.value);

    saveStacksAndTilesConfig();
    updateStacksAndTilesDisplay();

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::tilesOverlapEditTextChanged
///
void CAExperimentWindow::tilesOverlapEditTextChanged()
{
    QString text =  ui->overlapEdit->text();
    QStringList list = text.split(" ");
    int nbrStr = list.count();
    int value;

    if(nbrStr > 0){
        value = list[0].toInt();
    } else {
        value = (int) tilesOverlap * 100;
    }

    tilesOverlap = value / 100.0;
    settings.setValue("experiment/tilesOverlap", tilesOverlap);

    QString newText("");
    newText.append(QString("%1 %").arg(value,0));
    ui->overlapEdit->setText(newText);

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::widthLoPushButtonClicked
///
void CAExperimentWindow::widthLoPushButtonClicked()
{
    tilesWidthRange.setMin(tilesWidthRange.value);

    saveStacksAndTilesConfig();
    updateStacksAndTilesDisplay();

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::widthHiPushButtonClicked
///
void CAExperimentWindow::widthHiPushButtonClicked()
{
    tilesWidthRange.setMax(tilesWidthRange.value);

    saveStacksAndTilesConfig();
    updateStacksAndTilesDisplay();

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::heightLoPushButtonClicked
///
void CAExperimentWindow::heightLoPushButtonClicked()
{
    tilesHeightRange.setMin(tilesHeightRange.value);

    saveStacksAndTilesConfig();
    updateStacksAndTilesDisplay();

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::heightHiPushButtonClicked
///
void CAExperimentWindow::heightHiPushButtonClicked()
{
    tilesHeightRange.setMax(tilesHeightRange.value);

    saveStacksAndTilesConfig();
    updateStacksAndTilesDisplay();

    emit(stacksAndTilesSetRange(tilesWidthRange, tilesHeightRange, tilesOverlap, stacksRange));
}

///
/// \brief CAExperimentWindow::saveStacksAndTilesConfig
///
void CAExperimentWindow::saveStacksAndTilesConfig()

{
    if(tilesHeightRange.steps() == 1){
        tilesHeightRange.setMin(tilesHeightRange.value);
        tilesHeightRange.setMax(tilesHeightRange.value);
    }

    if(tilesWidthRange.steps() == 1){
        tilesWidthRange.setMin(tilesWidthRange.value);
        tilesWidthRange.setMax(tilesWidthRange.value);
    }

    settings.setValue("experiment/stacksFirst", stacksRange.min);
    settings.setValue("experiment/stacksLast", stacksRange.max);

    settings.setValue("experiment/tilesWidthLo", tilesWidthRange.min);
    settings.setValue("experiment/tilesWidthHi", tilesWidthRange.max);

    settings.setValue("experiment/tilesHeightLo", tilesHeightRange.min);
    settings.setValue("experiment/tilesHeightHi", tilesHeightRange.max);
}

///
/// \brief CAExperimentWindow::updateStacksAndTilesDisplay
///
void CAExperimentWindow::updateStacksAndTilesDisplay()
{
    QString text = QString("%1").arg(stacksRange.min,8,'f',3,QLatin1Char(' '));
    ui->stackFirstValue->setText(text);
    text = QString("%1").arg(stacksRange.max,8,'f',3,QLatin1Char(' '));
    ui->stackLastValue->setText(text);
    text = QString("%1").arg(stacksRange.steps(),5,10,QLatin1Char(' '));
    ui->stackSlicesValue->setText(text);

    text = QString("%1").arg(tilesHeightRange.min,8,'f',3,QLatin1Char(' '));
    ui->heightLoValue->setText(text);
    text = QString("%1").arg(tilesHeightRange.max,8,'f',3,QLatin1Char(' '));
    ui->heightHiValue->setText(text);
    text = QString("%1").arg(tilesHeightRange.steps(),5,10,QLatin1Char(' '));
    ui->heightTilesValue->setText(text);

    text = QString("%1").arg(tilesWidthRange.min,8,'f',3,QLatin1Char(' '));
    ui->widthLoValue->setText(text);
    text = QString("%1").arg(tilesWidthRange.max,8,'f',3,QLatin1Char(' '));
    ui->widthHiValue->setText(text);
    text = QString("%1").arg(tilesWidthRange.steps(),5,10,QLatin1Char(' '));
    ui->widthTilesValue->setText(text);
}

///
/// \brief CAExperimentWindow::setTileSize
/// \param size
///
void CAExperimentWindow::setTileSize(QSize size)
{
    tilesWidthRange.setStep(size.width());
    tilesHeightRange.setStep(size.height());

    int val = tilesWidthRange.steps();
    QString text = QString("%1").arg(val,5,10,QLatin1Char(' '));
    ui->widthTilesValue->setText(text);

    val = tilesHeightRange.steps();
    text = QString("%1").arg(val,5,10,QLatin1Char(' '));
    ui->heightTilesValue->setText(text);
}

///
/// \brief CAExperimentWindow::displayPosition
/// \param x
/// \param y
/// \param z
///
void CAExperimentWindow::displayPosition(double x, double y, double z)
{
    tilesWidthRange.value = y;
    tilesHeightRange.value = z;
    stacksRange.value = x;

    QString positionString = QString("(%1, %2, %3)")
            .arg(y,8,'f',3,QLatin1Char(' '))
            .arg(z,8,'f',3,QLatin1Char(' '))
            .arg(x,8,'f',3,QLatin1Char(' '));
    ui->sHCurrentPositionDisplay->setText(positionString);
}
