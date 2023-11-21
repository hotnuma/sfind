TEMPLATE = app
TARGET = sfind
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += tinyc

HEADERS = \
    dirparser.h \
    entry.h \
    pathcmp.h

SOURCES = \
    0Temp.c \
    dirparser.c \
    entry.c \
    main.c \
    pathcmp.c

DISTFILES = \
    Readme.md \
    install.sh \
    meson.build \


