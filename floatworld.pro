TEMPLATE = app
CONFIG += qt \
    debug
HEADERS += src/*.hpp \
    gui/newworld.hpp
HEADERS += gui/*.hpp
SOURCES += src/*.cpp
SOURCES += gui/*.cpp
OBJECTS_DIR = build
DESTDIR = build
FORMS += gui/mainwindow.ui \
    gui/newworld.ui
RESOURCES += gui/floatworld.qrc

OTHER_FILES += \
    README.markdown \
    PLANS
