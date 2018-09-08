
TEMPLATE = lib
DEPENDPATH += .
DEFINES += TW_DLL_BASE

CONFIG += staticlib


include(../config/common.pri)

HEADERS += class.h
HEADERS += macro.h
HEADERS += winheader.h

HEADERS += $$files(third/nlohmann/json.hpp)
HEADERS += $$files(third/*.h)
HEADERS += $$files(third/*.hpp)
SOURCES += $$files(third/*.cpp)

HEADERS += $$files(basic/*.h)
SOURCES += $$files(basic/*.cpp)

HEADERS += $$files(core/*.h)
SOURCES += $$files(core/*.cpp)

HEADERS += $$files(callback/*.h)
SOURCES += $$files(callback/*.cpp)

HEADERS += $$files(container/*.h)
SOURCES += $$files(container/*.cpp)

HEADERS += $$files(memory/*.h)
SOURCES += $$files(memory/*.cpp)

HEADERS += $$files(thread/*.h)
SOURCES += $$files(thread/*.cpp)

HEADERS += $$files(fs/*.h)
SOURCES += $$files(fs/*.cpp)

HEADERS += $$files(tools/*.h)
SOURCES += $$files(tools/*.cpp)

HEADERS += $$files(trace/*.h)
SOURCES += $$files(trace/*.cpp)

HEADERS += $$files(xml/*.h)
HEADERS += $$files(xml/*.h)
SOURCES += $$files(xml/*.cpp)

HEADERS += $$files(json/*.h)
SOURCES += $$files(json/*.cpp)

HEADERS += $$files(io/*.h)
SOURCES += $$files(io/*.cpp)

HEADERS += $$files(ex/*.h)
SOURCES += $$files(ex/*.cpp)