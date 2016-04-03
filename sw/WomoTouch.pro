#-------------------------------------------------
#
# Project created by QtCreator 2015-12-21T07:14:26
#
#-------------------------------------------------

QT       += core gui
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WomoTouch
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    rtcom.cpp

HEADERS  += mainwindow.h \
    rtcom.h

FORMS    += mainwindow.ui

DISTFILES +=


