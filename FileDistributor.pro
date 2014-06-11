#-------------------------------------------------
#
# Project created by QtCreator 2014-04-20T20:41:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileDistributor
TEMPLATE = app


SOURCES += main.cpp\
        distributordialog.cpp \
    copier.cpp

HEADERS  += distributordialog.h \
    copier.h

FORMS    += distributordialog.ui

RESOURCES += \
    FileDistributor.qrc

RC_ICONS = FD.ico
