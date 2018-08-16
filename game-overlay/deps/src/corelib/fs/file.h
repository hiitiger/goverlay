#pragma once

namespace Storm {

class File
{
    STORM_NONCOPYABLE(File);
public:
    enum OpenMode {
        ReadOnly = 0x0001,
        WriteOnly = 0x0002,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 0x0004,
        Truncate = 0x0008,
    };

    explicit File(const std::wstring& file);
    ~File();

    const std::wstring fileName() const;
    const std::wstring fullPath() const;
    bool isValid() const;

    bool open(OpenMode openMode);
    void close();

    bool setSize(int64_t);
    int64_t size() const;

    int64_t pos() const;
    bool seek(int64_t offset);
    bool seekEnd();
    bool atEnd() const;

    int32_t read(uint8_t* data, int32_t len);
    int32_t write(const uint8_t* data, int32_t len);

    Buffer readAll();
    Buffer readLine();
    std::vector<Buffer> readLines();
    int32_t writeLine(const Buffer& buffer);
    int32_t writeLines(const std::vector<Buffer>& buffers);

    bool rename(const std::wstring& fileName);

    static bool exists(const std::wstring& file);
    static bool remove(const std::wstring& file);
    static bool copy(const std::wstring& src, const std::wstring& dst);
    static bool copy(const std::wstring& src, const std::wstring& dst, bool overwrite);
    static bool move(const std::wstring& src, const std::wstring& dst);
    static bool move(const std::wstring& src, const std::wstring& dst, bool overwrite);

private:
    std::wstring fullPath_;
    void* file_;
};

DEFINE_ENUM_FLAG_OPERATORS(File::OpenMode);


}
