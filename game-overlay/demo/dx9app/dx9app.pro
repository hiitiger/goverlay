TEMPLATE = app
CharacterSet=1
CONFIG -= qt


INCLUDEPATH += .
INCLUDEPATH += ../../deps/include
INCLUDEPATH += ../../deps
INCLUDEPATH += pre

PRECOMPILED_HEADER = pre/stdafx.h

CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += ../../deps/lib/debug
}
CONFIG(release, debug|release) {
    QMAKE_LIBDIR += ../../deps/lib/release
}

contains(QMAKE_TARGET.arch, x86_64):{
message( "Building for 64 bit")

QMAKE_LIBDIR += ../../deps/dxsdk/Lib/x64
}

!contains(QMAKE_TARGET.arch, x86_64):{
message( "Building for 32 bit")

QMAKE_LIBDIR += ../../deps/dxsdk/Lib/x86

}

QMAKE_CXXFLAGS += /std:c++latest

OBJECTS_DIR = $(SolutionDir)Objdir/$(ProjectName)/$(Configuration)

DESTDIR = $(SolutionDir)/bin/$(Configuration)

LIBS+=legacy_stdio_definitions.lib

QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /DEBUG

HEADERS += $$files(./*.h)
SOURCES += $$files(./*.cpp)



RC_FILE = D9Game.rc
