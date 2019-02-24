#ifndef CAFILE_H
#define CAFILE_H

#include <QObject>
#include <QThread>
#include <stdint.h>
#include "cautilities.h"
#include "H5Cpp.h"

class CAFile; //forward declaration

///
/// \brief The CAFileThread class
///
class CAFileThread : public QThread
{
    Q_OBJECT
public:
    explicit CAFileThread(double * voxelSize, const char * detectorName);
    ~CAFileThread();
    void writeData(CAImage image, CACoordinate pos);
    void createAttribute(const char *name, double * value, int len);
    bool isValid();

protected:
   CAFile * file;

signals:
   void doWriteToFile(CAImage, CACoordinate);
   void doCreateAttribute(const char *, double *, int len);

};

using namespace H5;

class CAFile : public QObject
{
    Q_OBJECT
public:
    explicit CAFile(QObject *parent, double * voxelSize, const char * detectorName);
    ~CAFile();

    bool isValid();

protected:
    void init(double * voxelSize, const char * detectorName);
    void createDataSet(const Group & parentGroup,
                       const int nameIndex, hsize_t * chunk_dims);
private:
    bool initialized;
    char fullFileName[256];

    int dataSetIndex;
    int groupIndex[2];

    H5File * mFile;
    Group mGroup;
    Group mSubGroup;
    DataType mDataType;
    DataSpace * mDataSpace;
    DataSet mDataSet;

    CACoordinate currentPosition;

public slots:
    void writeData(CAImage image, CACoordinate pos);
    void createAttribute(const char* name,double* pvalue,int len);

};

#endif // CAFILE_H
