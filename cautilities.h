#ifndef CATYPES
#define CATYPES

#include <stdint.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define CARect(x, y, width, height) \
    cv::Rect(x,y,width, height)
///
/// \brief The CAImage class
///
class CAImage : public cv::Mat
{
public:
    CAImage():
        cv::Mat(512,512,CV_8UC1)
    {
    }

    CAImage(int rows, int cols, int bps, int depth):
        cv::Mat(rows, cols, CV_MAKETYPE(
                    (bps==1)?
                        CV_8U:(bps==2)?
                            CV_16U:(bps==4)?
                                CV_32F:(bps==8)?
                                    CV_64F:CV_8U,
                    depth))
    {
        switch(bps){
        case 1:
            break;
        case 2:
            break;
        case 4:
            break;
        case 8:
            break;
        default:
            /*std::invalid_argument( "bytesPerSample: valid values are 1(unsigned char),"
                                   "2(unsigned short), 4(float) or 8(double");*/
            break;
        }
    }

    CAImage(int rows, int cols, int type):
        cv::Mat(rows, cols, type)
    {
    }


    CAImage(const CAImage & other):
        cv::Mat(other, cv::Range::all(), cv::Range::all())
    {
    }

    CAImage(const cv::Mat & other):
        cv::Mat(other, cv::Range::all(), cv::Range::all())
    {
    }

    void contrastAdjust(CAImage & dst, int min, int max)
    {
        double alpha, beta;

        alpha = 65535.0/(max-min);
        beta = -alpha * min;
        this->convertTo(dst, -1, alpha, beta);
    }

    unsigned long byteCount() { return (unsigned long) (total() * elemSize());}
};

/// Cartesian coordinates
typedef struct CACoordinate{
    int x;
    int y;
    int z;
    // default + parameterized constructor
    CACoordinate(int x=0, int y=0, int z=0):
        x(x), y(y), z(z)
    {
    }
}CACoordinate;


template <typename T>
struct  CARange {
    T min;
    T max;
    T step;
    T value;

    // default + parameterized constructor
    CARange(T min=0, T max=0, T step=1, T value=0):
        min(min), max(max), step(step), value(value)
    {
    }

    //
    void setMin(T val)
    {
        if(val<= max){
            min = val;
        } else {
            min = val;
            max = val;
        }
    }

    void setMax(T val)
    {
        if(val >= min){
            max = val;
        } else {
            min = val;
            max = val;
        }
    }

    void setStep(T val)
    {
        step = val;
    }

    // assignment operator modifies object, therefore non-const
    CARange<T>& operator=(const CARange<T>& a)
    {
        min = a.min;
        max = a.max;
        step = a.step;
        value = a.value;

        return *this;
    }

    // increment
    CARange<T>& operator++()
    {
        value += step;

        return *this;
    }

    // decrement
    CARange<T>& operator--()
    {
        value -= step;

        return *this;
    }

    CARange<T>& setToMin()
    {
        value = min;

        return *this;
    }

    CARange<T>& setToMax()
    {
        value = max;

        return *this;
    }

    //max attained
    bool isMax()
    {
        long a = (long) round(value * 100);
        long b = (long) round(max * 100);
        return (a >= b);
    }

    //min attained
    bool isMin()
    {
        long a = (long) round(value * 100);
        long b = (long) round(min * 100);
        return (a <= b);
    }

    //Number of steps
    int steps()
    {
        return step ? (int)(1.5 + 1.0*(max - min)/step) : 1;
    }

    //Number of steps alreadyDone
    double stepsDone()
    {
        return step ? double(value - min)/step : 1.0;
    }

    ///
    /// \brief progress
    /// \return
    ///
    double progress()
    {
        double val;
        if(max == min) {
            val = 1.0;
        } else {
            double stepsDone = step ? (value - min)/step : 1;
            int stepsReq = step ? (int)(1.5 + 1.0*(max - min)/step) : 1;
            val = stepsDone/stepsReq;
        }

        return val;
    }
};

typedef struct CARoi{
    int x;
    int y;
    int width;
    int height;

    CARoi(int x = 0, int y = 0, int width = 1, int height = 1):
        x(x), y(y), width(width), height(height)
    {
    }

    CARoi& operator=(const CARoi& a)
    {
        x = a.x;
        y = a.y;
        width = a.width;
        height = a.height;

        return *this;
    }
} CARoi;

enum CAAxisOrder {
    ZYX = 0,
    ZXY = 1,
    YXZ = 2,
    YZX = 3,
    XZY = 4,
    XYZ = 5
};

class CAStacksAndTiles
{
public:
    enum CAAxisOrder axesOrder;
    CARange <double> x;
    CARange <double> y;
    CARange <double> z;

    // default + parameterized constructor
    CAStacksAndTiles():
        axesOrder(XYZ),
        x(CARange<double>()),
        y(CARange<double>()),
        z(CARange<double>())
    {
        x.step = 1;
        y.step = 1;
        z.step = 1;
    }

    CAStacksAndTiles(CARange <double> x, CARange <double> y, CARange <double> z):
        axesOrder(XYZ), x(x), y(y), z(z)
    {
    }

    // assignment operator modifies object, therefore non-const
    CAStacksAndTiles& operator=(const CAStacksAndTiles& a)
    {
        x = a.x;
        y = a.y;
        z = a.z;

        return *this;
    }

     void increment()
     {
         //will need to reimplement it if you choose another axis order
         if(!x.isMax()){
             ++x;
         }
         else if(!y.isMax()){
             x.setToMin();
             ++y;
         } else if(!z.isMax()){
             x.setToMin();
             y.setToMin();
             ++z;
         }
     }

     // decrement
     void decrement()
     {
         //will need to reimplement it if you choose another axis order
         if(!x.isMin()){
             --x;
         }
         else if(!y.isMin()){
             x.setToMax();
             --y;
         } else {
             x.setToMax();
             y.setToMax();
             --z;
         }
     }

    //
    CACoordinate getValues()
    {
        return CACoordinate(x.value, y.value, z.value);
    }

    //
    void setToMin()
    {
        x.setToMin();
        y.setToMin();
        z.setToMin();
    }

    void setToMax()
    {
        x.setToMax();
        y.setToMax();
        z.setToMax();
    }

    //
    bool isComplete()
    {
        return x.isMax() && y.isMax() && z.isMax();
    }

    double progress()
    {
        //will need to reimplement it if you choose another axis order
        return (x.stepsDone() + x.steps()* y.stepsDone() + x.steps()* y.steps()* z.stepsDone()) * 1.0/(x.steps() * y.steps()*z.steps());
    }
};

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif
inline void CASleep(int ms)
{
    Q_ASSERT(ms > 0);

#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

#endif // CATYPES

