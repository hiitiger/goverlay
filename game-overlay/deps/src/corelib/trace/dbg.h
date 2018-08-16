#pragma once

namespace trace {

    bool d_assert(bool cond, const char* file, const char* func, int line, const char* msg);

}

#ifdef _DEBUG
#define  DAssert(e) trace::d_assert((!!(e)), __FILE__, __FUNCTION__, __LINE__,"")
#define  DAssert_M(e, msg) trace::d_assert((!!(e)), __FILE__, __FUNCTION__, __LINE__, msg)
#else
#define  DAssert(e) 
#define  DAssert_M(e, msg)  
#endif // _DEBUG