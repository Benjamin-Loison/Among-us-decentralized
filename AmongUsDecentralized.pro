QT += widgets
QT += multimedia

SOURCES += \
    InGameUI.cpp \
    asteroids.cpp \
    Player.cpp \
    fixWiring.cpp \
    main.cpp \
    qPlus.cpp \
    Server.cpp \
    Client.cpp

HEADERS += \
    InGameUI.h \
    asteroids.h \
    Player.h \
    fixWiring.h \
    main.h \
    qPlus.h \
    Server.h \
    Client.h

# Source: https://stackoverflow.com/questions/19066593/copy-a-file-to-build-directory-after-compiling-project-with-qt
win32 {
    PWD_WIN = $${PWD}
    DESTDIR_WIN = $${OUT_PWD}
    PWD_WIN ~= s,/,\\,g
    DESTDIR_WIN ~= s,/,\\,g
    copydata.commands = $(COPY_DIR) $$PWD_WIN\\assets $$DESTDIR_WIN\\assets
}
macx {
    copydata.commands = $(COPY_DIR) $$PWD/assets $$OUT_PWD
}
linux {
    copydata.commands = $(COPY_DIR) $$PWD/assets $$OUT_PWD
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
