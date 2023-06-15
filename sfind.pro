TEMPLATE = app
TARGET = sfind
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += tinyc

HEADERS = \
    dirparser.h \
    pathcmp.h

SOURCES = \
    dirparser.c \
    main.c \
    pathcmp.c

DISTFILES = \
    install.sh \
    meson.build \


