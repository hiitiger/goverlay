TEMPLATE = app

CONFIG -= qt
CONFIG += console

CharacterSet=1

OBJECTS_DIR = $(SolutionDir)Objdir/$(ProjectName)/$(Configuration)

DESTDIR = $(SolutionDir)/bin/$(Configuration)

QMAKE_CFLAGS_RELEASE += /MT
QMAKE_CXXFLAGS_RELEASE += /MT
QMAKE_CFLAGS_DEBUG += /MTd
QMAKE_CXXFLAGS_DEBUG += /MTd

contains(QMAKE_TARGET.arch, x86_64):{
message( "Building for 64 bit")

TARGET = $$join(TARGET,,,_x64)
}

!contains(QMAKE_TARGET.arch, x86_64):{
message( "Building for 32 bit")
}


HEADERS += $$files(./*.h)
SOURCES += $$files(./*.cpp)