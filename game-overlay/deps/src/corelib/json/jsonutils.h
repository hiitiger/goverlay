#pragma once

#include <optional>
#include "json.h"
using namespace nlohmann;


#ifdef _MSC_VER // Microsoft compilers

#   define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))

#   define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#   define INTERNAL_EXPAND(x) x
#   define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#else // Non-Microsoft compilers

#   define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#endif


#define STRINGIZE(arg)  STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) #arg

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define FOR_EACH_0(what)
#define FOR_EACH_1(what, x1) what(x1)
#define FOR_EACH_2(what, x1, x2)\
  what(x1);\
  FOR_EACH_1(what, x2)
#define FOR_EACH_3(what, x1, x2, x3)\
  what(x1);\
  FOR_EACH_2(what, x2, x3)
#define FOR_EACH_4(what, x1, x2, x3, x4)\
  what(x1);\
  FOR_EACH_3(what,  x2, x3, x4)
#define FOR_EACH_5(what, x1, x2, x3, x4, x5)\
  what(x1);\
  FOR_EACH_4(what,  x2, x3, x4, x5)
#define FOR_EACH_6(what, x1, x2, x3, x4, x5, x6)\
  what(x1);\
  FOR_EACH_5(what,  x2, x3, x4, x5, x6)
#define FOR_EACH_7(what, x1, x2, x3, x4, x5, x6, x7)\
  what(x1);\
  FOR_EACH_6(what,  x2, x3, x4, x5, x6, x7)
#define FOR_EACH_8(what, x1, x2, x3, x4, x5, x6, x7, x8)\
  what(x1);\
  FOR_EACH_7(what,  x2, x3, x4, x5, x6, x7, x8)
#define FOR_EACH_9(what, x1, x2, x3, x4, x5, x6, x7, x8, x9)\
  what(x1);\
  FOR_EACH_8(what,  x2, x3, x4, x5, x6, x7, x8, x9)
#define FOR_EACH_10(what, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10)\
  what(x1);\
  FOR_EACH_9(what,  x2, x3, x4, x5, x6, x7, x8, x9, x10)
#define FOR_EACH_11(what, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)\
  what(x1);\
  FOR_EACH_10(what,  x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)
#define FOR_EACH_12(what, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12)\
  what(x1);\
  FOR_EACH_11(what,  x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12)
#define FOR_EACH_13(what, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13)\
  what(x1);\
  FOR_EACH_12(what,  x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13)
#define FOR_EACH_14(what, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14)\
  what(x1);\
  FOR_EACH_13(what,  x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14)
#define FOR_EACH_15(what, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15)\
  what(x1);\
  FOR_EACH_14(what,  x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15)
#define FOR_EACH_16(what, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16)\
  what(x1);\
  FOR_EACH_15(what,  x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16)

#define VA_ARGS(...)  , ##__VA_ARGS__

#define FOR_EACH_(N, what, ...) CONCATENATE(FOR_EACH_, N)(what VA_ARGS(__VA_ARGS__))
#define FOR_EACH(what, ...) FOR_EACH_(GET_ARG_COUNT(__VA_ARGS__), what VA_ARGS(__VA_ARGS__))

template<class A>
struct json_setter
{
    __forceinline static void set(const A& field, json& j, const char* str)
    {
        j[str] = field;
    }
};


template<class A>
struct json_setter<std::optional<A>>
{
    __forceinline static void set(const std::optional<A>& field, json& j, const char* str)
    {
        if (field)
        {
            j[str] = field.value();
        }
    }
};


template<class A>
struct json_getter
{
    __forceinline static void get(A& field, const json& j, const char* str)
    {
        field = j.at(str).get<A>();
    }
};


template<class A>
struct json_getter<std::optional<A>>
{
    __forceinline static void get(std::optional<A>& field, const json& j, const char* str)
    {
        if (j.find(str) != j.cend())
        {
            field = j[str].get<A>();
        }
    }
};


#define JSON_SET(field) \
json_setter<std::decay<decltype(object.field)>::type>::set(object.field, j, STRINGIZE(field))

#define JSON_GET(field)\
json_getter<std::decay<decltype(object.field)>::type>::get(object.field, j, STRINGIZE(field))

#define JSON_SERIALIZE(CLS, ...)\
inline void to_json(json&j, const CLS& object) \
{\
    j = json{};\
    FOR_EACH(JSON_SET, __VA_ARGS__);\
}\

#define JSON_DESERIALIZE(CLS, ...)\
inline void from_json(const json&j, CLS& object) \
{\
    FOR_EACH(JSON_GET, __VA_ARGS__);\
}\


#define  JSON_AUTO(CLS, ...)\
JSON_DESERIALIZE(CLS, __VA_ARGS__)\
JSON_SERIALIZE(CLS, __VA_ARGS__)
