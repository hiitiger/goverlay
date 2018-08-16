#pragma once

namespace Storm
{
    class IO
    {

    public:
        enum IOStatus
        {
            IO_Pending,
            IO_Failed,
            IO_Succees,
        };

        enum IOReqType
        {
            IO_Read,
            IO_Write,
        };


    class IOReqeust : public RefCounted
    {
        STORM_NONCOPYABLE(IOReqeust);

    public:
        IOReqeust(IO::IOReqType type)
            : status_(IO::IO_Pending)
            , type_(type)
            , validSize_(0)
        {
        }

        ~IOReqeust()
        {
        }

        std::atomic<IO::IOStatus> status_;
        const IO::IOReqType type_;

        std::wstring file_;
        Buffer  data_;
        int32_t validSize_;
    };



    static WorkerPool* ioPool();

    static void stopIO();

    typedef std::function<void(RefPtr<IOReqeust>)> IOCallback;
 
    static void postIORequest(RefPtr<IOReqeust>& req, const IOCallback& cb);

    static RefPtr<IOReqeust> asyncRead(const std::wstring& file, const IOCallback& cb);

    static RefPtr<IOReqeust> asyncWrite(const std::wstring& file, const Buffer& data, const IOCallback& cb);

    };

}
