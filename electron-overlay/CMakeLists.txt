project(electron-overlay)

add_definitions(-DNAPI_VERSION=6)
add_definitions(-DNAPI_EXPERIMENTAL)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")

add_definitions(-DNOMINMAX)
add_definitions(-D_WIN32_WINNT=0x0601)
add_definitions(-DUNICODE -D_UNICODE)

add_link_options(/DEBUG)


file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.cc src/*.cpp)


add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> )

include_directories(${CMAKE_JS_INC})
set(LIB_SUFFIX ".node")
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ${LIB_SUFFIX})
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})


execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REGEX REPLACE "[\r\n\"]" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

target_link_options(${PROJECT_NAME} PRIVATE /DEBUG;/OPT:REF;/OPT:ICF; )

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND xcopy \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release\\${PROJECT_NAME}${LIB_SUFFIX}\" \"${CMAKE_SOURCE_DIR}/electron-overlay\\\" /S /Y /E
    )