#-------------------------------------------------
#
# Project created by QtCreator 2015-04-02T14:45:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = heat_master
TEMPLATE = app


SOURCES += main.cpp \
    MainWindow.cpp \
    Worker.cpp \
    TcpServer.cpp

HEADERS  += \
    MainWindow.h \
    Worker.h \
    TcpServer.h

FORMS    += mainwindow.ui
