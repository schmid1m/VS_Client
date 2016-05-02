TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    VS_LAB/clientAPI.c \
    VS_LAB/PacketLib.c

HEADERS += \
    VS_LAB/clientAPI.h \
    VS_LAB/commonAPI.h \
    VS_LAB/internalMacros.h \
    VS_LAB/Macros.h \
    VS_LAB/PacketLib.h

