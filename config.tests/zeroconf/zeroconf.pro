win32:include(../../depends.pri)
unix:!macx:LIBS += -ldns_sd
CONFIG -= app_bundle
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += zeroconf.cpp
QT=core
CONFIG -= app_bundle