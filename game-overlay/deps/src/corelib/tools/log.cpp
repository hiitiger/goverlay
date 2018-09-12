#include "stable.h"
#include "log.h"
#include <process.h>
#include <time.h>


namespace Storm {

const int k_MAXLOGFILESIZE = 32 * 1024 * 1024;
static std::atomic<bool> k_logQuit = false;

class Logger
{
public:
    struct LogItem
    {
        LogItem()
        {
            memset(mod, 0, sizeof(char) * 64);
            memset(file, 0, sizeof(char) * 128);
            memset(function, 0, sizeof(char) * 128);
        }
        char mod[64];
        char file[128];
        char function[128];
        unsigned int line;
        DateTime datetime;
        std::wstring data;
    };


    DWORD threadId_;
    std::wstring fullPath_;

    std::thread thread_;
    std::condition_variable cv_;
    std::mutex logLock_;
    std::deque<std::unique_ptr<LogItem>> logItems_;

    Logger()
        : threadId_(0)
    {

        fullPath_ = Utils::appDataPath();
        fullPath_.append(L"\\log\\");
        Utils::makeSureDirExist(fullPath_);

        fullPath_.append(Utils::appProcessName());
        fullPath_.erase(fullPath_.find_last_of('.'));
        fullPath_.append(L".log");

        start();
    }
    ~Logger()
    {
        k_logQuit = true;
        cv_.notify_one();
        thread_.join();
    }

    void start()
    {
        thread_ = std::thread([this] { _logThread(); });
       
        addLog(MOD_CORELIB, __FILE__, __FUNCTION__, __LINE__, L"Start...");
    }

    void addLog(const char* mod, const char* file, const char* function, unsigned int line, const std::wstring& data)
    {
        LogItem* item = new LogItem;
        _snprintf_s(item->mod, _TRUNCATE, mod);
        _snprintf_s(item->file, _TRUNCATE, file);
        _snprintf_s(item->function, _TRUNCATE, function);
        item->line = line;
        item->datetime = DateTime::now();
        item->data = data;

        std::unique_lock<std::mutex> lock(logLock_);
        logItems_.push_back(std::unique_ptr<LogItem>(item));
        cv_.notify_one();
    }

private:
    void _writeLog(std::fstream& fs, std::deque<std::unique_ptr<LogItem>>& logItems)
    {
        const size_t HEADER_SIZE = 512;
        char header[HEADER_SIZE] = { 0 };

        for (auto it = logItems.begin(); it != logItems.end(); ++it)
        {
            auto item = std::move(*it);

            fs << item->datetime;

            memset(header, 0, sizeof(char) * HEADER_SIZE);
            sprintf_s(header, HEADER_SIZE, " mod:%s,file:%s,function:%s,line:%d,log:", item->mod, item->file, item->function, item->line);
            fs << header;

            std::string content = Utils::toUtf8(item->data.c_str(), static_cast<int>(item->data.size()));
            fs << content << std::endl;

            fs.flush();
        }
    }

    static  unsigned int __stdcall logThread(void* p)
    {
        ((Logger*)p)->_logThread();
        return 0;
    }

    void _backup(const std::wstring& file)
    {
        std::wstring bak = file + L".bak";
        std::experimental::filesystem::rename(file, bak);
    }

    void _logThread()
    {
        std::fstream fs;
        fs.open(fullPath_, std::ios_base::out | std::ios_base::app);

        bool running = true;
        while (true)
        {
            std::deque<std::unique_ptr<LogItem>> logItems;
            {
                std::unique_lock<std::mutex> lock(logLock_);
                while (!k_logQuit && logItems_.empty())
                {
                    cv_.wait(lock);
                }

                logItems.swap(logItems_);
            }

            _writeLog(fs, logItems);

            if (k_logQuit)
            {
                fs << DateTime::now();

                std::string end = " End...";
                fs << end << std::endl;
                break;
            }

            if (fs.tellp() > k_MAXLOGFILESIZE)
            {
                fs.close();
                _backup(fullPath_);
                fs.open(fullPath_,std::ios_base::out);
            }
        }

        fs.close();

        running = false;

        if (std::experimental::filesystem::file_size(fullPath_) > k_MAXLOGFILESIZE)
            _backup(fullPath_);
    }
};


static Logger* logger()
{
    static Logger logInstance;
    return &logInstance;
}


LogStream::~LogStream()
{
    if (!k_logQuit)
    {
        logger()->addLog(mod_, file_, function_, line_, textstream_.data());
    }
}

}
