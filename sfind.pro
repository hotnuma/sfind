TEMPLATE = app
TARGET = sfind
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE bool=BOOL true=TRUE false=FALSE
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += tinyc

HEADERS = \
    dirparser.h \
    entry.h

SOURCES = \
    0temp.c \
    dirparser.c \
    entry.c \
    main.c

DISTFILES = \
    Readme.md \
    install.sh \
    meson.build \


