QT += widgets
QT += network
QT += multimedia

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LFLAGS += --coverage
}

TRANSLATIONS = assets/AmongUsDecentralizedFR.ts assets/AmongUsDecentralizedEN.ts
RC_ICONS += assets/logo.ico

unix:OBJECTS_DIR = build/objects
MOC_DIR = build/moc

HEADERS += \
    sources/guis/camera.h \
    sources/guis/meetings.h \
    sources/guis/vitals.h \
    sources/main.h \
    sources/map/Door.h \
    sources/map/Player.h \
    sources/map/Room.h \
    sources/map/Vents.h \
    sources/network/Client.h \
    sources/network/Server.h \
    sources/qPlus.h \
    sources/tasks/AlignEngine.h \
    sources/tasks/EnterIDCode.h \
    sources/tasks/Task.h \
    sources/tasks/asteroids.h \
    sources/tasks/fixWiring.h \
    sources/ui/GameMap.h \
    sources/ui/InGameUI.h

SOURCES += \
    sources/guis/camera.cpp \
    sources/guis/meetings.cpp \
    sources/guis/vitals.cpp \
    sources/main.cpp \
    sources/map/Door.cpp \
    sources/map/Player.cpp \
    sources/map/Room.cpp \
    sources/map/Vents.cpp \
    sources/network/Client.cpp \
    sources/network/Server.cpp \
    sources/qPlus.cpp \
    sources/tasks/AlignEngine.cpp \
    sources/tasks/EnterIDCode.cpp \
    sources/tasks/Task.cpp \
    sources/tasks/asteroids.cpp \
    sources/tasks/fixWiring.cpp \
    sources/ui/GameMap.cpp \
    sources/ui/InGameUI.cpp

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

DISTFILES += \
    assets/AmongUsDecentralizedEN.ts
