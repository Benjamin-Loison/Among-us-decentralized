QT += widgets
QT += network
QT += multimedia

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LFLAGS += --coverage
}

TRANSLATIONS = assets/AmongUsDecentralizedFR.ts
RC_ICONS += assets/logo.ico

QMAKE_CXXFLAGS += -I./sources/includes/ui -I./sources/includes/tasks -I./sources/includes
unix:OBJECTS_DIR = build/objects
MOC_DIR = build/moc


SOURCES += \
    sources/cpp/tasks/*.cpp \
    sources/cpp/ui/*.cpp \
    sources/cpp/*.cpp

HEADERS += \
    sources/includes/tasks/*.h \
    sources/includes/ui/*.h \
    sources/includes/*.h

# Source: https://stackoverflow.com/questions/19066593/copy-a-file-to-build-directory-after-compiling-project-with-qt
win32 {
    PWD_WIN = $${PWD}
    DESTDIR_WIN = $${OUT_PWD}
    PWD_WIN ~= s,/,\\,g
    DESTDIR_WIN ~= s,/,\\,g
    copydata.commands = $(COPY_DIR) $$PWD_WIN\\assets $$DESTDIR_WIN\\assets
}
macx {
    copydata.commands =
}
linux {
    copydata.commands = $(COPY_DIR) $$PWD/assets $$OUT_PWD
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
