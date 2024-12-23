QT       += core gui
QT += widgets
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


QMAKE_CXXFLAGS += -g -O0
INSTALLS += target
target.path = /opt


TARGET = QtWuziqi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    GameModel.cpp

HEADERS  += mainwindow.h \
    GameModel.h

RESOURCES += \
    resource.qrc

DISTFILES += \
    sound/chessone.wav \
    sound/lose.wav \
    sound/win.wav

