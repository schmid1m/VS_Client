QT += core network
QT -= gui

TARGET = vs_client
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    VS_LAB/clientAPI.c \
    VS_LAB/PacketLib.c \
    vs_client.cpp \
    vs_server.cpp \
    VS_LAB/cwrapper.cpp \
    VS_LAB/myqtsocket.cpp

HEADERS += \
    VS_LAB/clientAPI.h \
    VS_LAB/commonAPI.h \
    VS_LAB/internalMacros.h \
    VS_LAB/Macros.h \
    VS_LAB/PacketLib.h \
    vs_client.h \
    vs_server.h \
    VS_LAB/cwrapper.h \
    VS_LAB/myqtsocket.h

