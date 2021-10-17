QT += widgets
QT += multimedia

SOURCES += \
    QLabelKeys.cpp \
    fixWiring.cpp \
    main.cpp \
    qPlus.cpp \
    Server.cpp \
    Client.cpp

HEADERS += \
    QLabelKeys.h \
    fixWiring.h \
    main.h \
    qPlus.h \
    Server.h \
    Client.h

# Source: https://stackoverflow.com/questions/19066593/copy-a-file-to-build-directory-after-compiling-project-with-qt
copydata.commands = $(COPY_DIR) $$PWD/assets $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
