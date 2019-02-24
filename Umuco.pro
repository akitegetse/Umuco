#-------------------------------------------------
#
# Project created by QtCreator 2015-07-23T11:22:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

mac {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas -Wno-deprecated-declarations \
                              -Wno-unused-local-typedef
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12
}

TARGET = Umuco
TEMPLATE = app

SOURCES += main.cpp\
    cadevicemanager.cpp \
    casuttermp285roe200.cpp\
    canidaq.cpp\
    cadaq.cpp\
    caumucomain.cpp \
    cafile.cpp \
    caexperiment.cpp \
    caexperimentwindow.cpp \
    caglwidget.cpp \
    caimagesimulator.cpp \
    caumucomainwindow.cpp \
    cadaqwindow.cpp\
    qcustomplot/qcustomplot.cpp \
    castage.cpp \
    cacamera.cpp \
    castagewindow.cpp \
    caalignmenttool.cpp

HEADERS  += \
    cadevicemanager.h \
    casuttermp285roe200.h\
    canidaq.h\
    cadaq.h\
    caumucomain.h \
    cafile.h \
    caexperiment.h \
    caexperimentwindow.h \
    cautilities.h \
    caglwidget.h \
    caimagesimulator.h \
    caumucomainwindow.h \
    cadaqwindow.h\
    qcustomplot/qcustomplot.h \
    cacamera.h \
    castage.h \
    castagewindow.h \
    caalignmenttool.h

FORMS    += \
    cadaqwindow.ui\
    caexperimentwindow.ui \
    caumucomainwindow.ui \
    castagewindow.ui \
    caalignmenttool.ui

DISTFILES += \
    resources/Button-Forward-icon.png

RESOURCES += \
    resources.qrc

QT += serialport\
    core opengl

RC_FILE = umuco.rc

INCLUDEPATH += $$PWD/qcustomplot

!mac {
    SOURCES += cahamamatsu.cpp \
        cahamamatsuwindow.cpp

    HEADERS += cahamamatsu.h \
        cahamamatsuwindow.h

    FORMS += cahamamatsuwindow.ui

    INCLUDEPATH += $$PWD/hamamatsu/include

    LIBS += -L$$PWD/hamamatsu/bin -ldcamapi -lpowrprof -ldxguid
    LIBS += -lopengl32 -lglu32

    INCLUDEPATH += $$PWD/hdf5/include
    LIBS += -L$$PWD/hdf5/lib
    LIBS += -lhdf5_cpp -lhdf5_hl_cpp -lhdf5

    INCLUDEPATH += $$PWD/nidaqmx/include
    LIBS += -L$$PWD/nidaqmx/lib64/msvc -lNIDAQmx

    INCLUDEPATH +="C:\OpenCV\include"
    LIBS += -L"C:/OpenCV/x64/vc12/lib" -L"C:/OpenCV/3rdparty/lib" \
            -lopencv_core300 -lopencv_imgproc300 -lopencv_imgcodecs300 -lopencv_highgui300\
            -lopencv_core300d -lopencv_imgproc300d -lopencv_imgcodecs300d -lopencv_highgui300d
} else {
    INCLUDEPATH +=/usr/local/include /usr/local/opt/opencv3/include
    LIBS += -L/usr/local/lib -L/usr/local/opt/opencv3/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc\
             -lopencv_highgui
    INCLUDEPATH += /usr/local/opt/hdf5/include
    LIBS += -lhdf5_cpp -lhdf5_hl_cpp -lhdf5
}
