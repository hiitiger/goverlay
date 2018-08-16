#include "stable.h"
#include <process.h>
#include <crtdbg.h>
#include "dbg.h"

namespace trace {

    struct DebugAssertData
    {
        const char* file;
        const char* func;
        int line;
        const char* msg;
        bool doBreak;
    };

    bool d_assert(bool cond, const char* file, const char* func, int line, const char* msg)
    {
        if (cond)
        {
            return false;
        }

        DebugAssertData d = { file, func, line, msg, false };

#ifdef _DEBUG
  //      std::thread([&d](){
            int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
            int ret = _CrtDbgReport(_CRT_ERROR, d.file, d.line, d.func, d.msg);
            _CrtSetReportMode(_CRT_ERROR, reportMode);

            if (ret == 1)
            {
                d.doBreak = true;
            }

            if (d.doBreak)
            {
                DebugBreak();
            }
 //       }).join();
#endif // _DEBUG


        return true;
    }
}
