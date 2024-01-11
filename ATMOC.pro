QT += quick widgets quickcontrols2 multimedia
RC_FILE = recource.rc
TARGET = CAT
CONFIG += c++11
DEFINES += QT_MESSAGELOGCONTEXT

SOURCES += \
        CompMonitor.cpp \
        EnviModule.cpp \
        EnviReader.cpp \
        LeastSquareSolver.cpp \
        QrcFilesRestorer.cpp \
        Sounder.cpp \
        main.cpp \
        mpfit.c

HEADERS += \
    CompMonitor.h \
    EnviModule.h \
    EnviReader.h \
    PolygonCollisionChecker.h \
    QrcFilesRestorer.h \
    Sounder.h \
    UniversalDarkPixelFinder.h \
    UniversalImageReader.h \
    Version.h \
    common_types.h \
    mpfit.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


