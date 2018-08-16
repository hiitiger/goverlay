TEMPLATE = lib

CONFIG -= qt

DEFINES += _WIN32_WINNT=0x0601

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


SOURCES += $$files(./main.cpp)