#include "caglwidget.h"
#ifdef __APPLE__
#include <glu.h>
#else
#include <GL/glu.h>
#endif
#include <QMouseEvent>
#include <QPainter>
#include <math.h>

///
/// \brief CAGLWidget::CAGLWidget
/// \param parent
///
CAGLWidget::CAGLWidget(QWidget *parent):
    QOpenGLWidget(parent),
    QOpenGLFunctions(),
    image(new CAImage(2048,2048,2,1)),
    myTime(QTime()),
    imContrastMax(65535.0),
    topHat(false)
{
    myTime.start();
}

///
/// \brief CAGLWidget::~CAGLWidget
///
CAGLWidget::~CAGLWidget()
{
    delete image;
}

///
/// \brief CAGLWidget::enableTopHat
/// \param ena
///
void CAGLWidget::enableTopHat(bool ena)
{
    topHat = ena;
}

///
/// \brief CAGLWidget::setImage
/// \param im
///
void CAGLWidget::setImage(CAImage * im)
{
    cv::MatSize sizes = im->size;
    if(im->rows != image->rows || im->cols != image->cols){
        //image->resize(im->cols, im->rows);
        image->create(im->dims, im->size, im->type());
    }

    if(topHat){
        int morph_size = 25;
        cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, //MORPH_RECT,
                                                    cv::Size( 2*morph_size + 1, 2*morph_size+1 ),
                                                    cv::Point( morph_size, morph_size ) );
        cv::morphologyEx(*im,*im,cv::MORPH_TOPHAT,element, cv::Point(-1,-1), 1 );
    }
    im->contrastAdjust(*image, 0, imContrastMax);

    double a = 1.0*sizes[0]/sizes[1];
    int w = width();
    int h = height();
    if (a > 1) h = w*a; else w = h/a;
    this->resize(QSize(w, h));

    update();
}

///
/// \brief CAGLWidget::setSubImage
/// \param x
/// \param y
/// \param im
///
void CAGLWidget::setSubImage(int x, int y, CAImage *im)
{
    cv::Mat subIm = image->colRange(x,im->cols).rowRange(y,im->rows);

    im->copyTo(subIm);
    update();
}

///
/// \brief CAGLWidget::getImage
/// \return
///
CAImage CAGLWidget::getImage(void)
{
    return *image;
}

///
/// \brief CAGLWidget::setImageSize
/// \param col
/// \param row
///
void CAGLWidget::setImageSize(int col, int row)
{
    delete image;
    image = new CAImage(col,row,2,1);
}

///
/// \brief CAGLWidget::setImContrast
/// \param val
///
void CAGLWidget::setImContrast(double val)
{
    imContrastMax = val;
}

///
/// \brief CAGLWidget::minimumSizeHint
/// \return
///
QSize CAGLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

///
/// \brief CAGLWidget::sizeHint
/// \return
///
QSize CAGLWidget::sizeHint() const
{
    return QSize(600, 600);
}

///
/// \brief CAGLWidget::initializeGL
///
void CAGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    zoom = QRect(0,0,width(), height());
    selection = QRect(0,0,0,0);

    glClearColor (0.0,0.0,0.0,0.0);
    glClearDepth(1);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,this->width(),0,this->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    lastDisplayTime = myTime.elapsed();
}

///
/// \brief CAGLWidget::paintGL
///
void CAGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(zoom.left(), zoom.right(), zoom.top(), zoom.bottom());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    cv::MatSize sizes = image->size;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, sizes[0], sizes[1],
            0, GL_LUMINANCE, GL_UNSIGNED_SHORT, image->data );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBegin(GL_QUADS);
    glColor3f(0.3f,1.0f, 0.0f);
    glTexCoord2i(0,0); glVertex2i(0,this->height());
    glTexCoord2i(0,1); glVertex2i(0,0);
    glTexCoord2i(1,1); glVertex2i(this->width(),0);
    glTexCoord2i(1,0); glVertex2i(this->width(),this->height());
    glEnd();

    addGrid();
    drawSelection();

    glFlush();
    glDeleteTextures(1,&texture);

    int currentTime = myTime.elapsed();
    int frameRate = round(1.0E3/(currentTime - lastDisplayTime));
    lastDisplayTime = currentTime;

    QPainter painter(this);
    painter.beginNativePainting();
    painter.setPen(Qt::red);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRectF(10,10,40,40), QString("%1fps").arg(frameRate));
    painter.endNativePainting();
}

///
/// \brief CAGLWidget::resizeGL
/// \param width
/// \param height
///
void CAGLWidget::resizeGL(int width, int height)
{
    QRect geom = geometry();
    setGeometry(geom.x(),geom.y(), width, height);
    zoom = QRect(0,0,width,height);
}

///
/// \brief CAGLWidget::mousePressEvent
/// \param event
///
void CAGLWidget::mousePressEvent(QMouseEvent *event)
{
    roiFirstCorner = event->pos();
}

///
/// \brief CAGLWidget::mouseReleaseEvent
/// \param event
///
void CAGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    roiOppositeCorner = event->pos();
    if(selection.width()>7 && selection.height()> 7){
        double ah = 1.0 * zoom.height()/height();
        double aw = 1.0 * zoom.width()/width();
        int x1 = zoom.x() + selection.x()*aw;
        int y1 = zoom.y() + selection.y()* ah;
        int h = selection.height() * ah;
        int w = selection.width() * aw;
        zoom = QRect(x1, y1,w,h);
        selection = QRect(0,0,0,0);
        update();
    }
}

void CAGLWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    zoom = QRect(0,0,width(),height());
    update();
}

void CAGLWidget::mouseMoveEvent(QMouseEvent * event)
{
    QPoint currentPos = event->pos();
    int x1 = qMin(roiFirstCorner.x(), currentPos.x());
    int y1 = qMin(roiFirstCorner.y(), currentPos.y());
    int x2 = qMax(roiFirstCorner.x(), currentPos.x());
    int y2 = qMax(roiFirstCorner.y(), currentPos.y());
    selection = QRect(x1, y1, x2-x1, y2-y1);

    update();
}

///
/// \brief CAGLWidget::drawSelection
///
void CAGLWidget::drawSelection(void)
{
    if(selection.width() != 0 || selection.width() != 0){
        double ah = 1.0 * zoom.height()/height();
        double aw = 1.0 * zoom.width()/width();
        int x1 = zoom.left() + selection.x()*aw;
        int y1 = zoom.bottom() - selection.y()* ah;
        int h = selection.height() * ah;
        int w = selection.width() * aw;

        int x2 = x1 + w;
        int y2 = y1 - h;

        glBegin(GL_LINES);
        glColor3f(1.0f,0.0f,0.0f);
        glVertex2f(x1, y1); glVertex2f(x2, y1);
        glVertex2f(x1, y1); glVertex2f(x1, y2);
        glVertex2f(x1, y2); glVertex2f(x2, y2);
        glVertex2f(x2, y2); glVertex2f(x2, y1);
        glEnd();
    }
}

///
/// \brief CAGLWidget::addGrid
///
void CAGLWidget::addGrid()
{
    glBegin(GL_LINES);
    glColor3f(1.0f,1.0f,1.0f);

    int xGrid = this->width()/4;
    int yGrid = this->height()/4;
    int x = xGrid;
    int y = yGrid;

    while(x<this->width()){
        glVertex2f(x, 0);
        glVertex2f(x, this->height());
        x += xGrid;
    }
    while(y < this->height()){
        glVertex2f(0, y);
        glVertex2f(this->width(), y);
        y += yGrid;
    }
    glEnd();
}
