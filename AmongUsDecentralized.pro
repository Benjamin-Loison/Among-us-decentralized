QT += widgets
QT += network
QT += multimedia

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LFLAGS += --coverage
}

TRANSLATIONS = assets/AmongUsDecentralizedFR.ts
RC_ICONS += assets/logo.ico

QMAKE_CXXFLAGS += -I./sources/includes/guis -I./sources/includes/map -I./sources/includes/network -I./sources/includes/tasks -I./sources/includes/ui -I./sources/includes -I./sources/includes/tasks/cannon
unix:OBJECTS_DIR = build/objects
MOC_DIR = build/moc

SOURCES += \
    sources/cpp/guis/*.cpp \
    sources/cpp/map/*.cpp \
    sources/cpp/network/*.cpp \
    sources/cpp/tasks/*.cpp \
    sources/cpp/ui/*.cpp \
    sources/cpp/*.cpp \
    sources/cpp/tasks/cannon/*.cpp

HEADERS += \
    sources/includes/guis/*.h \
    sources/includes/map/*.h \
    sources/includes/network/*.h \
    sources/includes/tasks/*.h \
    sources/includes/ui/*.h \
    sources/includes/*.h \
    sources/includes/tasks/cannon/*.h

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
