#include "stable.h"
#include "dbgconsole.h"

#pragma warning(disable:4996)

namespace trace {

void DebugConsole::allocDebugConsole()
{
//#ifdef _DEBUG
    AllocConsole();
    SetConsoleTitleW(L"Debug Console");
    freopen("conin$", "r+t", stdin);
    freopen("conout$", "w+t", stdout);
    freopen("conout$", "w+t", stderr);

//#endif // _DEBUG
}

void DebugConsole::releaseDebugConsole()
{
	HWND wnd = GetConsoleWindow();
	FreeConsole();

	PostMessage(wnd, WM_CLOSE, 0, 0);
}

void DebugConsole::setConsoleTitle(const std::string& value)
{
	SetConsoleTitleA(value.c_str());
}

}

namespace trace
{
    struct output
    {
        std::string mod_;
        std::string msg_;
        std::chrono::microseconds ts_;
    };

    std::vector<rang::fg> color_ = { rang::fg::red, rang::fg::green, rang::fg::yellow,rang::fg::blue,rang::fg::magenta,rang::fg::cyan, rang::fg::gray };
    std::vector<rang::fgB> colorB_ = { rang::fgB::red, rang::fgB::green, rang::fgB::yellow,rang::fgB::blue,rang::fgB::magenta,rang::fgB::cyan, rang::fgB::gray };
    
    std::set< std::string > mods_;
    std::unordered_map<std::string, std::chrono::microseconds> mods_ts_;
    std::unordered_map<std::string, int> mods_color_;
    Storm::SyncQueue<output> outputs_;
    std::atomic<bool> quit_ = false;

    bool enabled_(const std::string& mod)
    {
        return true;
        return mods_.find(mod) != mods_.end();
    }

    void write_(const std::string& m, const std::string& msg, std::chrono::microseconds d)
    {
        using namespace std::chrono;
        std::cout << color_[mods_color_[m]] << m << rang::style::reset << " " << msg << " " << colorB_[mods_color_[m]] << "+" <<  duration_cast<milliseconds>(d).count() << "ms" << rang::style::reset << std::endl;
    }

    struct DebugThread
    {
        std::thread thread_;

        ~DebugThread() {
            quit_ = true;
            outputs_.stop();
            thread_.join();
        }

        static int color(const std::string& mod) {
            
            int i = (int)(std::hash<std::string>()(mod)) % color_.size();
            return i;
        }

        static DebugThread& get()
        {
            static DebugThread t{ std::thread([&]() {

                while (!quit_) {
                    std::deque<output> outputs;
                    outputs_.dequeueAll(outputs);
                    for (auto&i : outputs)
                    {
                        std::chrono::microseconds d = std::chrono::microseconds::zero();
                        auto it = mods_ts_.find(i.mod_);
                        if (it != mods_ts_.end()) {
                            auto ts = it->second;
                            d = i.ts_ - ts;
                        }
                        mods_ts_.insert_or_assign(i.mod_, i.ts_);

                        auto itc = mods_color_.find(i.mod_);
                        if (itc == mods_color_.end())
                        {
                            mods_color_.insert_or_assign(i.mod_, color(i.mod_));
                        }

                        write_(i.mod_, i.msg_, d);
                    }
                }
            })};

            return t;
        }

        void enqueue(output&& o) {
            outputs_.enqueue(std::move(o));
        }
    };

    void DebugTrace::next_(const std::string& mod, const std::string& msg, std::chrono::microseconds t)
    {
        if (enabled_(mod))
        {
            DebugThread::get().enqueue({ mod, msg, t });
        }
    }
}