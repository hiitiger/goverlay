#pragma once

#include "../third/rang.h"

namespace trace {

enum class DbgType
{
    D_Info,
    D_Warning,
    D_Error,
    D_Ok,
};


class DebugConsole
{
public:

	static void allocDebugConsole();
	static void releaseDebugConsole();

	static void setConsoleTitle(const std::string&);
};


class DebugCout
{
public:
    DebugCout(DbgType t)
    {
        switch (t)
        {
        case DbgType::D_Info: {return; }
        case DbgType::D_Warning: {std::cout << rang::fg::yellow; return; }
        case DbgType::D_Error: {std::cout << rang::fg::red; return; }
        case DbgType::D_Ok: {std::cout << rang::fg::green; return; }
        }
    }
    ~DebugCout()
    {
        std::cout << rang::style::reset
            << std::endl;
    }

    template<class T>
    DebugCout& operator<< (T && a)
    {
        std::cout << std::forward<T>(a);
        return *this;
    }
};

#define  DbgCout(type) trace::DebugCout(type)

#define  DbgInfo    DbgCout(trace::DbgType::D_Info)
#define  DbgWarn    DbgCout(trace::DbgType::D_Warning)
#define  DbgError   DbgCout(trace::DbgType::D_Error)
#define  DbgOk      DbgCout(trace::DbgType::D_Ok)


}


namespace trace {

    class DebugTrace 
    {
        static void next_(const std::string& mod, const std::string& msg, std::chrono::microseconds t);
    public:
        DebugTrace(const std::string& mod)
            : mod_(mod)
            , micros_(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()))
        {

        }

        ~DebugTrace()
        {
            next_(mod_, ss_.str(), micros_);
        }

        template<class T>
        DebugTrace& operator << (T&& t)
        {
            ss_ << t;
            return (*this);
        }

    private:
        const std::string& mod_;
        std::stringstream ss_;
        std::chrono::microseconds micros_;
    };
};

#define  __debug(mod) trace::DebugTrace(mod)


namespace trace {

    class ConsolePerf
    {
        const char* msg_;

        std::chrono::microseconds start_ = std::chrono::microseconds::zero();
    public:
        ConsolePerf(const char* msg)
            : msg_(msg)
        {
            using namespace std::chrono;
            auto tp = steady_clock::now();
            auto cur = tp.time_since_epoch();
            start_ = duration_cast<microseconds>(cur);
        }

        ~ConsolePerf()
        {
            using namespace std::chrono;
            auto tp = steady_clock::now();
            auto cur = tp.time_since_epoch();
            auto end_ = duration_cast<microseconds>(cur);

            auto timeCost = (end_ - start_).count();
            std::cout << msg_ << " cost " << timeCost << "us" << std::endl;
        }
    };
}

#define  __perf_scope(msg) trace::ConsolePerf __perf_scope_##msg(#msg)