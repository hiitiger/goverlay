#include "stable.h"
#include "io.h"
#include "thread/dispatcher.h"
#include "thread/workerpool.h"

namespace Storm
{

    Storm::WorkerPool* IO::ioPool()
    {
        static WorkerPool pool;
        return &pool;
    }

    void IO::stopIO()
    {
        ioPool()->stop();
    }

    void IO::postIORequest(RefPtr<IOReqeust>& req, const IOCallback& cb)
    {
        ThreadDispatcherPtr dispatcher = ThreadDispatcher::current();
        IO::ioPool()->add([=]() {
            if (req->type_ == IO_Read)
            {
                File file(req->file_);
                if (file.open(File::ReadOnly))
                {
                    req->data_ = file.readAll();
                    req->status_ = IO_Succees;
                }
                else
                {
                    req->status_ = IO_Failed;
                }
            }
            else
            {
                File file(req->file_);
                if (file.open(File::ReadWrite))
                {
                    req->validSize_ = file.write(req->data_.data(), req->data_.size());
                    if (req->validSize_ == req->data_.size())
                    {
                        req->status_ = IO_Succees;
                    }
                    else
                    {
                        req->status_ = IO_Failed;
                    }
                }
                else
                {
                    req->status_ = IO_Failed;
                }
            }

            dispatcher->invokeAsync([=]() {
                cb(req);
            });
        });
    }

    Storm::RefPtr<IO::IOReqeust> IO::asyncRead(const std::wstring& file, const IOCallback& cb )
    {
        RefPtr<IO::IOReqeust> req = new IO::IOReqeust(IO_Read);
        req->file_ = file;
        postIORequest(req, cb);
        return req;
    }

    Storm::RefPtr<IO::IOReqeust> IO::asyncWrite(const std::wstring& file, const Buffer& data, const IOCallback& cb )
    {
        RefPtr<IO::IOReqeust> req = new IO::IOReqeust(IO_Write);
        req->file_ = file;
        req->data_ = data;
        postIORequest(req, cb);
        return req;
    }

}

