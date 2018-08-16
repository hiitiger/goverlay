#pragma once

namespace Storm {

class TextStream
{
public:
    TextStream()
    {
        data_.reserve(512);
    }
    ~TextStream()
    {

    }

    const std::wstring& data() const { return data_;  }

    TextStream& operator<<(bool d) { return (*this) << (d ? "true" : "false"); }
    TextStream& operator<<(char d) { return (*this) << (long)d;}
    TextStream& operator<<(short d) { return (*this) << (long)d;}
    TextStream& operator<<(unsigned short d) {return (*this) << (unsigned long)d;}
    TextStream& operator<<(int d) { return (*this) << (long)d;}
    TextStream& operator<<(unsigned int d) {return (*this) << (unsigned long)d;}

    TextStream& operator<<(long d)
    {
        wchar_t text[k_printBufferSize] = { 0 };
        _snwprintf_s(text, k_printBufferSize, _TRUNCATE, L"%d", d);
        data_.append(text);
        return *this;
    }
    TextStream& operator<<(unsigned long d)
    {
        wchar_t text[k_printBufferSize] = { 0 };
        _snwprintf_s(text, k_printBufferSize, _TRUNCATE, L"%u", d);
        data_.append(text);
        return *this;
    }
    TextStream& operator<<(int64_t d)
    {
        wchar_t text[k_printBufferSize] = { 0 };
        _snwprintf_s(text, k_printBufferSize, _TRUNCATE, L"%I64d", d);
        data_.append(text);
        return *this;
    }
    TextStream& operator<<(uint64_t d)
    {
        wchar_t text[k_printBufferSize] = { 0 };
        _snwprintf_s(text, k_printBufferSize, _TRUNCATE, L"%I64u", d);
        data_.append(text);
        return *this;
    }
    TextStream& operator<<(float d) {return (*this) << (double)d;}
    TextStream& operator<<(double d) 
    {
        wchar_t text[k_printBufferSize] = { 0 };
        _snwprintf_s(text, k_printBufferSize, _TRUNCATE, L"%0.8f", d);
        data_.append(text);
        return *this;
    }
    TextStream& operator<<(const wchar_t* d){ data_.append(d);return *this; }
    TextStream& operator<<(const std::wstring& d) { data_.append(d);return *this; }
    TextStream& operator<<(const char* d){return (*this) << Utils::fromLocal8Bit(d);}
    TextStream& operator<<(const std::string& d) { return (*this) << Utils::fromLocal8Bit(d.c_str(), static_cast<int>(d.size()));}

    TextStream& operator<<(const void* d)
    {
        wchar_t text[k_printBufferSize] = { 0 };
#if defined(_M_AMD64) || defined(_M_X64)
        _snwprintf_s(text, k_printBufferSize, _TRUNCATE, L"0x%I64X", (std::uint64_t)d);

#else
        _snwprintf_s(text, k_printBufferSize, _TRUNCATE, L"0x%08X", (std::uint32_t)d);
#endif
        data_.append(text);
        return *this;
    }

    template<class T, typename = std::enable_if_t<!std::is_pointer<T>>>
    TextStream& operator << (const T& t)
    {
        std::stringstream ss;
        ss << t;
        return (*this) << ss.str(); 
    }

private:
    std::wstring data_;
    static const int k_printBufferSize = 32;      
};

}
