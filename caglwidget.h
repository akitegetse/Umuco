#ifndef CAGLWIDGET_H
#define CAGLWIDGET_H

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTime>
#include "cautilities.h"

class CAGLWidget : public QOpenGLWidget , protected QOpenGLFunctions
{
    Q_OBJECT

public:
    CAGLWidget(QWidget *parent = 0);
    ~CAGLWidget();

    void setImage(CAImage *im);
    void setSubImage(int x, int y, CAImage *im);
    CAImage getImage(void);
    void setImageSize(int col, int row);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setImContrast(double val);

    void addGrid();

protected:
    CAImage * image;

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void drawSelection(void);

private:
    GLuint texture;
    QPoint roiFirstCorner;
    QPoint roiOppositeCorner;
    QRect zoom;
    QRect selection;
    QTime myTime;
    int lastDisplayTime;
    double imContrastMax;
    bool topHat;

public slots:
    void enableTopHat(bool ena);
};

#endif // CAGLWIDGET_H
