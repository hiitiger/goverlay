#pragma once

#define  MOD_CORELIB "Corelib"


#define  STRINGINFY_(x) #x
#define  STRINGINFY(x) STRINGINFY_(x)

#define STORM_NONCOPYABLE(classname)\
    classname(const classname &) = delete; \
    classname &operator=(const classname &) = delete;


///------------------------------------------------------------------
struct  LambdaNoCopy
{
    LambdaNoCopy() = default;
    LambdaNoCopy(const LambdaNoCopy&) = delete;
    LambdaNoCopy& operator=(const LambdaNoCopy&) = delete;
    LambdaNoCopy(LambdaNoCopy&&) = default;
};

#define  CONCAT_INNER(x, y) x##y
#define  CONCAT(X,Y) CONCAT_INNER(X, Y)
#define  UNIQ_ID(X) CONCAT(X, __COUNTER__)
#define  nocopy UNIQ_ID(x) = LambdaNoCopy()
///------------------------------------------------------------------

