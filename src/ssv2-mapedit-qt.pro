QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += shared/ headers/ ./

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    actor.cpp \
    debug.cpp \
    dlgselect.cpp \
    framemap.cpp \
    imswrap.cpp \
    main.cpp \
    mainwindow.cpp \
    mapfile.cpp \
    mapscroll.cpp \
    mapwidget.cpp \
    script.cpp \
    scriptarch.cpp \
    selection.cpp \
    shared/DotArray.cpp \
    shared/FileWrap.cpp \
    shared/Frame.cpp \
    shared/FrameSet.cpp \
    shared/PngMagic.cpp \
    shared/helper.cpp \
    shared/qtgui/qfilewrap.cpp \
    shared/qtgui/qthelper.cpp

HEADERS += \
    actor.h \
    debug.h \
    defs.h \
    dlgselect.h \
    framemap.h \
    imswrap.h \
    mainwindow.h \
    mapfile.h \
    mapscroll.h \
    mapwidget.h \
    script.h \
    scriptarch.h \
    selection.h \
    shared/CRC.h \
    shared/DotArray.h \
    shared/FileWrap.h \
    shared/Frame.h \
    shared/FrameSet.h \
    shared/IFile.h \
    shared/ISerial.h \
    shared/PngMagic.h \
    shared/glhelper.h \
    shared/helper.h \
    shared/qtgui/cheat.h \
    shared/qtgui/qfilewrap.h \
    shared/qtgui/qthelper.h

FORMS += \
    dlgselect.ui \
    mainwindow.ui

unix:LIBS += -lz
win32:LIBS += -L"libs" -lzlib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ssv2-mapedit-qt.qrc


