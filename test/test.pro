#-------------------------------------------------
#
# Project created by QtCreator 2016-01-08T11:40:31
#
#-------------------------------------------------

QT       += core gui
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../../raspberry_example/rtcom.cpp

HEADERS  += mainwindow.h \
    ../../raspberry_example/rtcom.h

FORMS    += mainwindow.ui
