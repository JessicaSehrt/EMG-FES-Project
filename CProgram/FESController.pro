TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
        main_kopie.c

SOURCES += $$files(src/*.c)
HEADERS += $$files(src/*.h)

DISTFILES += \
    lib/libsmpt.lib


