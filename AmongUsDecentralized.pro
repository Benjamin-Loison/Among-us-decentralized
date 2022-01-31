QT += widgets
QT += network
QT += multimedia

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LFLAGS += --coverage
}

TRANSLATIONS = assets/AmongUsDecentralizedFR.ts
RC_ICONS += assets/logo.ico

QMAKE_CXXFLAGS += -I./sources/guis -I./sources/map -I./sources/network -I./sources/tasks -I./sources/ui -I./sources
unix:OBJECTS_DIR = build/objects
MOC_DIR = build/moc

SOURCES += \
    sources/guis/*.cpp \
    sources/map/*.cpp \
    sources/network/*.cpp \
    sources/tasks/*.cpp \
    sources/ui/*.cpp \
    sources/*.cpp

HEADERS += \
    sources/guis/*.h \
    sources/map/*.h \
    sources/network/*.h \
    sources/tasks/*.h \
    sources/ui/*.h \
    sources/*.h

# Source: https://stackoverflow.com/questions/19066593/copy-a-file-to-build-directory-after-compiling-project-with-qt
win32 {
    PWD_WIN = $${PWD}
    DESTDIR_WIN = $${OUT_PWD}
    PWD_WIN ~= s,/,\\,g
    DESTDIR_WIN ~= s,/,\\,g
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
