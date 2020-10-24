QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    get_info.cpp \
    main.cpp \
    mainwindow.cpp \
    socket.cpp \
    thread.cpp

HEADERS += \
    get_info.h \
    mainwindow.h \
    socket.h \
    thread.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += /root/android/sdk/ndk/21.1.6352462/sysroot/usr/include

RESOURCES += \
    icons.qrc

