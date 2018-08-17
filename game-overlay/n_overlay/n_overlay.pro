TEMPLATE = lib

CONFIG -= qt

DEFINES += _WIN32_WINNT=0x0601

INCLUDEPATH += .
INCLUDEPATH += ../deps/include
INCLUDEPATH += $(DXSDK_DIR)Include
CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += ../deps/lib/debug
}
CONFIG(release, debug|release) {
    QMAKE_LIBDIR += ../deps/lib/release
}

OBJECTS_DIR = $(SolutionDir)Objdir/$(ProjectName)/$(Configuration)

DESTDIR = $(SolutionDir)/bin/$(Configuration)

PRECOMPILED_HEADER = stable.h

QMAKE_CFLAGS_RELEASE += /MT
QMAKE_CXXFLAGS_RELEASE += /MT
QMAKE_CFLAGS_DEBUG += /MTd
QMAKE_CXXFLAGS_DEBUG += /MTd


contains(QMAKE_TARGET.arch, x86_64):{
message( "Building for 64 bit")

TARGET = $$join(TARGET,,,_x64)
LIBS += libMinHook.x64.lib
LIBS += corelib.x64.lib
}

!contains(QMAKE_TARGET.arch, x86_64):{
message( "Building for 32 bit")
LIBS += libMinHook.x86.lib
LIBS += corelib.lib
}


HEADERS += dxgi1_2.h
SOURCES += ./main.cpp

HEADERS += ./hook/apihook.hpp

HEADERS += ./ipc/tinyipc.h
HEADERS += ./ipc/ipcmsg.h
HEADERS += ./ipc/ipclink.h
SOURCES += ./ipc/ipclink.cc
HEADERS += ./ipc/ipccenter.h
SOURCES += ./ipc/ipccenter.cc


HEADERS += ./overlay/uiapp.h
SOURCES += ./overlay/uiapp.cc

HEADERS += ./overlay/hookapp.h
SOURCES += ./overlay/hookapp.cc

HEADERS += ./overlay/overlay.h
SOURCES += ./overlay/overlay.cc

