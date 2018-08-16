#pragma once
#include <Windows.h>
#include <string>

const int TinyIpcMsg_Zero = 0;

class MsgPacker
{
    std::string& m_data;
public:
    MsgPacker(std::string& data)
        : m_data(data)
    {

    }

    void pushData(const char* raw, size_t len)
    {
        m_data.append(raw, len);
    }

    void pushData(char d)
    {
        return pushData((const char*)(&d), sizeof(d));
    }

    void pushData(bool d)
    {
        return pushData((char)(d ? 1 : 0));
    }

    void pushData(int d)
    {
        return pushData((const char*)(&d), sizeof(d));
    }

    void pushData(unsigned int d)
    {
        return pushData((const char*)(&d), sizeof(d));
    }

    void pushData(int64_t d)
    {
        return pushData((const char*)(&d), sizeof(d));
    }

    void pushData(const std::string &str)
    {
        pushData((int)str.length());
        return pushData((const char*)(str.c_str()), sizeof(std::string::value_type)*str.length());
    }

    void pushData(const std::wstring &str)
    {
        pushData((int)(str.length() * sizeof(std::wstring::value_type)));
        return pushData((const char*)(str.c_str()), sizeof(std::wstring::value_type)*str.length());
    }
};

class MsgUnpacker
{
    const std::string& m_data;
    size_t m_pos;
public:
    MsgUnpacker(const std::string& data)
        : m_data(data)
        , m_pos(0)
    {

    }

    bool popData(char* raw, size_t len)
    {
        if ((m_pos + len) <= m_data.length()  )
        {
            memcpy(raw, m_data.c_str() + m_pos, len);
            m_pos += len;
            return true;
        }
        return false;
    }

    bool popData(char& d)
    {
        return popData((char*)(&d), sizeof(d));
    }

    bool popData(bool& d)
    {
        char x;
        if (popData(x))
        {
            d = x != 0;
            return true;
        }
        return false;
    }

    bool popData(int& d)
    {
        return popData((char*)(&d), sizeof(d));
    }

    bool popData(unsigned int& d)
    {
        return popData((char*)(&d), sizeof(d));
    }

    bool popData(int64_t& d)
    {
        return popData((char*)(&d), sizeof(d));
    }

    bool popData(std::string &str)
    {
       int len = 0;
       if (popData(len))
       {
           str.resize(len / sizeof(std::string::value_type));
           return popData(const_cast<char*>(str.c_str()), len);
       }
       else
       {
           return false;
       }
    }

    bool popData(std::wstring &str)
    {
        int len = 0;
        if (popData(len))
        {
            str.resize(len / sizeof(std::wstring::value_type));
            return popData((char*)(str.c_str()), len);
        }
        else
        {
            return false;
        }
    }
};


struct IpcMsg
{
    int msgId;
    IpcMsg()
    {
        msgId = 0;
    };

    virtual ~IpcMsg(){;}
    virtual void pack(std::string& data) const
    {
        MsgPacker packer(data);
        packer.pushData(msgId);
    }

    virtual void upack(const std::string& data)
    {
        MsgUnpacker unpacker(data);
        unpacker.popData(msgId);;
    }
};