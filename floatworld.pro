TEMPLATE = app
CONFIG += qt \
    debug
HEADERS += src/*.hpp
HEADERS += gui/*.hpp
SOURCES += src/*.cpp
SOURCES += gui/*.cpp
OBJECTS_DIR = build
DESTDIR = build
FORMS += gui/mainwindow.ui
RESOURCES += gui/floatworld.qrc
QMAKE_CFLAGS+=-pg
QMAKE_CXXFLAGS+=-pg
QMAKE_LFLAGS+=-pg
