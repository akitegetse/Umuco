#include "cafile.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QEventLoop>

#define RANK 3

///
/// \brief CAFileThread::CAFileThread
/// \param voxelSize
/// \param detectorName
///
CAFileThread::CAFileThread(double * voxelSize, const char * detectorName):
    file(new CAFile(0, voxelSize, detectorName))
{
    qRegisterMetaType<CACoordinate >("CACoordinate");

    if(file->isValid()){ // if file successfully created
        //move the file to the new thread and start the thread
        file->moveToThread(this);
        start();

        connect(this, SIGNAL(doWriteToFile(CAImage, CACoordinate)),
                file, SLOT(writeData(CAImage,CACoordinate)),Qt::QueuedConnection);
        connect(this, SIGNAL(doCreateAttribute(const char*,double*,int)),
                file, SLOT(createAttribute(const char*,double*,int)));
    }
}

///
/// \brief CAFileThread::~CAFileThread
///
CAFileThread::~CAFileThread()
{
    if(isRunning()){
        quit();
        while (!isFinished());
    }
    delete file;
}

///
/// \brief CAFileThread::isValid
/// \return
///
bool CAFileThread::isValid()
{
    return file->isValid();
}

///
/// \brief CAFileThread::writeData
/// \param image
/// \param pos
///
void CAFileThread::writeData(CAImage image, CACoordinate pos)
{
    emit(doWriteToFile(image,pos));
}

///
/// \brief CAFileThread::createAttribute
/// \param name
/// \param value
/// \param len
///
void CAFileThread::createAttribute(const char *name, double * value, int len)
{
    emit(doCreateAttribute(name, value, len));
}

///
/// \brief CAFile::CAFile
/// \param parent
/// \param voxelSize
/// \param detectorName
///
CAFile::CAFile(QObject *parent, double * voxelSize, const char * detectorName):
    QObject(parent),
    initialized(false),
    mFile(NULL),
    mGroup(0),
    mDataType(0),
    mDataSpace(NULL),
    mDataSet(0),
    currentPosition(CACoordinate(-1,-1,-1))
{
    init(voxelSize, detectorName);
}

///
/// \brief CAFile::~CAFile
///
CAFile::~CAFile()
{
    if (isValid()){
        if(mDataSpace != NULL) delete mDataSpace;
        if(mFile != NULL) delete mFile;
    }
}

///
/// \brief CAFile::isValid
/// \return
///
bool CAFile::isValid()
{
    if(initialized) return (mFile->getId() != -1);
    else return false;
}

///
/// \brief CAFile::init
/// \param voxelSize
/// \param detectorName
///
void CAFile::init(double * voxelSize, const char * detectorName)
{
    //The directory where the file will be saved
    //The file name
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save File"),"", tr("hdf5 (*.h5 *.hdf5)"));

    if(fileName.compare("") != 0){
        sprintf(fullFileName, "%s",fileName.toLatin1().data());

        dataSetIndex = 0;
        groupIndex[0] = 0;
        groupIndex[1] = 0;

        /// Turn off the auto-printing when failure occurs so
        /// that we can handle the errors appropriately
        Exception::dontPrint();

        /// Create a new file using the default property lists.
        mFile = new H5File(fullFileName, H5F_ACC_TRUNC);

        if (mFile->getId() != 1){
            /// Specify the data type used by the data set
            mDataType = DataType(PredType::NATIVE_UINT16);

            /// Attributes
            ///
            /// user
            QString userName = qgetenv("USER");
            if (userName.isEmpty()){
                userName = qgetenv("USERNAME");
            }
            /// Create new string datatype for attribute
            StrType strDataType(PredType::C_S1, userName.length());
            /// Create new dataspace for attribute
            DataSpace attrDataSpace = DataSpace(H5S_SCALAR);
            /// Create a file attribute.
            Attribute attribute = mFile->createAttribute("user", strDataType,attrDataSpace);
            /// Write the attribute data
            attribute.write(strDataType, userName.toLatin1().data());
            //attribute.close();

            /// Detector
            /// Create new string datatype for attribute
            strDataType = StrType(PredType::C_S1, strlen(detectorName));
            /// Create new dataspace for attribute
            /*attrDataSpace = DataSpace(H5S_SCALAR);*/
            /// Create a file attribute.
            attribute = mFile->createAttribute("detector", strDataType,attrDataSpace);
            /// Write the attribute data
            attribute.write(strDataType, detectorName);
            //attribute.close();

            /// tile type
            /// Create new string datatype for attribute
            strDataType = StrType(PredType::C_S1, 13);
            /// Create new dataspace for attribute
            /*attrDataSpace = DataSpace(H5S_SCALAR);*/
            /// Create a file attribute.
            attribute = mFile->createAttribute("tilingType", strDataType,attrDataSpace);
            /// Write the attribute data
            attribute.write(strDataType, "raw");
            //attribute.close();

            /// Voxel
            const hsize_t voxelDim[3] = {1,3,1};
            /// Create new dataspace for attribute
            attrDataSpace = DataSpace(3, voxelDim);
            /// Create a file attribute.
            attribute = mFile->createAttribute("voxelSize", PredType::NATIVE_FLOAT,attrDataSpace);
            /// Write the attribute data
            float vsize[3] = {(float)voxelSize[2],(float)voxelSize[1],(float)voxelSize[0]};
            attribute.write(PredType::NATIVE_FLOAT, vsize);
            //attribute.close();

            initialized = true;
        } else {
            QWidget msgBox;
            QMessageBox::critical(&msgBox, tr("FILE CREATION"),
                                  tr("H5 creation failed!!\n"
                                     "May be you are trying to overwrite an open file."),
                                  QMessageBox::Ok);
        }
    }
}

///
/// \brief CAFile::createAttribute
/// \param name
/// \param pvalue
/// \param len
///
void CAFile::createAttribute(const char* name,double* pvalue,int len)
{
    const hsize_t attrDim[3] = {1,(hsize_t) len,1};
    /// Create new dataspace for attribute
    DataSpace attrDataSpace = DataSpace(3, attrDim);
    /// Create a file attribute.
    Attribute attribute = mFile->createAttribute(name, PredType::NATIVE_DOUBLE,attrDataSpace);
    /// Write the attribute data
    attribute.write(PredType::NATIVE_DOUBLE, pvalue);
}

///
/// \brief CAFile::writeData
/// \param image
///
void CAFile::writeData(CAImage image, CACoordinate pos)
{

    if (initialized){
        if(mDataSpace == NULL){

            const hsize_t dims[3] = {1, (unsigned) image.cols, (unsigned) image.rows};
            hsize_t maxdims[3] = {H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED};
            ///  Create a data space
            mDataSpace = new DataSpace(RANK, dims, maxdims);

            /// image size attribute
            const hsize_t thisDim[3] = {1, 3,1};
            /// Create new dataspace for attribute
            DataSpace attrDataSpace = DataSpace(3, thisDim);
            /// Create a file attribute.
            Attribute attribute = mFile->createAttribute("imageSize", PredType::NATIVE_INT,attrDataSpace);
            /// Write the attribute data
            int imsize[3] = {(int)dims[0],(int)dims[1],(int)dims[2]};
            attribute.write(PredType::NATIVE_INT, imsize);
            //attribute.close();
        }

        hsize_t imageDims[3] = {1, (unsigned) image.cols, (unsigned) image.rows};

        if(pos.y != currentPosition.y || pos.z != currentPosition.z){
            int currentIndex = groupIndex[1];
            if(pos.z != currentPosition.z){
                /// Create a new row group
                char groupName[32];
                sprintf(groupName, "R%02d", groupIndex[0]);
                mGroup = mFile->createGroup(groupName);
                currentIndex = 0;
                groupIndex[1] = 1;
                groupIndex[0] +=  1;
            } else groupIndex[1] += 1;

            /// Create a new sub group
            char subGroupName[32];
            sprintf(subGroupName, "C%02d", currentIndex);
            mSubGroup = mGroup.createGroup(subGroupName);

            dataSetIndex = 0;
            createDataSet(mSubGroup, dataSetIndex++, imageDims);

            /// Write the data to the data set
            mDataSet.write((const void*) image.data, mDataType);
        } else{
            hsize_t dims[3];
            DataSpace space = mDataSet.getSpace();
            space.getSimpleExtentDims(dims);

            hsize_t size[3];
            size[0] = dims[0]+ imageDims[0];
            size[1] = dims[1];
            size[2] = dims[2];
            if(dims[0] < 64){
                mDataSet.extend(size);

                /// Select a hyperslab in extended portion of dataset
                DataSpace filespace = mDataSet.getSpace();
                hsize_t offset[3];
                offset[0] = dims[0];
                offset[1] = 0;
                offset[2] = 0;
                filespace.selectHyperslab(H5S_SELECT_SET , imageDims, offset);

                //Define memory space
                DataSpace memspace = DataSpace(RANK, imageDims);

                //Write the data to the extended portion of dataset
                mDataSet.write((const void*) image.data, PredType::NATIVE_UINT16, memspace, filespace);
            } else {
                createDataSet(mSubGroup, dataSetIndex++, imageDims);
                /// Write the data to the data set
                mDataSet.write((const void*) image.data, mDataType);
            }
        }

        currentPosition = pos;
    }
}

void CAFile::createDataSet(const Group & parentGroup, const int nameIndex, hsize_t * chunk_dims)
{
    /// Modify dataset creation properties, i.e. enable chunking
    DSetCreatPropList  plist;
    plist.setChunk(RANK, chunk_dims);

    // Set ZLIB (DEFLATE) Compression using level 6.
    // uncomment this to enable compression
    //plist.setDeflate(6);
    //unsigned szipOptionsMask = H5_SZIP_NN_OPTION_MASK;
    //unsigned szipPixelsPerBlock = 16;
    //plist.setSzip(szipOptionsMask, szipPixelsPerBlock);

    /// Create the data set
    char dsetName[32];
    sprintf(dsetName, "D%04d",nameIndex*64);
    mDataSet = parentGroup.createDataSet(dsetName,mDataType,*mDataSpace, plist);
}
