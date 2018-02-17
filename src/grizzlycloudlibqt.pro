QT       += network
QT       -= gui
#QMAKE_CXXFLAGS += -DNO_TLS

TARGET = grizzlycloudqt
TEMPLATE = lib
CONFIG += staticlib

DEFINES += GC_CLIENT_LIB_LIBRARY

SOURCES += grizzlycloud.cpp \
    proto.cpp \
    utils.cpp \
    tunnel.cpp \
    message.cpp \
    allowed.cpp \
    gcclient.cpp \
    gcserver.cpp \
    gcthread.cpp

HEADERS += grizzlycloud.h\
        gc_client_lib_global.h \
    proto.hpp \
    utils.h \
    client.h \
    tunnel.h \
    message.h \
    allowed.h \
    gcclient.h \
    gcserver.h \
    gcthread.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
